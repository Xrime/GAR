//
// Created by xint2 on 07/05/2026.
//
#include "../../include/anonymity/tor_control.h"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace gar::anonymity {
    TorControl::TorControl() :sock_(-1) {}
    bool TorControl::connect(const std::string& host , int port) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) !=0){
            last_error_ = "WSAStartup failed";
            return false;
        }
        sock_ =(int)socket(AF_INET, SOCK_STREAM, 0);
        if (sock_<0) {
            last_error_= "Socket creation failed";
            return false;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        if (::connect(sock_,(sockaddr*)&addr, sizeof(addr)) != 0) {
            last_error_ = "connect failed";
            return false;
        }
        return true;
    }
    bool TorControl::authenticate(const std::string &password) {
        std::string cmd = "AUTHENTICATE";
        if (!password.empty()) cmd +="\""+ password +"\"";
        cmd+= "\r\n";
        if (!send_line(cmd)) return false;

        std::string line;
        if (!read_line(line)) return false;
        if (line.rfind("250", 0) != 0) {
            last_error_ = "Auth failed: " + line;
            return false;
        }
        return true;
    }
    bool TorControl::signal_newnym() {
        if (!send_line("SIGNAL NEWNYM\r\n")) return false;
        std::string line;
        if (!read_line(line)) return false;
        if (line.rfind("250", 0) !=0) {
            last_error_ = "NEWNYM failed: " + line;
            return false;
        }
        return true;
    }
    void TorControl::disconnect() {
        if (sock_>=0) {
            closesocket(sock_);
            sock_ = -1;
        }
        WSACleanup();
    }
    bool TorControl::send_line (const std::string& line) {
        int sent = send(sock_, line.c_str(),(int)line.size(), 0);
        if (sent <=0) {
            last_error_= "send failed";
            return false;
        }
        return true;
    }
    bool TorControl::read_line(std::string &out) {
        char buf[512]{};
        int n = recv(sock_, buf, sizeof(buf)-1, 0);
        if (n<=0) {
            last_error_ = "Read failed";
            return false;
        }
        out.assign(buf, buf + n);
        return true;
    }bool TorControl::getinfo(const std::string &key, std::string &value) {
        std::string cmd = "GETINFO " + key +"\r\n";
        if (!send_line(cmd)) return false;
        std::string line;
        if (!read_line(line)) return false;

        if (line.rfind("250", 0) !=0) {
            last_error_= "GETINFO failed: " + line;
            return false;
        }
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            value = line.substr(eq +1);
            while (!value.empty() && (value.back() == 'r' || value.back() == '\n')) {
                value.pop_back();
            }
            return true;
        }
        value.clear();
        return true;
    }





}
