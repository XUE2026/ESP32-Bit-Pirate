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
    MidiApiService& midiApiService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    HelpShell& helpShell
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      midiService(midiService),
      midiApiService(midiApiService),
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
    else if (root == "usb") {
        if (cmd.getSubcommand() == "start") handleUsbStart();
        else if (cmd.getSubcommand() == "stop") handleUsbStop();
        else handleUsb();
    }
    else if (root == "api") {
        if (cmd.getSubcommand() == "start") handleApiStart();
        else if (cmd.getSubcommand() == "stop") handleApiStop();
        else if (cmd.getSubcommand() == "config") handleApiConfig();
        else if (cmd.getSubcommand() == "whitelist") handleApiWhitelist(cmd);
        else if (cmd.getSubcommand() == "blacklist") handleApiBlacklist(cmd);
        else if (cmd.getSubcommand() == "autostart") handleApiAutostart(cmd);
        else handleApi();
    }
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
USB MIDI - show status and submenu
*/
void MidiController::handleUsb() {
    if (midiApiService.isRunning()) {
        terminalView.println("\nUSB MIDI (API): Running on ports 72 (HTTP) and 73 (WS).");
        terminalView.println("  Active clients : " + std::to_string(midiApiService.getActiveClientCount()));
        terminalView.println("  Total requests : " + std::to_string(midiApiService.getTotalRequests()));
        terminalView.println("  Autostart      : " + std::string(midiApiService.getAutoStart() ? "enabled" : "disabled"));
        terminalView.println("");
        terminalView.println("Subcommands: start, stop");
    } else {
        terminalView.println("\nUSB MIDI (API): Not running.");
        terminalView.println("Type 'usb start' to start, 'usb stop' to stop.\n");
    }
}

/*
USB MIDI - start
*/
void MidiController::handleUsbStart() {
    if (midiApiService.isRunning()) {
        terminalView.println("USB MIDI: Already running.\n");
        return;
    }
    if (midiApiService.begin()) {
        terminalView.println("USB MIDI (API): Started on ports 72 (HTTP) and 73 (WS).\n");
    } else {
        terminalView.println("USB MIDI (API): Failed to start.\n");
    }
}

/*
USB MIDI - stop
*/
void MidiController::handleUsbStop() {
    if (!midiApiService.isRunning()) {
        terminalView.println("USB MIDI: Not running.\n");
        return;
    }
    midiApiService.end();
    terminalView.println("USB MIDI (API): Stopped.\n");
}

/*
API management - show submenu
*/
void MidiController::handleApi() {
    terminalView.println("\nMIDI API Management");
    terminalView.println("  Status  : " + std::string(midiApiService.isRunning() ? "running" : "stopped"));
    terminalView.println("  Mode    : " + std::string(midiApiService.isWhitelistMode() ? "whitelist" : "blacklist"));
    terminalView.println("  Clients : " + std::to_string(midiApiService.getActiveClientCount()));
    terminalView.println("  Reqs    : " + std::to_string(midiApiService.getTotalRequests()));
    terminalView.println("  Autostrt: " + std::string(midiApiService.getAutoStart() ? "yes" : "no"));
    terminalView.println("");
    terminalView.println("Subcommands:");
    terminalView.println("  api start              Start the API server");
    terminalView.println("  api stop               Stop the API server");
    terminalView.println("  api config             Configure whitelist/blacklist mode");
    terminalView.println("  api whitelist add/del <ip>   Manage whitelist");
    terminalView.println("  api blacklist add/del <ip>   Manage blacklist");
    terminalView.println("  api autostart on|off         Toggle auto-start on WiFi connect\n");
}

/*
API management - start server
*/
void MidiController::handleApiStart() {
    if (midiApiService.isRunning()) {
        terminalView.println("MIDI API: Already running.\n");
        return;
    }
    if (midiApiService.begin()) {
        terminalView.println("MIDI API: Server started on ports 72 (HTTP) and 73 (WS).\n");
    } else {
        terminalView.println("MIDI API: Failed to start server.\n");
    }
}

/*
API management - stop server
*/
void MidiController::handleApiStop() {
    if (!midiApiService.isRunning()) {
        terminalView.println("MIDI API: Not running.\n");
        return;
    }
    midiApiService.end();
    terminalView.println("MIDI API: Server stopped.\n");
}

/*
API management - configure whitelist/blacklist mode
*/
void MidiController::handleApiConfig() {
    terminalView.println("\nMIDI API Access Control Configuration\n");

    bool current = midiApiService.isWhitelistMode();
    std::string prompt = "Use whitelist mode? (Y=whitelist / n=blacklist) [current: " +
                         std::string(current ? "whitelist" : "blacklist") + "]";
    bool useWhitelist = userInputManager.readYesNo(prompt, current);
    midiApiService.setAccessControlMode(useWhitelist);

    terminalView.println("Access control mode set to: " + std::string(useWhitelist ? "whitelist" : "blacklist"));

    auto list = useWhitelist ? midiApiService.getWhitelist() : midiApiService.getBlacklist();
    terminalView.println("Current " + std::string(useWhitelist ? "whitelist" : "blacklist") + " entries:");
    if (list.empty()) {
        terminalView.println("  (none)");
    } else {
        for (const auto& entry : list) {
            terminalView.println("  " + entry);
        }
    }
    terminalView.println("");
}

/*
API management - whitelist add/remove
*/
void MidiController::handleApiWhitelist(const TerminalCommand& cmd) {
    auto args = cmd.getArgs();
    if (args.size() < 2) {
        terminalView.println("Usage: api whitelist add|del <ip>");
        terminalView.println("  add <ip>   Add an IP to the whitelist");
        terminalView.println("  del <ip>   Remove an IP from the whitelist\n");
        return;
    }

    const std::string& action = args[0];
    const std::string& ip = args[1];

    if (action == "add") {
        midiApiService.addToWhitelist(ip);
        terminalView.println("MIDI API: Added " + ip + " to whitelist.\n");
    } else if (action == "del" || action == "remove") {
        midiApiService.removeFromWhitelist(ip);
        terminalView.println("MIDI API: Removed " + ip + " from whitelist.\n");
    } else {
        terminalView.println("MIDI API: Unknown action '" + action + "'. Use add or del.\n");
    }
}

/*
API management - blacklist add/remove
*/
void MidiController::handleApiBlacklist(const TerminalCommand& cmd) {
    auto args = cmd.getArgs();
    if (args.size() < 2) {
        terminalView.println("Usage: api blacklist add|del <ip>");
        terminalView.println("  add <ip>   Add an IP to the blacklist");
        terminalView.println("  del <ip>   Remove an IP from the blacklist\n");
        return;
    }

    const std::string& action = args[0];
    const std::string& ip = args[1];

    if (action == "add") {
        midiApiService.addToBlacklist(ip);
        terminalView.println("MIDI API: Added " + ip + " to blacklist.\n");
    } else if (action == "del" || action == "remove") {
        midiApiService.removeFromBlacklist(ip);
        terminalView.println("MIDI API: Removed " + ip + " from blacklist.\n");
    } else {
        terminalView.println("MIDI API: Unknown action '" + action + "'. Use add or del.\n");
    }
}

/*
API management - toggle auto-start
*/
void MidiController::handleApiAutostart(const TerminalCommand& cmd) {
    auto args = cmd.getArgs();
    if (args.empty()) {
        bool current = midiApiService.getAutoStart();
        terminalView.println("MIDI API: Auto-start is " + std::string(current ? "enabled" : "disabled"));
        terminalView.println("Usage: api autostart on|off\n");
        return;
    }

    const std::string& val = args[0];
    if (val == "on" || val == "1" || val == "true" || val == "yes") {
        midiApiService.setAutoStart(true);
        terminalView.println("MIDI API: Auto-start enabled. Server will start on WiFi connect.\n");
    } else if (val == "off" || val == "0" || val == "false" || val == "no") {
        midiApiService.setAutoStart(false);
        terminalView.println("MIDI API: Auto-start disabled.\n");
    } else {
        terminalView.println("MIDI API: Invalid value '" + val + "'. Use on or off.\n");
    }
}

/*
Help
*/
void MidiController::handleHelp() {
    terminalView.println("\nUnknown command. Available MIDI commands:");
    helpShell.run(state.getCurrentMode(), false);
}