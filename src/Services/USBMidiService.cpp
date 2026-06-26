#include "USBMidiService.h"

// TinyUSB MIDI class
#include "class/midi/tusb_midi.h"

// The TinyUSB MIDI callbacks need to be in C linkage
extern "C" {

// Invoked when received new MIDI data
static uint8_t _midi_rx_buf[64];
static size_t _midi_rx_len = 0;
static SemaphoreHandle_t _midi_mutex = nullptr;

void tud_midi_rx_cb(uint8_t cable, uint8_t const* buffer, uint16_t bufsize) {
    if (!_midi_mutex) return;
    if (xSemaphoreTake(_midi_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        size_t toCopy = min(bufsize, (uint16_t)(sizeof(_midi_rx_buf) - _midi_rx_len));
        memcpy(_midi_rx_buf + _midi_rx_len, buffer, toCopy);
        _midi_rx_len += toCopy;
        xSemaphoreGive(_midi_mutex);
    }
}

// Invoked when the device is mounted / configured
void tud_midi_mount_cb(void) {
    // USB host has configured the MIDI device
}

void tud_midi_unmount_cb(void) {
    // USB host has disconnected
}

} // extern "C"

// --- Helper to decode CIN to MidiMessage type ---
static MidiMessageType cinToMsgType(uint8_t cin) {
    switch (cin & 0x0F) {
        case 0x8: return MidiMessageType::NoteOff;
        case 0x9: return MidiMessageType::NoteOn;
        case 0xA: return MidiMessageType::PolyKeyPress;
        case 0xB: return MidiMessageType::ControlChange;
        case 0xC: return MidiMessageType::ProgramChange;
        case 0xD: return MidiMessageType::ChannelPress;
        case 0xE: return MidiMessageType::PitchBend;
        case 0xF: // Single-byte or SysEx
        default:  return MidiMessageType::Invalid;
    }
}

static uint8_t msgTypeToCIN(MidiMessageType type) {
    switch (type) {
        case MidiMessageType::NoteOff:       return 0x8;
        case MidiMessageType::NoteOn:        return 0x9;
        case MidiMessageType::PolyKeyPress:  return 0xA;
        case MidiMessageType::ControlChange: return 0xB;
        case MidiMessageType::ProgramChange: return 0xC; // 2-byte
        case MidiMessageType::ChannelPress:  return 0xD; // 2-byte
        case MidiMessageType::PitchBend:     return 0xE;
        default:                             return 0x0;
    }
}

// --- USBMidiEventPacket methods ---

MidiMessage USBMidiEventPacket::toMidiMessage(uint64_t timestamp) const {
    MidiMessage msg;
    msg.timestamp = timestamp;

    uint8_t cin = header & 0x0F;
    uint8_t cable = (header >> 4) & 0x0F;

    // Channel voice messages
    if (cin >= 0x8 && cin <= 0xE) {
        MidiMessageType type = cinToMsgType(cin);
        uint8_t channel = 0; // Channel info is in the status byte
        // USB MIDI packets store the status byte in byte1
        if (byte1 >= 0x80 && byte1 < 0xF0) {
            type = static_cast<MidiMessageType>(byte1 & 0xF0);
            channel = byte1 & 0x0F;
            msg.data1 = byte2 & 0x7F;
            msg.data2 = byte3 & 0x7F;
        } else {
            // Running status not supported in USB MIDI
            msg.type = type;
            msg.channel = 0;
            msg.data1 = byte1 & 0x7F;
            msg.data2 = byte2 & 0x7F;
        }
        msg.type = type;
        msg.channel = channel;
    } else if (cin == 0xF) { // Single byte
        msg.type = static_cast<MidiMessageType>(byte1);
    } else if (cin == 0x0) { // Reserved / mixed data
        msg.type = MidiMessageType::Invalid;
    } else {
        msg.type = MidiMessageType::Invalid;
    }

    (void)cable;
    return msg;
}

USBMidiEventPacket USBMidiEventPacket::fromMidiMessage(const MidiMessage& msg) {
    USBMidiEventPacket packet;
    packet.header = 0x00; // cable 0

    if (msg.isChannelMessage()) {
        uint8_t cin = msgTypeToCIN(msg.type);

        // For 2-byte messages (PC, CP)
        if (msg.type == MidiMessageType::ProgramChange || msg.type == MidiMessageType::ChannelPress) {
            packet.header = cin;
            packet.byte1 = static_cast<uint8_t>(msg.type) | (msg.channel & 0x0F);
            packet.byte2 = msg.data1 & 0x7F;
            packet.byte3 = 0;
        } else {
            // 3-byte messages (Note On/Off, CC, PB, PolyPress)
            packet.header = cin;
            packet.byte1 = static_cast<uint8_t>(msg.type) | (msg.channel & 0x0F);
            packet.byte2 = msg.data1 & 0x7F;
            packet.byte3 = msg.data2 & 0x7F;
        }
    } else if (msg.isSystemRealtime()) {
        packet.header = 0x0F;
        packet.byte1 = static_cast<uint8_t>(msg.type);
        packet.byte2 = 0;
        packet.byte3 = 0;
    } else {
        // System common, SysEx - send as raw
        packet.header = 0x00;
        packet.byte1 = static_cast<uint8_t>(msg.type);
        packet.byte2 = msg.data1;
        packet.byte3 = msg.data2;
    }

    return packet;
}

// --- USBMidiService ---

USBMidiService::USBMidiService() {
    _midi_mutex = xSemaphoreCreateMutex();
}

USBMidiService::~USBMidiService() {
    end();
    if (_midi_mutex) {
        vSemaphoreDelete(_midi_mutex);
        _midi_mutex = nullptr;
    }
}

bool USBMidiService::begin() {
    if (active) return true;

    // Initialize TinyUSB with MIDI support
    // The Arduino framework's USB.begin() already initializes TinyUSB.
    // We need to init the MIDI interface.
    // In the Arduino ESP32 framework, we use the internal TinyUSB API.
    
    // Note: The Arduino framework initializes TinyUSB before setup().
    // To add MIDI, we rely on the CONFIG_TINYUSB_MIDI_ENABLED build flag
    // which includes MIDI support in the TinyUSB device stack.
    
    // Wait a moment for USB enumeration
    delay(100);
    
    active = true;
    tinyUsbReady = true;
    _midi_rx_len = 0;
    
    return true;
}

void USBMidiService::end() {
    active = false;
    tinyUsbReady = false;
    _midi_rx_len = 0;
}

void USBMidiService::sendMidi(const MidiMessage& msg) {
    if (!active || !tinyUsbReady) return;
    
    auto packet = USBMidiEventPacket::fromMidiMessage(msg);
    sendRawPacket(packet);
}

void USBMidiService::sendRawPacket(const USBMidiEventPacket& packet) {
    if (!active || !tinyUsbReady) return;
    
    uint8_t data[4] = { packet.header, packet.byte1, packet.byte2, packet.byte3 };
    
    // Use TinyUSB's MIDI write function
    // tud_midi_n_write(uint8_t cable, uint8_t const* buffer, uint16_t bufsize)
    // But we need to check if TinyUSB is ready
    if (tud_midi_mounted()) {
        tud_midi_packet_write(0, data);
    }
}

bool USBMidiService::pollMessage(MidiMessage& msg) {
    if (!active || !tinyUsbReady) return false;
    
    // Check for received MIDI data
    if (_midi_rx_len >= 4) {
        USBMidiEventPacket packet;
        
        if (xSemaphoreTake(_midi_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            if (_midi_rx_len >= 4) {
                packet.header = _midi_rx_buf[0];
                packet.byte1 = _midi_rx_buf[1];
                packet.byte2 = _midi_rx_buf[2];
                packet.byte3 = _midi_rx_buf[3];
                
                // Shift remaining data
                if (_midi_rx_len > 4) {
                    memmove(_midi_rx_buf, _midi_rx_buf + 4, _midi_rx_len - 4);
                }
                _midi_rx_len -= 4;
                xSemaphoreGive(_midi_mutex);
                
                msg = packet.toMidiMessage(millis());
                return true;
            }
            xSemaphoreGive(_midi_mutex);
        }
    }
    
    return false;
}