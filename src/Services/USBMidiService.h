#pragma once
#include <Arduino.h>
#include <string>
#include <vector>
#include <cstdint>
#include "Services/MidiService.h"

// USB MIDI Event Packet (4 bytes)
struct USBMidiEventPacket {
    uint8_t header;  // CN (high nibble) | CIN (low nibble)
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;

    // Convert to MidiMessage
    MidiMessage toMidiMessage(uint64_t timestamp) const;

    // Convert from MidiMessage
    static USBMidiEventPacket fromMidiMessage(const MidiMessage& msg);
};

class USBMidiService {
public:
    USBMidiService();
    ~USBMidiService();

    // Start USB MIDI device
    bool begin();
    
    // Stop USB MIDI device
    void end();
    
    bool isActive() const { return active; }

    // Send a MIDI message over USB
    void sendMidi(const MidiMessage& msg);
    void sendRawPacket(const USBMidiEventPacket& packet);

    // Receive MIDI messages from USB host (call in loop)
    bool pollMessage(MidiMessage& msg);

    // Check if USB is connected/configured
    bool isConnected() const { return tinyUsbReady; }

private:
    bool active = false;
    bool tinyUsbReady = false;
    uint64_t lastPollMs = 0;
};