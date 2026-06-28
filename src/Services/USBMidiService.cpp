#include "USBMidiService.h"

// TinyUSB core and MIDI device
#include "tusb.h"

// --- USBMidiEventPacket methods ---

MidiMessage USBMidiEventPacket::toMidiMessage(uint64_t timestamp) const {
    MidiMessage msg;
    msg.timestamp = timestamp;

    uint8_t cin = header & 0x0F;
    uint8_t cable = (header >> 4) & 0x0F;

    // Channel voice messages
    if (cin >= 0x8 && cin <= 0xE) {
        MidiMessageType type;
        uint8_t channel = 0;

        switch (cin) {
            case 0x8: type = MidiMessageType::NoteOff; break;
            case 0x9: type = MidiMessageType::NoteOn; break;
            case 0xA: type = MidiMessageType::PolyKeyPress; break;
            case 0xB: type = MidiMessageType::ControlChange; break;
            case 0xC: type = MidiMessageType::ProgramChange; break;
            case 0xD: type = MidiMessageType::ChannelPress; break;
            case 0xE: type = MidiMessageType::PitchBend; break;
            default: type = MidiMessageType::Invalid; break;
        }

        // USB MIDI packets store the status byte in byte1
        if (byte1 >= 0x80 && byte1 < 0xF0) {
            type = static_cast<MidiMessageType>(byte1 & 0xF0);
            channel = byte1 & 0x0F;
            msg.data1 = byte2 & 0x7F;
            msg.data2 = byte3 & 0x7F;
        } else {
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

    auto cinForType = [](MidiMessageType t) -> uint8_t {
        switch (t) {
            case MidiMessageType::NoteOff: return 0x8;
            case MidiMessageType::NoteOn: return 0x9;
            case MidiMessageType::PolyKeyPress: return 0xA;
            case MidiMessageType::ControlChange: return 0xB;
            case MidiMessageType::ProgramChange: return 0xC;
            case MidiMessageType::ChannelPress: return 0xD;
            case MidiMessageType::PitchBend: return 0xE;
            default: return 0x0;
        }
    };

    if (msg.isChannelMessage()) {
        uint8_t cin = cinForType(msg.type);

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
}

USBMidiService::~USBMidiService() {
    end();
}

bool USBMidiService::begin() {
    if (active) return true;

    // Wait a moment for USB enumeration
    delay(100);

    active = true;
    tinyUsbReady = true;

    return true;
}

void USBMidiService::end() {
    active = false;
    tinyUsbReady = false;
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
    if (tud_midi_n_mounted(0)) {
        tud_midi_n_packet_write(0, data);
    }
}

bool USBMidiService::pollMessage(MidiMessage& msg) {
    if (!active || !tinyUsbReady) return false;

    // Check for received MIDI data using TinyUSB API
    uint8_t packet[4];
    if (tud_midi_n_packet_read(0, packet)) {
        USBMidiEventPacket usbPacket;
        usbPacket.header = packet[0];
        usbPacket.byte1 = packet[1];
        usbPacket.byte2 = packet[2];
        usbPacket.byte3 = packet[3];

        msg = usbPacket.toMidiMessage(millis());
        return true;
    }

    return false;
}