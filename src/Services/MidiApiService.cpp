#include "MidiApiService.h"
#include <ArduinoJson.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <arpa/inet.h>

// Global pointer for static callbacks
static MidiApiService* g_midiApi = nullptr;

// WebSocket frame helpers
#define WS_FRAME_TEXT 0x01
#define WS_FRAME_BIN  0x02
#define WS_FRAME_CLOSE 0x08
#define WS_FRAME_PING 0x09
#define WS_FRAME_PONG 0x0A

static esp_err_t ws_send_frame(httpd_req_t *req, uint8_t type, const uint8_t *data, size_t len) {
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));
    ws_pkt.type = (httpd_ws_type_t)type;
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = len;
    return httpd_ws_send_frame_async(req->handle, httpd_req_to_sockfd(req), &ws_pkt);
}

// --- MidiApiService implementation ---

MidiApiService::MidiApiService(MidiService& midiService)
    : midiService(midiService) {
    // Set default whitelist/blacklist
    // Empty by default = allow all
}

MidiApiService::~MidiApiService() {
    end();
}

bool MidiApiService::begin() {
    if (running) return true;

    httpd_config_t config72 = HTTPD_DEFAULT_CONFIG();
    config72.server_port = 72;
    config72.lru_purge_enable = true;
    config72.max_open_sockets = 4; // Keep it small
    config72.recv_wait_timeout = 5;
    config72.send_wait_timeout = 5;
    config72.task_priority = 5;
    config72.stack_size = 8192;
    config72.max_uri_handlers = 8;

    httpd_config_t config73 = HTTPD_DEFAULT_CONFIG();
    config73.server_port = 73;
    config73.lru_purge_enable = true;
    config73.max_open_sockets = 4;
    config73.recv_wait_timeout = 5;
    config73.send_wait_timeout = 5;
    config73.task_priority = 5;
    config73.stack_size = 4096;
    config73.max_uri_handlers = 4;

    g_midiApi = this;

    // Start port 72 (REST API)
    esp_err_t err = httpd_start(&httpd72, &config72);
    if (err != ESP_OK) {
        log_e("MIDI API: Failed to start port 72: %d", err);
        g_midiApi = nullptr;
        return false;
    }

    // Register REST endpoints on port 72
    httpd_uri_t uri_get_info = {
        .uri = "/info",
        .method = HTTP_GET,
        .handler = handleGetInfo,
        .user_ctx = this
    };
    httpd_register_uri_handler(httpd72, &uri_get_info);

    httpd_uri_t uri_get_status = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = handleGetStatus,
        .user_ctx = this
    };
    httpd_register_uri_handler(httpd72, &uri_get_status);

    httpd_uri_t uri_send_midi = {
        .uri = "/send",
        .method = HTTP_POST,
        .handler = handleSendMidi,
        .user_ctx = this
    };
    httpd_register_uri_handler(httpd72, &uri_send_midi);

    // Start port 73 (WebSocket)
    err = httpd_start(&httpd73, &config73);
    if (err != ESP_OK) {
        log_e("MIDI API: Failed to start port 73: %d", err);
        httpd_stop(httpd72);
        httpd72 = nullptr;
        g_midiApi = nullptr;
        return false;
    }

    // Register WebSocket endpoint on port 73
    httpd_uri_t uri_ws = {
        .uri = "/midi",
        .method = HTTP_GET,
        .handler = handleWebSocket,
        .user_ctx = this,
        .is_websocket = true
    };
    httpd_register_uri_handler(httpd73, &uri_ws);

    running = true;
    log_i("MIDI API: Started on ports 72 (HTTP) and 73 (WS)");
    return true;
}

void MidiApiService::end() {
    if (!running) return;

    running = false;
    activeClient.isActive = false;

    if (httpd73) {
        httpd_stop(httpd73);
        httpd73 = nullptr;
    }
    if (httpd72) {
        httpd_stop(httpd72);
        httpd72 = nullptr;
    }

    g_midiApi = nullptr;
    log_i("MIDI API: Stopped");
}

// --- Access Control ---

void MidiApiService::addToWhitelist(const std::string& ip) {
    std::lock_guard<std::mutex> lock(listMutex);
    whitelist.insert(ip);
    blacklist.erase(ip);
}

void MidiApiService::removeFromWhitelist(const std::string& ip) {
    std::lock_guard<std::mutex> lock(listMutex);
    whitelist.erase(ip);
}

void MidiApiService::addToBlacklist(const std::string& ip) {
    std::lock_guard<std::mutex> lock(listMutex);
    blacklist.insert(ip);
    whitelist.erase(ip);
}

void MidiApiService::removeFromBlacklist(const std::string& ip) {
    std::lock_guard<std::mutex> lock(listMutex);
    blacklist.erase(ip);
}

void MidiApiService::clearList() {
    std::lock_guard<std::mutex> lock(listMutex);
    whitelist.clear();
    blacklist.clear();
    useWhitelist = false;
}

std::vector<std::string> MidiApiService::getWhitelist() const {
    std::lock_guard<std::mutex> lock(listMutex);
    return std::vector<std::string>(whitelist.begin(), whitelist.end());
}

std::vector<std::string> MidiApiService::getBlacklist() const {
    std::lock_guard<std::mutex> lock(listMutex);
    return std::vector<std::string>(blacklist.begin(), blacklist.end());
}

bool MidiApiService::isClientAllowed(uint32_t clientIp) {
    std::lock_guard<std::mutex> lock(listMutex);

    // Convert IP to string
    char ipBuf[16];
    snprintf(ipBuf, sizeof(ipBuf), "%u.%u.%u.%u",
             (clientIp >> 0) & 0xFF,
             (clientIp >> 8) & 0xFF,
             (clientIp >> 16) & 0xFF,
             (clientIp >> 24) & 0xFF);
    std::string ipStr(ipBuf);

    // If not whitelist mode and no blacklist entries, allow all
    if (!useWhitelist && blacklist.empty()) return true;

    if (useWhitelist) {
        // Only allow whitelisted IPs
        return whitelist.find(ipStr) != whitelist.end();
    }

    // Blacklist mode: block blacklisted IPs
    return blacklist.find(ipStr) == blacklist.end();
}

bool MidiApiService::registerClient(uint32_t clientIp) {
    std::lock_guard<std::mutex> lock(clientMutex);

    // If no active client, register this one
    if (!activeClient.isActive) {
        activeClient.ip = clientIp;
        activeClient.connectedSince = millis();
        activeClient.isActive = true;
        return true;
    }

    // If this client is already the active one, allow
    if (activeClient.ip == clientIp) {
        return true;
    }

    // Another client is already connected
    return false;
}

void MidiApiService::unregisterClient() {
    std::lock_guard<std::mutex> lock(clientMutex);
    if (activeClient.isActive) {
        activeClient.isActive = false;
        activeClient.ip = 0;
        activeClient.connectedSince = 0;
    }
}

bool MidiApiService::checkAccess(httpd_req_t *req) {
    // Simplified: allow all clients, enforce single-IP via registerClient
    if (!registerClient(0)) {
        httpd_resp_set_status(req, "429 Too Many Requests");
        httpd_resp_send(req, "Another client is already connected", HTTPD_RESP_USE_STRLEN);
        return false;
    }
    totalRequests++;
    return true;
}

MidiApiService* MidiApiService::getInstance(httpd_req_t *req) {
    return static_cast<MidiApiService*>(req->user_ctx);
}

// --- REST API Handlers ---

esp_err_t MidiApiService::handleGetInfo(httpd_req_t *req) {
    MidiApiService* api = getInstance(req);
    if (!api) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Service not available");
        return ESP_FAIL;
    }

    if (!api->checkAccess(req)) return ESP_FAIL;

    JsonDocument doc;
    doc["service"] = "ESP32-Bit-Pirate MIDI API";
    doc["version"] = "1.0";
    doc["midi_active"] = api->midiService.isActive();
    doc["midi_tx_pin"] = api->midiService.getTxPin();
    doc["midi_rx_pin"] = api->midiService.getRxPin();
    doc["thru_enabled"] = api->midiService.getThru();
    doc["api_running"] = api->isRunning();
    doc["api_autostart"] = api->getAutoStart();
    doc["clients_active"] = api->getActiveClientCount();
    doc["access_mode"] = api->isWhitelistMode() ? "whitelist" : "blacklist";
    doc["total_requests"] = api->getTotalRequests();

    std::string response;
    serializeJson(doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response.c_str(), response.length());
    return ESP_OK;
}

esp_err_t MidiApiService::handleGetStatus(httpd_req_t *req) {
    MidiApiService* api = getInstance(req);
    if (!api) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Service not available");
        return ESP_FAIL;
    }

    if (!api->checkAccess(req)) return ESP_FAIL;

    JsonDocument doc;
    doc["running"] = api->isRunning();
    doc["connected"] = api->midiService.isActive();
    doc["thru"] = api->midiService.getThru();
    doc["available"] = api->midiService.available();
    doc["client_active"] = api->getActiveClientCount();
    doc["uptime_ms"] = millis();

    std::string response;
    serializeJson(doc, response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response.c_str(), response.length());
    return ESP_OK;
}

esp_err_t MidiApiService::handleSendMidi(httpd_req_t *req) {
    MidiApiService* api = getInstance(req);
    if (!api) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Service not available");
        return ESP_FAIL;
    }

    if (!api->checkAccess(req)) return ESP_FAIL;

    // Read POST body
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    std::string response;

    // Check for "raw" hex bytes
    if (doc.containsKey("raw")) {
        JsonArray raw = doc["raw"].as<JsonArray>();
        std::vector<uint8_t> bytes;
        for (auto v : raw) {
            bytes.push_back((uint8_t)v.as<int>());
        }
        if (!bytes.empty()) {
            api->midiService.sendRaw(bytes);
        }
        response = "{\"status\":\"ok\",\"sent\":" + std::to_string(bytes.size()) + "}";
    }
    // Check for "note" object
    else if (doc.containsKey("note")) {
        int ch = doc["note"]["channel"] | 1;
        int note = doc["note"]["note"] | 60;
        int vel = doc["note"]["velocity"] | 100;

        if (vel == 0) {
            api->midiService.sendNoteOff((uint8_t)(ch - 1), (uint8_t)note, 0);
        } else {
            api->midiService.sendNoteOn((uint8_t)(ch - 1), (uint8_t)note, (uint8_t)vel);
        }
        response = "{\"status\":\"ok\",\"type\":\"note\",\"channel\":" + std::to_string(ch) +
                   ",\"note\":" + std::to_string(note) + ",\"velocity\":" + std::to_string(vel) + "}";
    }
    // Check for "cc" object
    else if (doc.containsKey("cc")) {
        int ch = doc["cc"]["channel"] | 1;
        int ctrl = doc["cc"]["controller"] | 0;
        int val = doc["cc"]["value"] | 0;
        api->midiService.sendControlChange((uint8_t)(ch - 1), (uint8_t)ctrl, (uint8_t)val);
        response = "{\"status\":\"ok\",\"type\":\"cc\",\"channel\":" + std::to_string(ch) +
                   ",\"controller\":" + std::to_string(ctrl) + ",\"value\":" + std::to_string(val) + "}";
    }
    // Check for "program" / "pgm"
    else if (doc.containsKey("program")) {
        int ch = doc["program"]["channel"] | 1;
        int pgm = doc["program"]["program"] | 0;
        api->midiService.sendProgramChange((uint8_t)(ch - 1), (uint8_t)pgm);
        response = "{\"status\":\"ok\",\"type\":\"program\",\"channel\":" + std::to_string(ch) +
                   ",\"program\":" + std::to_string(pgm) + "}";
    }
    // Check for "pitch" object
    else if (doc.containsKey("pitch")) {
        int ch = doc["pitch"]["channel"] | 1;
        int val = doc["pitch"]["value"] | 8192;
        api->midiService.sendPitchBend((uint8_t)(ch - 1), (uint16_t)val);
        response = "{\"status\":\"ok\",\"type\":\"pitch\",\"channel\":" + std::to_string(ch) +
                   ",\"value\":" + std::to_string(val) + "}";
    }
    // Check for "sysEx" array
    else if (doc.containsKey("sysex")) {
        JsonArray sysex = doc["sysex"].as<JsonArray>();
        std::vector<uint8_t> data;
        for (auto v : sysex) {
            data.push_back((uint8_t)v.as<int>());
        }
        api->midiService.sendSysEx(data);
        response = "{\"status\":\"ok\",\"type\":\"sysex\",\"length\":" + std::to_string(data.size()) + "}";
    }
    // Check for "clock" boolean
    else if (doc.containsKey("clock")) {
        api->midiService.sendClock();
        response = "{\"status\":\"ok\",\"type\":\"clock\"}";
    }
    // Check for "transport" string
    else if (doc.containsKey("transport")) {
        std::string cmd = doc["transport"].as<std::string>();
        if (cmd == "start") api->midiService.sendStart();
        else if (cmd == "stop") api->midiService.sendStop();
        else if (cmd == "continue") api->midiService.sendContinue();
        response = "{\"status\":\"ok\",\"type\":\"transport\",\"command\":\"" + cmd + "\"}";
    }
    else {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"Unknown command format\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response.c_str(), response.length());
    return ESP_OK;
}

esp_err_t MidiApiService::handleWebSocket(httpd_req_t *req) {
    MidiApiService* api = getInstance(req);
    if (!api) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Service not available");
        return ESP_FAIL;
    }

    if (req->method == HTTP_GET) {
        // WebSocket handshake - handled by the httpd framework
        // We need to accept the WS connection
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(ws_pkt));
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;

        esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
        if (ret != ESP_OK) {
            return ret;
        }

        // Send welcome message
        std::string welcome = "{\"msg\":\"MIDI WebSocket connected\",\"type\":\"welcome\"}";
        ws_send_frame(req, WS_FRAME_TEXT, (const uint8_t*)welcome.c_str(), welcome.length());

        // Main WS loop - forward MIDI messages in real-time
        // and accept incoming MIDI data from the client
        uint8_t* rx_buf = (uint8_t*)malloc(256);
        if (!rx_buf) return ESP_ERR_NO_MEM;

        bool wsOpen = true;
        uint64_t lastMidiCheck = 0;

        while (wsOpen && api->isRunning()) {
            // Check for incoming WS data (MIDI from client -> device)
            httpd_ws_frame_t rx_pkt;
            memset(&rx_pkt, 0, sizeof(rx_pkt));
            rx_pkt.payload = rx_buf;
            rx_pkt.len = 256;

            esp_err_t rret = httpd_ws_recv_frame(req, &rx_pkt, 0);
            if (rret == ESP_OK && rx_pkt.len > 0) {
                // Parse incoming MIDI JSON from WebSocket
                rx_buf[rx_pkt.len] = '\0';
                JsonDocument doc;
                DeserializationError err = deserializeJson(doc, (const char*)rx_buf);
                if (!err) {
                    // Handle the same commands as the REST API
                    if (doc.containsKey("raw")) {
                        JsonArray raw = doc["raw"].as<JsonArray>();
                        std::vector<uint8_t> bytes;
                        for (auto v : raw) bytes.push_back((uint8_t)v.as<int>());
                        if (!bytes.empty()) api->midiService.sendRaw(bytes);
                    }
                    if (doc.containsKey("note")) {
                        int ch = doc["note"]["channel"] | 1;
                        int note = doc["note"]["note"] | 60;
                        int vel = doc["note"]["velocity"] | 100;
                        if (vel == 0) api->midiService.sendNoteOff((uint8_t)(ch - 1), (uint8_t)note, 0);
                        else api->midiService.sendNoteOn((uint8_t)(ch - 1), (uint8_t)note, (uint8_t)vel);
                    }
                    if (doc.containsKey("cc")) {
                        int ch = doc["cc"]["channel"] | 1;
                        int ctrl = doc["cc"]["controller"] | 0;
                        int val = doc["cc"]["value"] | 0;
                        api->midiService.sendControlChange((uint8_t)(ch - 1), (uint8_t)ctrl, (uint8_t)val);
                    }
                }
            } else if (rret == ESP_ERR_HTTPD_INVALID_REQ) {
                wsOpen = false;
                break;
            }

            // Forward incoming MIDI messages to the WebSocket client
            // Check for new MIDI data periodically
            if (millis() - lastMidiCheck > 10) {
                MidiMessage msg;
                if (api->midiService.pollMessage(msg)) {
                    // Send MIDI message as JSON over WebSocket
                    JsonDocument outDoc;
                    outDoc["type"] = "midi";
                    outDoc["timestamp"] = msg.timestamp;

                    uint8_t s = static_cast<uint8_t>(msg.type);
                    outDoc["status"] = s;
                    outDoc["channel"] = msg.channel + 1;
                    outDoc["data1"] = msg.data1;
                    outDoc["data2"] = msg.data2;
                    outDoc["display"] = msg.toString();

                    std::string outStr;
                    serializeJson(outDoc, outStr);
                    ws_send_frame(req, WS_FRAME_TEXT, (const uint8_t*)outStr.c_str(), outStr.length());
                }
                lastMidiCheck = millis();
            }

            delay(5);
        }

        free(rx_buf);

        // Unregister client when WS closes
        api->unregisterClient();
        return ESP_OK;
    }

    return ESP_FAIL;
}