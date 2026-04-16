//
// Created by xint2 on 16/04/2026.
//

#include "../../include/anonymity/tor_manager.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <set>
#include <thread>
#include <vector>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>


namespace gar::anonymity {
    namespace fs = std::filesystem;

    TorManager::TorManager()
        : tor_process_handle_(nullptr),is_running(false),last_error_("") {
        std::cout<<"Tormanager is ready"<<std::endl;

    }
    TorManager::~TorManager() {
        std::cout<<"TorManager Destroying..."<<std::endl;
        stopTor();//Make sure Tor is stopped
        std::cout<<"TorManager Destroyed"<<std::endl;
    }

    bool TorManager::startTor() {
        std::cout<<"\nTorManager ~~~~~~~~~~ STARTING TOR~~~~~~~~~~~~"<<std::endl;

        std::cout<<"\n{TorManager} searching for tor.exe..."<<std::endl;
        std::string tor_path= findTorExecutable();

        if (tor_path.empty()) {
            setError("Could not find tor in any expected locationn");
            std::cout<<"{TorManager} x Tor executable not found"<<std::endl;
            return false;
        }
        std::cout<<"Tor found at "<<tor_path<<std::endl;

        std::cout<<"Starting Tor process..."<<std::endl;

        STARTUPINFOA si ={0};
        PROCESS_INFORMATION pi ={0};

        si.cb =sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow =SW_HIDE;

        BOOL success = CreateProcessA(
            tor_path.c_str(),
            nullptr,
            nullptr,
            nullptr,
            FALSE,CREATE_NEW_CONSOLE,
            nullptr,nullptr,&si,&pi

            );
        if (!success) {
            DWORD error= GetLastError();
            std::string error_msg ="Failed to start Tor(error: "+std::to_string(error)+")";
            setError(error_msg);
            std::cout<<"TorManager X process creation failed"<<std::endl;
            return false;
        }
        tor_process_handle_ =pi.hProcess;
        is_running =true;

        std::cout<<"TorManager Process started (PID "<<pi.dwProcessId<<")"<<std::endl;
        std::cout<<"TorManager Step 3: waiting for Tor to inilalize..."<<std::endl;

        if (!waitForTorReady(30)) {
            std::cout<<"Timeout waiting for Tor"<<std::endl;
            stopTor();
            return false;
        }
        std::cout<<"TorManager~~~~~~~~TOR STARTED SUCCESSFULY~~~~~~~~~"<<std::endl<<std::endl;
        return true;

    }

    void TorManager::stopTor() {
        if (!is_running) {
            return;
        }
        std::cout<<"{TorManager} stopping Tor..."<<std::endl;

        if (tor_process_handle_ != nullptr) {
            TerminateProcess((HANDLE) tor_process_handle_,0);
            CloseHandle((HANDLE)tor_process_handle_);

            tor_process_handle_ =nullptr;
        }
        is_running =false;
        std::cout<<"{TorManager} Tor stopped"<<std::endl;
    }

    bool TorManager::isTorRunning() const {
        return is_running;

    }
    bool TorManager::waitForTorReady(int timeout_second) {
        std::cout<<"{TorManager} Polling SOCKS5 port for up to"<<timeout_second<<"seconds..."<<std::endl;

        auto start =std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds(timeout_second);
        int attempt =0;
        while (true) {
            attempt++;

            auto elapsed =std::chrono::steady_clock::now() - start;
            if (elapsed>timeout) {
                setError("Timeout waiting for Tor initialize");
                std::cout<<"{TorManager} Timeout after"<<attempt<<"attempts"<<std::endl;
                return false;
            }
            if (verifyTorIsRunnig()) {
                std::cout << "{TorManager} SOCKS5 port responding (attempt "<<attempt<<")"<<std::endl;
                return false;

            }
            if (attempt % 10 ==0) {
                std::cout<<"{TorManager} still waiting.....(attempt"<<attempt<<")"<<std::endl;

            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    bool TorManager::verifyTorIsRunnig() {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2,2), &wsa_data) !=0) {
            return false;
        }
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            WSACleanup();
            return false;

        }
        struct sockaddr_in addr;
        addr.sin_family =AF_INET;
        addr.sin_port=htons(9050);
        inet_pton(AF_INET,"127.0.0.1", &addr.sin_addr);

        int result = connect(sock,(struct sockaddr*)&addr, sizeof(addr));

        closesocket(sock);
        WSACleanup();

        return result ==0;



        //geting process ID


        }
    unsigned long TorManager::getTorProcessId() const {
        return 0;
    }

    std::string TorManager::getLastError() const {
        return last_error_;
    }
    std::string TorManager::findTorExecutable() {
        std::vector<std::string> possible_paths ={
            "tor/tor.exe",
            "./tor/tor.exe",
            "../tor/tor.exe"
        };
        for (const auto& path:possible_paths) {
            if (fs::exists(path)) {
                std::cout<<"{TorManager} Found tor.exe at"<<path<<std::endl;
                return path;
            }

        }
        setError("tor.exe not found in bundled or standard location");
        return "";
    }
    void TorManager::setError(const std::string &error) {
        last_error_ = error;
        std::cerr<<"{TorManager} Errror:"<<error<<std::endl;
    }



}







