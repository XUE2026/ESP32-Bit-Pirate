#include "MidiService.h"
#include <sstream>
#include <iomanip>

static constexpr uint32_t MIDI_BAUD = 31250;

// Use Serial2 (UART_NUM_2) for MIDI
static HardwareSerial& midiSerial = Serial2;

MidiService::~MidiService() {
    end();
}

bool MidiService::begin(uint8_t txPin, uint8_t rxPin) {
    if (active) end();

    txPin_ = txPin;
    rxPin_ = rxPin;

    // Start HardwareSerial at MIDI baud rate
    midiSerial.begin(MIDI_BAUD, SERIAL_8N1, rxPin, txPin);

    active = true;
    runningStatus_ = 0;
    rxBuffer_.clear();
    return true;
}

void MidiService::end() {
    if (active) {
        midiSerial.end();
        active = false;
    }
    runningStatus_ = 0;
    rxBuffer_.clear();
}

void MidiService::sendByte(uint8_t b) {
    if (!active) return;
    midiSerial.write(b);
    if (thruEnabled) {
        rxBuffer_.push_back(b);
    }
}

void MidiService::sendRaw(const std::vector<uint8_t>& data) {
    if (!active || data.empty()) return;
    midiSerial.write(data.data(), data.size());
}

// --- MIDI Message Senders ---

void MidiService::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::NoteOn) | (channel & 0x0F);
    uint8_t buf[3] = { status, note & 0x7F, velocity & 0x7F };
    midiSerial.write(buf, 3);
}

void MidiService::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::NoteOff) | (channel & 0x0F);
    uint8_t buf[3] = { status, note & 0x7F, velocity & 0x7F };
    midiSerial.write(buf, 3);
}

void MidiService::sendControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::ControlChange) | (channel & 0x0F);
    uint8_t buf[3] = { status, controller & 0x7F, value & 0x7F };
    midiSerial.write(buf, 3);
}

void MidiService::sendProgramChange(uint8_t channel, uint8_t program) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::ProgramChange) | (channel & 0x0F);
    uint8_t buf[2] = { status, program & 0x7F };
    midiSerial.write(buf, 2);
}

void MidiService::sendPitchBend(uint8_t channel, uint16_t value) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::PitchBend) | (channel & 0x0F);
    uint8_t lsb = value & 0x7F;
    uint8_t msb = (value >> 7) & 0x7F;
    uint8_t buf[3] = { status, lsb, msb };
    midiSerial.write(buf, 3);
}

void MidiService::sendPolyKeyPress(uint8_t channel, uint8_t note, uint8_t pressure) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::PolyKeyPress) | (channel & 0x0F);
    uint8_t buf[3] = { status, note & 0x7F, pressure & 0x7F };
    midiSerial.write(buf, 3);
}

void MidiService::sendChannelPress(uint8_t channel, uint8_t pressure) {
    uint8_t status = static_cast<uint8_t>(MidiMessageType::ChannelPress) | (channel & 0x0F);
    uint8_t buf[2] = { status, pressure & 0x7F };
    midiSerial.write(buf, 2);
}

void MidiService::sendSongPosition(uint16_t beats) {
    uint8_t lsb = beats & 0x7F;
    uint8_t msb = (beats >> 7) & 0x7F;
    uint8_t buf[3] = { static_cast<uint8_t>(MidiMessageType::SongPosition), lsb, msb };
    midiSerial.write(buf, 3);
}

void MidiService::sendSongSelect(uint8_t song) {
    uint8_t buf[2] = { static_cast<uint8_t>(MidiMessageType::SongSelect), song & 0x7F };
    midiSerial.write(buf, 2);
}

void MidiService::sendTuneRequest() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::TuneRequest);
    midiSerial.write(b);
}

void MidiService::sendClock() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::Clock);
    midiSerial.write(b);
}

void MidiService::sendStart() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::Start);
    midiSerial.write(b);
}

void MidiService::sendContinue() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::Continue);
    midiSerial.write(b);
}

void MidiService::sendStop() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::Stop);
    midiSerial.write(b);
}

void MidiService::sendActiveSensing() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::ActiveSensing);
    midiSerial.write(b);
}

void MidiService::sendSystemReset() {
    uint8_t b = static_cast<uint8_t>(MidiMessageType::SystemReset);
    midiSerial.write(b);
}

void MidiService::sendSysEx(const std::vector<uint8_t>& data) {
    if (!active) return;
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(MidiMessageType::SystemExclusive));
    packet.insert(packet.end(), data.begin(), data.end());
    packet.push_back(static_cast<uint8_t>(MidiMessageType::SysExEnd));
    midiSerial.write(packet.data(), packet.size());
}

bool MidiService::available() const {
    if (!active) return false;
    return midiSerial.available() > 0 || !rxBuffer_.empty();
}

uint8_t MidiService::readByte() {
    if (!active) return 0;
    if (!rxBuffer_.empty()) {
        uint8_t b = rxBuffer_.front();
        rxBuffer_.erase(rxBuffer_.begin());
        return b;
    }
    if (midiSerial.available()) {
        return (uint8_t)midiSerial.read();
    }
    return 0;
}

void MidiService::flush() {
    if (!active) return;
    while (midiSerial.available()) midiSerial.read();
    rxBuffer_.clear();
    runningStatus_ = 0;
}

// --- MIDI Message Parsing ---

size_t MidiService::parseMessage(const std::vector<uint8_t>& data, size_t offset, MidiMessage& msg) {
    if (offset >= data.size()) return 0;

    uint8_t s = data[offset];
    msg.timestamp = millis();

    // System Real-Time messages (single byte)
    if (s >= 0xF8) {
        msg.type = static_cast<MidiMessageType>(s);
        msg.channel = 0;
        msg.data1 = 0;
        msg.data2 = 0;
        return 1;
    }

    // System Common messages
    if (s == 0xF0) { // SysEx start
        msg.type = MidiMessageType::SystemExclusive;
        msg.sysExData.clear();
        size_t i = offset + 1;
        while (i < data.size()) {
            if (data[i] == 0xF7) {
                i++;
                break;
            }
            if (data[i] >= 0xF8) {
                i++;
                continue;
            }
            msg.sysExData.push_back(data[i]);
            i++;
        }
        return i - offset;
    }

    if (s == 0xF1) {
        if (offset + 1 >= data.size()) return 0;
        msg.type = MidiMessageType::TimeCode;
        msg.data1 = data[offset + 1] & 0x7F;
        return 2;
    }

    if (s == 0xF2) {
        if (offset + 2 >= data.size()) return 0;
        msg.type = MidiMessageType::SongPosition;
        uint16_t pos = data[offset + 1] | (data[offset + 2] << 7);
        msg.data1 = pos & 0x7F;
        msg.data2 = (pos >> 7) & 0x7F;
        return 3;
    }

    if (s == 0xF3) {
        if (offset + 1 >= data.size()) return 0;
        msg.type = MidiMessageType::SongSelect;
        msg.data1 = data[offset + 1] & 0x7F;
        return 2;
    }

    if (s == 0xF6 || s == 0xF7) {
        msg.type = static_cast<MidiMessageType>(s);
        return 1;
    }

    // Channel Voice Messages
    if (s >= 0x80 && s < 0xF0) {
        uint8_t typeBits = s & 0xF0;
        uint8_t chan = s & 0x0F;
        size_t needed = 0;

        switch (typeBits) {
            case 0x80:
            case 0x90:
            case 0xA0:
            case 0xB0:
                needed = 3; break;
            case 0xC0:
            case 0xD0:
                needed = 2; break;
            case 0xE0:
                needed = 3; break;
            default:
                return 0;
        }

        if (offset + needed > data.size()) return 0;

        msg.type = static_cast<MidiMessageType>(typeBits);
        msg.channel = chan;
        msg.data1 = data[offset + 1] & 0x7F;
        msg.data2 = (needed == 3) ? (data[offset + 2] & 0x7F) : 0;
        runningStatus_ = s;
        return needed;
    }

    // Running status
    if (runningStatus_ != 0 && s < 0x80) {
        uint8_t typeBits = runningStatus_ & 0xF0;
        uint8_t chan = runningStatus_ & 0x0F;
        size_t needed = 0;

        switch (typeBits) {
            case 0x80:
            case 0x90:
            case 0xA0:
            case 0xB0:
                needed = 2; break;
            case 0xC0:
            case 0xD0:
                needed = 1; break;
            case 0xE0:
                needed = 2; break;
            default:
                return 0;
        }

        if (offset + needed > data.size()) return 0;

        msg.type = static_cast<MidiMessageType>(typeBits);
        msg.channel = chan;
        msg.data1 = s & 0x7F;
        msg.data2 = (needed >= 2) ? (data[offset + 1] & 0x7F) : 0;
        return needed;
    }

    return 0;
}

bool MidiService::readMessage(MidiMessage& msg, uint32_t timeoutMs) {
    if (!active) return false;

    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        while (midiSerial.available()) {
            rxBuffer_.push_back((uint8_t)midiSerial.read());
        }

        if (!rxBuffer_.empty()) {
            size_t consumed = parseMessage(rxBuffer_, 0, msg);
            if (consumed > 0) {
                rxBuffer_.erase(rxBuffer_.begin(), rxBuffer_.begin() + consumed);
                return true;
            }
            if (runningStatus_ != 0) {
                MidiMessage rsMsg;
                size_t c = parseMessage(rxBuffer_, 0, rsMsg);
                if (c > 0) {
                    rxBuffer_.erase(rxBuffer_.begin(), rxBuffer_.begin() + c);
                    msg = rsMsg;
                    return true;
                }
            }
        }

        delay(1);
    }
    return false;
}

bool MidiService::pollMessage(MidiMessage& msg) {
    if (!active) return false;

    while (midiSerial.available()) {
        rxBuffer_.push_back((uint8_t)midiSerial.read());
    }

    if (!rxBuffer_.empty()) {
        size_t consumed = parseMessage(rxBuffer_, 0, msg);
        if (consumed > 0) {
            rxBuffer_.erase(rxBuffer_.begin(), rxBuffer_.begin() + consumed);
            return true;
        }
    }
    return false;
}

// --- MidiMessage::toString ---

std::string MidiMessage::toString() const {
    std::stringstream ss;
    uint32_t secs = timestamp / 1000;
    uint32_t ms = timestamp % 1000;
    ss << "[" << secs << "." << std::setw(3) << std::setfill('0') << ms << "] ";

    switch (type) {
        case MidiMessageType::NoteOff:
            ss << "NoteOff  ch=" << (int)(channel + 1)
               << " note=" << (int)data1
               << " vel=" << (int)data2;
            break;
        case MidiMessageType::NoteOn:
            ss << "NoteOn   ch=" << (int)(channel + 1)
               << " note=" << (int)data1
               << " vel=" << (int)data2;
            break;
        case MidiMessageType::PolyKeyPress:
            ss << "PolyPress ch=" << (int)(channel + 1)
               << " note=" << (int)data1
               << " press=" << (int)data2;
            break;
        case MidiMessageType::ControlChange:
            ss << "CC       ch=" << (int)(channel + 1)
               << " ctrl=" << (int)data1
               << " val=" << (int)data2;
            break;
        case MidiMessageType::ProgramChange:
            ss << "ProgChg  ch=" << (int)(channel + 1)
               << " prog=" << (int)data1;
            break;
        case MidiMessageType::ChannelPress:
            ss << "ChPress  ch=" << (int)(channel + 1)
               << " press=" << (int)data1;
            break;
        case MidiMessageType::PitchBend:
            ss << "PitchBend ch=" << (int)(channel + 1)
               << " val=" << (data1 | (data2 << 7));
            break;
        case MidiMessageType::SystemExclusive:
            ss << "SysEx    len=" << sysExData.size();
            for (auto b : sysExData) ss << " " << std::hex << std::setw(2) << std::setfill('0') << (int)b;
            ss << std::dec;
            break;
        case MidiMessageType::Clock:
            ss << "Clock";
            break;
        case MidiMessageType::Start:
            ss << "Start";
            break;
        case MidiMessageType::Continue:
            ss << "Continue";
            break;
        case MidiMessageType::Stop:
            ss << "Stop";
            break;
        case MidiMessageType::Tick:
            ss << "Tick";
            break;
        case MidiMessageType::ActiveSensing:
            ss << "ActiveSensing";
            break;
        case MidiMessageType::SystemReset:
            ss << "SystemReset";
            break;
        case MidiMessageType::TuneRequest:
            ss << "TuneRequest";
            break;
        case MidiMessageType::SongPosition:
            ss << "SongPos  beats=" << (data1 | (data2 << 7));
            break;
        case MidiMessageType::SongSelect:
            ss << "SongSel  song=" << (int)data1;
            break;
        case MidiMessageType::TimeCode:
            ss << "TimeCode val=" << (int)data1;
            break;
        default:
            ss << "Unknown  type=0x" << std::hex << (int)static_cast<uint8_t>(type) << std::dec;
            break;
    }
    return ss.str();
}