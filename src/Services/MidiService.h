#pragma once
#include <Arduino.h>
#include <string>
#include <vector>
#include <cstdint>

#define MIDI_PORT UART_NUM_2

// MIDI channel voice message status byte masks (high nibble)
enum class MidiMessageType : uint8_t {
    NoteOff        = 0x80,
    NoteOn         = 0x90,
    PolyKeyPress   = 0xA0,
    ControlChange  = 0xB0,
    ProgramChange  = 0xC0,
    ChannelPress   = 0xD0,
    PitchBend      = 0xE0,
    SystemExclusive = 0xF0,
    TimeCode       = 0xF1,
    SongPosition   = 0xF2,
    SongSelect     = 0xF3,
    TuneRequest    = 0xF6,
    SysExEnd       = 0xF7,
    Clock          = 0xF8,
    Tick           = 0xF9,
    Start          = 0xFA,
    Continue       = 0xFB,
    Stop           = 0xFC,
    ActiveSensing  = 0xFE,
    SystemReset    = 0xFF,
    Invalid        = 0x00
};

struct MidiMessage {
    MidiMessageType type = MidiMessageType::Invalid;
    uint8_t channel = 0;    // 0-15
    uint8_t data1 = 0;      // note number / controller number / program number
    uint8_t data2 = 0;      // velocity / value
    std::vector<uint8_t> sysExData; // for system exclusive
    uint64_t timestamp = 0; // millis() when received

    bool isChannelMessage() const {
        uint8_t s = static_cast<uint8_t>(type);
        return s >= 0x80 && s < 0xF0;
    }

    bool isSystemRealtime() const {
        uint8_t s = static_cast<uint8_t>(type);
        return s >= 0xF8;
    }

    std::string toString() const;
};

class MidiService {
public:
    MidiService() = default;
    ~MidiService();

    // Configure and begin MIDI on specified UART pins
    bool begin(uint8_t txPin, uint8_t rxPin);
    void end();

    bool isActive() const { return active; }
    uint8_t getTxPin() const { return txPin_; }
    uint8_t getRxPin() const { return rxPin_; }

    // Send MIDI messages
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendControlChange(uint8_t channel, uint8_t controller, uint8_t value);
    void sendProgramChange(uint8_t channel, uint8_t program);
    void sendPitchBend(uint8_t channel, uint16_t value); // 0-16383, center=8192
    void sendPolyKeyPress(uint8_t channel, uint8_t note, uint8_t pressure);
    void sendChannelPress(uint8_t channel, uint8_t pressure);
    void sendSongPosition(uint16_t beats);
    void sendSongSelect(uint8_t song);
    void sendTuneRequest();
    void sendClock();
    void sendStart();
    void sendContinue();
    void sendStop();
    void sendActiveSensing();
    void sendSystemReset();
    void sendSysEx(const std::vector<uint8_t>& data);

    // Send a MidiMessage object via UART (for USB MIDI Thru routing)
    void sendMidi(const MidiMessage& msg);

    // Send raw bytes
    void sendRaw(const std::vector<uint8_t>& data);
    void sendByte(uint8_t b);

    // Receive
    bool available() const;
    uint8_t readByte();

    // Parse one MIDI message from the RX buffer (blocking, with timeout)
    // Returns true if a complete message was parsed
    bool readMessage(MidiMessage& msg, uint32_t timeoutMs = 10);

    // Non-blocking: check if a complete message is available (call in loop)
    bool pollMessage(MidiMessage& msg);

    // Flush and clear buffers
    void flush();

    // Thru: echo incoming bytes to TX
    void setThru(bool enabled) { thruEnabled = enabled; }
    bool getThru() const { return thruEnabled; }

private:
    bool active = false;
    uint8_t txPin_ = 1;
    uint8_t rxPin_ = 2;
    bool thruEnabled = false;

    // Running status for MIDI IN parsing
    uint8_t runningStatus_ = 0;
    std::vector<uint8_t> rxBuffer_;

    // Parse a MidiMessage from a byte sequence starting at buffer index
    // Returns number of bytes consumed, or 0 if incomplete
    size_t parseMessage(const std::vector<uint8_t>& data, size_t offset, MidiMessage& msg);
};