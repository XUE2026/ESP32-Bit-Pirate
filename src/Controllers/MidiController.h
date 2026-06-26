#pragma once
#include <string>
#include "Models/TerminalCommand.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/MidiService.h"
#include "Services/MidiApiService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Shells/HelpShell.h"
#include "States/GlobalState.h"

class MidiController {
public:
    MidiController(
        ITerminalView& terminalView,
        IInput& terminalInput,
        MidiService& midiService,
        MidiApiService& midiApiService,
        ArgTransformer& argTransformer,
        UserInputManager& userInputManager,
        HelpShell& helpShell
    );

    void handleCommand(const TerminalCommand& cmd);
    void ensureConfigured();

private:
    bool ensurePresent_();

    void handleConfig();
    void handleSend(const TerminalCommand& cmd);
    void handleReceive();
    void handleSniff();
    void handleNote(const TerminalCommand& cmd);
    void handleCc(const TerminalCommand& cmd);
    void handlePgm(const TerminalCommand& cmd);
    void handlePitch(const TerminalCommand& cmd);
    void handleClock();
    void handleStart_();
    void handleStop_();
    void handleContinue_();
    void handleThru(const TerminalCommand& cmd);
    void handleReset();
    void handleHelp();

    // USB MIDI (via WebSocket API on port 73)
    void handleUsb();
    void handleUsbStart();
    void handleUsbStop();

    // API management
    void handleApi();
    void handleApiStart();
    void handleApiStop();
    void handleApiConfig();
    void handleApiWhitelist(const TerminalCommand& cmd);
    void handleApiBlacklist(const TerminalCommand& cmd);
    void handleApiAutostart(const TerminalCommand& cmd);

    ITerminalView& terminalView;
    IInput& terminalInput;
    MidiService& midiService;
    MidiApiService& midiApiService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};