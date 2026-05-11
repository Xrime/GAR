//
// Created by xint2 on 08/05/2026.
//
#include "../../include/core/dns_resolver.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <chrono>

namespace gar::core {

    dnsResolver::dnsResolver(const std::string &socks_host, int socks_port)
    : host_(socks_host), port_(socks_port){}
    bool dnsResolver::resolve(const std::string &host, std::string &out_ip) {
        auto now = std::chrono::steady_clock::now();
        auto it = cache_.find(host);
        if (it != cache_.end()) {
            if (now < it -> second.expires_at) {
                out_ip = it->second.ip;
                return true;
            }
            cache_.erase(it);
        }
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            last_error_= "Wsastartup failed";
            return false;
        }
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            last_error_ = "socket creation failed";
            WSACleanup();
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &addr.sin_addr);

        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) !=0) {
            last_error_= "Connect to socks failed";
            closesocket(sock);
            WSACleanup();
            return false;
        }
        unsigned char greet[] = {0x05, 0x01,0x00};
        if (send(sock, (const char*)greet, 3,0) != 3) {
            last_error_= "SOCKS greet failed";
            closesocket(sock);
            WSACleanup();
            return false;
        }
        unsigned char gresp[2]{};
        if (recv(sock, (char*)gresp,2,0) != 2 || gresp[1] != 0x00) {
            last_error_ = "socks auth not accepted";
            closesocket(sock);
            WSACleanup();
            return false;
        }
        std::vector<unsigned char> req;
        req.push_back(0x05);
        req.push_back(0xF0);
        req.push_back(0x00);
        req.push_back(0x03);
        req.push_back((unsigned char)host.size());
        req.insert(req.end(), host.begin(), host.end());
        req.push_back(0x00);
        req.push_back(0x00);

        if ( send(sock, (const char*)req.data(), (int)req.size(),0) != (int)req.size()) {
            last_error_= "RESOLVE send failed";
            closesocket(sock);
            WSACleanup();
            return false;
        }

        unsigned char header[4]{};
        if (recv(sock, (char*)header,4,0) != 4 || header[1] != 0x00) {
            last_error_ = "RESOLVED failed";
            closesocket(sock);
            WSACleanup();
            return false;
        }
        unsigned char atyp =header[3];
        char ipbuf[INET6_ADDRSTRLEN]{};

        if (atyp == 0x01) {
            unsigned char addr4[4]{};
            if (recv(sock,(char*)addr4,4,0) != 4) {
                last_error_ = "IPV$ read failed";
                closesocket(sock);
                WSACleanup();
                return false;
            }
            inet_ntop(AF_INET, addr4, ipbuf, sizeof(ipbuf));
        }else if (atyp == 0x04) {
            unsigned char addr6[16] {};
            if ( recv(sock, (char*)addr6, 16, 0) != 16) {
                last_error_= "IPV^ read failed";
                closesocket(sock);
                WSACleanup();
                return false;
            }
            inet_ntop(AF_INET6, addr6, ipbuf, sizeof(ipbuf));
        }else {
            last_error_ = "UNKNOWN ATYP";
            closesocket(sock);
            WSACleanup();
            return false;
        }
        unsigned char portbuf[2]{};
        recv(sock, (char*)portbuf, 2,0);

        out_ip = ipbuf;
        closesocket(sock);
        WSACleanup();
        cache_[host] = {out_ip, std::chrono::steady_clock::now() +ttl_};
        return true;
    }
    void dnsResolver::clear_cache() {
        cache_.clear();
    }

}