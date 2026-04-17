//
// Created by xint2 on 16/04/2026.
//

#include "../../include/anonymity/tor_manager.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <vector>
#include  <sstream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

namespace gar::anonymity {
    namespace fs = std::filesystem;

    TorManager::TorManager()
        : tor_process_handle_(nullptr), is_running(false), last_error_("") {
        std::cout << "Tormanager is ready" << std::endl;

    }

    TorManager::~TorManager() {
        std::cout << "TorManager Destroying..." << std::endl;
        stopTor();//
        std::cout << "TorManager Destroyed" << std::endl;
    }

    bool TorManager::startTor() {
        std::cout << "\n[TorManager] ========== STARTING TOR ==========" << std::endl;

        std::cout << "[TorManager] Step 1: Searching for tor.exe..." << std::endl;
        std::string tor_path = findTorExecutable();

        if (tor_path.empty()) {
            setError("Could not find tor in any expected locationn");
            std::cout << "{TorManager} ✗ Tor executable not found" << std::endl;
            return false;
        }
        std::cout<<"Tor found at " <<tor_path<< std::endl;

        std::cout << "\n{TorManager} Step 2: Looking for torrc..." << std::endl;
        std::string torrc_path = findTorrcFile();

        if (torrc_path.empty()) {
            std::cout << "[TorManager]  No torrc found, using defaults" << std::endl;
        } else {
            std::cout << "[TorManager]  Found torrc at: " << torrc_path << std::endl;
        }

        std::cout << "[TorManager] Step 3: Starting Tor process..." << std::endl;

        STARTUPINFOA si = {};
        PROCESS_INFORMATION pi = {};

        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        // Build command line
        std::string cmd_line = "";
        if (!torrc_path.empty()) {
            cmd_line = "\"" + tor_path + "\" -f \"" + torrc_path + "\"";
        } else {
            cmd_line = "\"" + tor_path + "\"";
        }

        std::cout << "[TorManager] Command: " << cmd_line << std::endl;

        BOOL success = CreateProcessA(
            nullptr,
            (LPSTR)cmd_line.c_str(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (!success) {
            DWORD error = GetLastError();
            std::string error_msg = "Failed to start Tor (error: " + std::to_string(error) + ")";
            setError(error_msg);
            std::cout << "TorManager Process creation failed!" << std::endl;
            return false;
        }

        tor_process_handle_ = pi.hProcess;
        is_running = true;

        std::cout << "TorManager Process started (PID: " << pi.dwProcessId << ")" << std::endl;
        std::cout << "Waiting for Tor to initialize..." << std::endl;

        if (!waitForTorReady(30)) {
            std::cout << "TorManager timeout waiting for Tor!" << std::endl;
            stopTor();
            return false;
        }

        std::cout << "========== TOR STARTED SUCCESSFULLY ==========" << std::endl << std::endl;
        return true;
    }

    void TorManager::stopTor() {
        if (!is_running) {
            return;
        }

        std::cout << " Stopping Tor..." << std::endl;

        if (tor_process_handle_ != nullptr) {
            TerminateProcess((HANDLE)tor_process_handle_, 0);
            CloseHandle((HANDLE)tor_process_handle_);
            tor_process_handle_ = nullptr;
        }

        is_running = false;
        std::cout << "Tor stopped" << std::endl;
    }

    bool TorManager::isTorRunning() const {
        return is_running;
    }

    bool TorManager::waitForTorReady(int timeout_seconds) {
        std::cout << "Polling SOCKS5 port for up to " << timeout_seconds << " seconds..." << std::endl;

        auto start = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds(timeout_seconds);
        int attempt = 0;

        while (true) {
            attempt++;

            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed > timeout) {
                setError("Timeout waiting for Tor to initialize");
                std::cout << " Timeout after " << attempt << " attempts" << std::endl;
                return false;
            }

            //Check if Tor is running
            if (verifyTorIsRunning()) {
                std::cout << "SOCKS5 port responding (attempt " << attempt << ")" << std::endl;
                return true;
            }

            if (attempt % 2 == 0) {
                std::cout << " Still waiting... (attempt " << attempt << ")" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    bool TorManager::verifyTorIsRunning() {
        std::cout << "verifyTorIsRunning() called" << std::endl;

        //Initialize Windows Sockets
        WSADATA wsa_data;
        int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

        if (wsa_result != 0) {
            std::cout << "WSAStartup failed with error: " << wsa_result << std::endl;
            return false;
        }
        std::cout << "WSAStartup successful" << std::endl;

        //create socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (sock == INVALID_SOCKET) {
            std::cout << "socket() failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return false;
        }

        std::cout << "socket created successfully" << std::endl;

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(9050);
        std::cout << "Port set to 9050" << std::endl;

        int addr_result = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        std::cout << "inet_pton result: " << addr_result << std::endl;

        if (addr_result <= 0) {
            std::cout << "inet_pton failed" << std::endl;
            closesocket(sock);
            WSACleanup();
            return false;
        }

        std::cout << "Attempting to connect to 127.0.0.1:9050..." << std::endl;
        int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        std::cout << "connect() returned: " << result << std::endl;

        closesocket(sock);
        WSACleanup();

        bool is_ready = (result == 0);
        std::cout << "verifyTorIsRunning returning: " << (is_ready ? "TRUE" : "FALSE") << std::endl;

        return is_ready;
    }

    unsigned long TorManager::getTorProcessId() const {
        return 0;
    }

    std::string TorManager::getLastError() const {
        return last_error_;
    }

    std::string TorManager::findTorExecutable() {
        std::vector<std::string> possible_paths = {
            "tor/tor.exe",
            "./tor/tor.exe",
            "../tor/tor.exe",
        };

        for (const auto& path : possible_paths) {
            if (fs::exists(path)) {
                return fs::absolute(path).string();
            }
        }

        setError("tor.exe not found in bundled or standard locations");
        return "";
    }

    std::string TorManager::findTorrcFile() {
        std::vector<std::string> possible_paths = {
            "tor/torrc",
            "./tor/torrc",
            "../tor/torrc",
        };

        for (const auto& path : possible_paths) {
            if (fs::exists(path)) {
                return fs::absolute(path).string();
            }
        }

        return "";
    }

    void TorManager::setError(const std::string& error) {
        last_error_ = error;
        std::cerr << "[TorManager] ERROR: " << error << std::endl;
    }
}