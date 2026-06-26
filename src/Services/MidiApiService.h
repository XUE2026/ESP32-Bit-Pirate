#pragma once
#include <Arduino.h>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include "esp_http_server.h"
#include "Services/MidiService.h"

// Max 1 concurrent IP restriction
struct MidiApiClient {
    uint32_t ip;          // Client IP as uint32
    unsigned long connectedSince = 0;
    bool isActive = false;
};

class MidiApiService {
public:
    MidiApiService(MidiService& midiService);
    ~MidiApiService();

    // Start the API server on ports 72 (HTTP) and 73 (WS)
    bool begin();
    
    // Stop the API server
    void end();
    
    bool isRunning() const { return running; }

    // Auto-start configuration
    void setAutoStart(bool autostart) { autoStartEnabled = autostart; }
    bool getAutoStart() const { return autoStartEnabled; }

    // Whitelist/Blacklist management
    void setAccessControlMode(bool whitelistMode) { useWhitelist = whitelistMode; }
    bool isWhitelistMode() const { return useWhitelist; }
    
    void addToWhitelist(const std::string& ip);
    void removeFromWhitelist(const std::string& ip);
    void addToBlacklist(const std::string& ip);
    void removeFromBlacklist(const std::string& ip);
    void clearList();
    
    std::vector<std::string> getWhitelist() const;
    std::vector<std::string> getBlacklist() const;
    
    // Check if a client IP is allowed
    bool isClientAllowed(uint32_t clientIp);
    
    // Register this client as the active session
    bool registerClient(uint32_t clientIp);
    void unregisterClient();
    uint32_t getActiveClient() const { return activeClient.ip; }
    
    // Get current connected client count (should be 0 or 1)
    int getActiveClientCount() const { return activeClient.isActive ? 1 : 0; }

    // Statistics
    unsigned long getTotalRequests() const { return totalRequests; }

    // The MIDI stream WebSocket handler sends all received MIDI messages
    // to the connected WS client
    void notifyMidiMessage(const MidiMessage& msg);

private:
    MidiService& midiService;
    
    bool running = false;
    bool autoStartEnabled = true; // Default: auto-start on WiFi connect
    
    httpd_handle_t httpd72 = nullptr;  // Port 72 - REST API
    httpd_handle_t httpd73 = nullptr;  // Port 73 - WebSocket
    
    // Access control
    bool useWhitelist = false;
    std::set<std::string> whitelist;
    std::set<std::string> blacklist;
    mutable std::mutex listMutex;
    
    // Single client enforcement
    MidiApiClient activeClient;
    mutable std::mutex clientMutex;
    
    // Statistics
    unsigned long totalRequests = 0;
    
    // Internal: check access and register
    bool checkAccess(httpd_req_t *req);
    
    // REST API handlers (static)
    static esp_err_t handleGetInfo(httpd_req_t *req);
    static esp_err_t handleSendMidi(httpd_req_t *req);
    static esp_err_t handleWebSocket(httpd_req_t *req);
    static esp_err_t handleGetStatus(httpd_req_t *req);

    // Get this instance from context pointer
    static MidiApiService* getInstance(httpd_req_t *req);
};

// Global pointer for static handlers
// Set when begin() is called, cleared when end() is called