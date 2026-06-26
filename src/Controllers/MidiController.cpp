#include "MidiController.h"
#include <sstream>
#include <iomanip>
#include <vector>

/*
Constructor
*/
MidiController::MidiController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    MidiService& midiService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    HelpShell& helpShell
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      midiService(midiService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      helpShell(helpShell)
{}

/*
Entry point for MIDI command
*/
void MidiController::handleCommand(const TerminalCommand& cmd) {
    const auto& root = cmd.getRoot();

    if (root == "config") handleConfig();
    else if (root == "send") handleSend(cmd);
    else if (root == "receive") handleReceive();
    else if (root == "sniff") handleSniff();
    else if (root == "note") handleNote(cmd);
    else if (root == "cc") handleCc(cmd);
    else if (root == "pgm" || root == "program") handlePgm(cmd);
    else if (root == "pitch") handlePitch(cmd);
    else if (root == "clock") handleClock();
    else if (root == "start") handleStart_();
    else if (root == "stop") handleStop_();
    else if (root == "continue") handleContinue_();
    else if (root == "thru") handleThru(cmd);
    else if (root == "reset") handleReset();
    else handleHelp();
}

/*
Ensure configured
*/
void MidiController::ensureConfigured() {
    if (!configured) handleConfig();
    else {
        // Re-apply config from state
        midiService.begin(state.getMidiTxPin(), state.getMidiRxPin());
    }
}

/*
Ensure present / running
*/
bool MidiController::ensurePresent_() {
    if (!configured) {
        terminalView.println("MIDI: not configured. Type 'config' first.\n");
        return false;
    }
    if (!midiService.isActive()) {
        terminalView.println("MIDI: interface not active. Type 'config' first.\n");
        return false;
    }
    return true;
}

/*
Config
*/
void MidiController::handleConfig() {
    terminalView.println("MIDI (UART 31250 baud 8N1) configuration.\n");

    auto forbidden = state.getProtectedPins();

    uint8_t txPin = (uint8_t)userInputManager.readValidatedPinNumber("MIDI TX GPIO", state.getMidiTxPin(), forbidden);
    forbidden.push_back(txPin);
    uint8_t rxPin = (uint8_t)userInputManager.readValidatedPinNumber("MIDI RX GPIO", state.getMidiRxPin(), forbidden);

    state.setMidiTxPin(txPin);
    state.setMidiRxPin(rxPin);

    if (midiService.begin(txPin, rxPin)) {
        configured = true;
        terminalView.println("\nMIDI interface started on TX=" + std::to_string(txPin) + " RX=" + std::to_string(rxPin) + "\n");
    } else {
        terminalView.println("\nMIDI: failed to start interface.\n");
    }
}

/*
Send raw MIDI bytes
*/
void MidiController::handleSend(const TerminalCommand& cmd) {
    if (!ensurePresent_()) return;

    std::string data = cmd.getSubcommand();
    if (data.empty()) {
        terminalView.println("Usage: send <hex bytes>");
        terminalView.println("Example: send 90 3C 7F  (Note On ch1, note 60, vel 127)");
        return;
    }

    std::vector<uint8_t> bytes;
    std::stringstream ss(data);
    std::string token;
    while (ss >> token) {
        if (token.size() >= 2 && token.substr(0, 2) == "0x") {
            bytes.push_back((uint8_t)std::stoul(token, nullptr, 16));
        } else if (argTransformer.isValidHex(token)) {
            bytes.push_back((uint8_t)std::stoul(token, nullptr, 16));
        } else if (argTransformer.isValidInt(token)) {
            bytes.push_back((uint8_t)std::stoul(token));
        } else {
            terminalView.println("MIDI send: invalid byte '" + token + "'");
            return;
        }
    }

    if (bytes.empty()) return;

    midiService.sendRaw(bytes);
    terminalView.println("MIDI: sent " + std::to_string(bytes.size()) + " byte(s).");
}

/*
Receive MIDI messages continuously
*/
void MidiController::handleReceive() {
    if (!ensurePresent_()) return;

    terminalView.println("\nMIDI Receive: listening for messages... Press [ENTER] to stop.\n");

    while (true) {
        if (terminalInput.readChar() == '\n' || terminalInput.readChar() == '\r') break;

        MidiMessage msg;
        if (midiService.pollMessage(msg)) {
            terminalView.println(msg.toString());
        } else {
            delay(5);
        }
    }

    terminalView.println("\nMIDI Receive: stopped by user.\n");
}

/*
Sniff MIDI traffic (raw hex dump)
*/
void MidiController::handleSniff() {
    if (!ensurePresent_()) return;

    terminalView.println("\nMIDI Sniff: raw hex dump. Press [ENTER] to stop.\n");

    while (true) {
        if (terminalInput.readChar() == '\n' || terminalInput.readChar() == '\r') break;

        if (midiService.available()) {
            uint8_t b = midiService.readByte();
            std::stringstream ss;
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
            terminalView.print(ss.str());
        } else {
            delay(5);
        }
    }

    terminalView.println("\nMIDI Sniff: stopped by user.\n");
}

/*
Send a Note On/Off message
*/
void MidiController::handleNote(const TerminalCommand& cmd) {
    if (!ensurePresent_()) return;

    auto args = cmd.getArgs();
    if (args.size() < 2) {
        terminalView.println("Usage: note <channel(1-16)> <note(0-127)> [velocity(0-127)]");
        terminalView.println("If velocity is 0, sends Note Off.");
        terminalView.println("Example: note 1 60 100");
        return;
    }

    uint8_t ch = std::stoi(args[0]) - 1;
    uint8_t note = std::stoi(args[1]);
    uint8_t vel = (args.size() >= 3) ? std::stoi(args[2]) : 100;

    if (ch > 15) { terminalView.println("MIDI: channel must be 1-16.\n"); return; }
    if (note > 127) { terminalView.println("MIDI: note must be 0-127.\n"); return; }
    if (vel > 127) { terminalView.println("MIDI: velocity must be 0-127.\n"); return; }

    if (vel == 0) {
        midiService.sendNoteOff(ch, note, 0);
        terminalView.println("MIDI: NoteOff  ch=" + std::to_string(ch + 1) + " note=" + std::to_string(note));
    } else {
        midiService.sendNoteOn(ch, note, vel);
        terminalView.println("MIDI: NoteOn   ch=" + std::to_string(ch + 1) + " note=" + std::to_string(note) + " vel=" + std::to_string(vel));
    }
}

/*
Send Control Change message
*/
void MidiController::handleCc(const TerminalCommand& cmd) {
    if (!ensurePresent_()) return;

    auto args = cmd.getArgs();
    if (args.size() < 2) {
        terminalView.println("Usage: cc <channel(1-16)> <controller(0-127)> [value(0-127)]");
        terminalView.println("Example: cc 1 7 100  (Volume ch1 = 100)");
        return;
    }

    uint8_t ch = std::stoi(args[0]) - 1;
    uint8_t ctrl = std::stoi(args[1]);
    uint8_t val = (args.size() >= 3) ? std::stoi(args[2]) : 0;

    if (ch > 15) { terminalView.println("MIDI: channel must be 1-16.\n"); return; }
    if (ctrl > 127) { terminalView.println("MIDI: controller must be 0-127.\n"); return; }
    if (val > 127) { terminalView.println("MIDI: value must be 0-127.\n"); return; }

    midiService.sendControlChange(ch, ctrl, val);
    terminalView.println("MIDI: CC       ch=" + std::to_string(ch + 1) + " ctrl=" + std::to_string(ctrl) + " val=" + std::to_string(val));
}

/*
Send Program Change
*/
void MidiController::handlePgm(const TerminalCommand& cmd) {
    if (!ensurePresent_()) return;

    auto args = cmd.getArgs();
    if (args.empty()) {
        terminalView.println("Usage: pgm <channel(1-16)> <program(0-127)>");
        terminalView.println("       program <channel(1-16)> <program(0-127)>");
        terminalView.println("Example: pgm 1 0  (Grand Piano on ch1)");
        return;
    }

    uint8_t ch = std::stoi(args[0]) - 1;
    uint8_t pgm = (args.size() >= 2) ? std::stoi(args[1]) : 0;

    if (ch > 15) { terminalView.println("MIDI: channel must be 1-16.\n"); return; }
    if (pgm > 127) { terminalView.println("MIDI: program must be 0-127.\n"); return; }

    midiService.sendProgramChange(ch, pgm);
    terminalView.println("MIDI: ProgChg  ch=" + std::to_string(ch + 1) + " pgm=" + std::to_string(pgm));
}

/*
Send Pitch Bend
*/
void MidiController::handlePitch(const TerminalCommand& cmd) {
    if (!ensurePresent_()) return;

    auto args = cmd.getArgs();
    if (args.empty()) {
        terminalView.println("Usage: pitch <channel(1-16)> [value(0-16383)]");
        terminalView.println("Default value is 8192 (center).");
        terminalView.println("Example: pitch 1 8192");
        return;
    }

    uint8_t ch = std::stoi(args[0]) - 1;
    uint16_t val = (args.size() >= 2) ? (uint16_t)std::stoul(args[1]) : 8192;

    if (ch > 15) { terminalView.println("MIDI: channel must be 1-16.\n"); return; }
    if (val > 16383) { terminalView.println("MIDI: pitch value must be 0-16383.\n"); return; }

    midiService.sendPitchBend(ch, val);
    terminalView.println("MIDI: PitchBend ch=" + std::to_string(ch + 1) + " val=" + std::to_string(val));
}

/*
Send MIDI Clock continuously
*/
void MidiController::handleClock() {
    if (!ensurePresent_()) return;

    terminalView.println("\nMIDI Clock: sending real-time clock... Press [ENTER] to stop.\n");

    while (true) {
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        midiService.sendClock();
        delay(1); // ~1ms = ~1000 BPM, adjustable
    }

    terminalView.println("\nMIDI Clock: stopped by user.\n");
}

/*
Send Start
*/
void MidiController::handleStart_() {
    if (!ensurePresent_()) return;
    midiService.sendStart();
    terminalView.println("MIDI: Start");
}

/*
Send Stop
*/
void MidiController::handleStop_() {
    if (!ensurePresent_()) return;
    midiService.sendStop();
    terminalView.println("MIDI: Stop");
}

/*
Send Continue
*/
void MidiController::handleContinue_() {
    if (!ensurePresent_()) return;
    midiService.sendContinue();
    terminalView.println("MIDI: Continue");
}

/*
Toggle Thru
*/
void MidiController::handleThru(const TerminalCommand& cmd) {
    if (!ensurePresent_()) return;

    auto args = cmd.getArgs();
    bool enable = !midiService.getThru(); // toggle if no arg

    if (!args.empty()) {
        enable = (args[0] == "on" || args[0] == "1" || args[0] == "true");
    }

    midiService.setThru(enable);
    terminalView.println(std::string("MIDI: Thru ") + (enable ? "ON (incoming bytes echoed to TX)" : "OFF"));
}

/*
Reset
*/
void MidiController::handleReset() {
    terminalView.println("MIDI: Resetting interface...");
    midiService.end();
    configured = false;
    terminalView.println("MIDI: Reset. Type 'config' to reconfigure.");
}

/*
Help
*/
void MidiController::handleHelp() {
    terminalView.println("\nUnknown command. Available MIDI commands:");
    helpShell.run(state.getCurrentMode(), false);
}