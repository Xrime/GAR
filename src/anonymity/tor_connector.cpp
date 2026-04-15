//
// Created by xint2 on 15/04/2026.
//

#include "../../include/anonymity/tor_connector.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

namespace gar::anonymity {

    TorConnector::TorConnector(const std::string &socks5_host, int socks5_port)
        : socks5_host_(socks5_host),
        socks5_port_(socks5_port),
        is_connected_(false),
        last_error_("")

    {
        std::cout <<"[TorConnector] created with"<< socks5_host<<":"<<socks5_port<<std::endl;
        #ifdef _WIN32
            WSADATA wsa_data;
            int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
            if (result !=0) {
                setError("wsastartup failed");
                std::cerr<<"[Torconnector] wsastartup error :"<<result<<std::endl;
            }
        #endif

    }
    TorConnector::~TorConnector() {
        std::cout<<"Torconnection is distroy"<<std::endl;
        #ifdef _WIN32
            WSACleanup();
        #endif
    }

    bool TorConnector::testconnection() {
        std::cout<<"TorConnector :testing connection to ..."<<socks5_host_<<":"<<socks5_port_<<std::endl;

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        #ifdef _WIN32
            if (sock== INVALID_SOCKET) {
                setError("Failed to connect");
                return false;
            }
        #else
            if (sock < 0) {
                setError("Failed to create socket");
                return false;
            }
        #endif


        struct sockaddr_in tor_addr;
        tor_addr.sin_family=AF_INET;
        tor_addr.sin_port= htons(socks5_port_);

        int addr_result = inet_pton(AF_INET, socks5_host_.c_str(), &tor_addr.sin_addr);
        if (addr_result<=0) {
            setError("Invalid tor address");
            #ifdef _WIN32
                closesocket(sock);
            #else
                close(sock)
            #endif
            return false;

        }
        std::cout <<"Torconnector Attempting connection..."<< std::endl;
        int connect_result = connect(sock, (struct sockaddr*)&tor_addr, sizeof(tor_addr));

        #ifdef _WIN32
            if (connect_result==SOCKET_ERROR) {
                int error =WSAGetLastError();
                setError("connection failed");
                std::cerr<<"Torconnector connection error"<<error<<std::endl;
                closesocket(sock);
                return false;
        }
        #else
            if (connect_result < 0 ){
                setError("connection failed");
                close(sock);
                return false;
            }
        #endif
                std::cout << "TorConnector connected to tor SOCK5!!!" << std::endl;
        unsigned char handshake[]= {0x05, 0x01 , 0x00};

        int send_result = send(sock, (const char*)handshake, 3,0);
        if (send_result!=3) {
            setError("Failed to send handshake");
            #ifdef _WIN32
                        closesocket(sock);
            #else
                        close(sock);
            #endif
                        return false;
        }
        std::cout<<"Tor connection send handshake.... "<<std::endl;
        unsigned char response[2];
        int recv_result =recv(sock, (char*)response,2,0);
        if (recv_result!=2) {
            setError("Failed to recieve handshake responce");
            #ifdef _WIN32
                        closesocket(sock);
            #else
                        close(sock);
            #endif
                        return false;
        }
        //checking response
        if (response[0] != 0x05 || response[1] != 0x00) {
            setError("Tor SOCK5 handshake failed");

        }
    }

}