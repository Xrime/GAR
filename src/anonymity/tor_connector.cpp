//
// Created by xint2 on 15/04/2026.
//

#include "../../include/anonymity/tor_connector.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
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

    }
    TorConnector::~TorConnector() {
        std::cout<<"Torconnection is destroy"<<std::endl;

    }

    bool TorConnector::testconnection() {
        std::cout<<"TorConnector :testing connection to ..."<<socks5_host_<<":"<<socks5_port_<<std::endl;
        #ifdef _WIN32
                WSADATA wsa_data;
                int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

                if (wsa_result != 0) {
                    setError("WSAStartup failed");
                    std::cout << "[TorConnector] WSAStartup failed with error: " << wsa_result << std::endl;
                    return false;
                }
                std::cout << "[TorConnector] WSAStartup successful" << std::endl;
        #endif
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock== INVALID_SOCKET) {
            setError("Failed to connect");
            #ifdef _WIN32
                        WSACleanup();
            #endif
                        return false;
                    }


        std::cout<<"Tor socket created"<<std::endl;


        struct sockaddr_in tor_addr={};
        tor_addr.sin_family=AF_INET;
        tor_addr.sin_port= htons(socks5_port_);

        int addr_result = inet_pton(AF_INET, socks5_host_.c_str(), &tor_addr.sin_addr);
        if (addr_result<=0) {
            setError("Invalid tor address");
            #ifdef _WIN32
                winCleanUP(sock);
            #else
                close(sock)
            #endif
            return false;

        }
        std::cout <<"Torconnector Attempting connection..."<<socks5_host_<<": "<<socks5_port_<<std::endl;
        int connect_result = connect(sock, (struct sockaddr*)&tor_addr, sizeof(tor_addr));

        #ifdef _WIN32
            if (connect_result==SOCKET_ERROR) {
                int error =WSAGetLastError();
                setError("connection failed");
                std::cerr<<"Torconnector connection error"<<error<<std::endl;
                winCleanUP(sock);
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
                        winCleanUP(sock);
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
                       winCleanUP(sock);
            #else
                        close(sock);
            #endif
                        return false;
        }
        //checking response
        if (response[0] != 0x05 || response[1] != 0x00) {
            setError("Tor SOCK5 handshake failed");
            #ifdef _WIN32
                winCleanUP(sock);
            #else
                        close(sock);
            #endif
                        return false;
        }
        std::cout << "Torconnector socks5 handshake is successfule"<<std::endl;
        #ifdef _WIN32
               winCleanUP(sock);
        #else
                close(sock);
        #endif

        is_connected_ =true;
        last_error_="";
        std::cout<<"..TorConnector.. Tor connection is verified"<<std::endl;
        return true;

    }
    //geting socks5 adr
    std::string TorConnector::getSocks5Address() const {
        std::stringstream ss;
        ss<<socks5_host_<<":"<<socks5_port_;
        return ss.str();
    }

    bool TorConnector::isConnected() const {
        return is_connected_;
    }
    // std::string TorConnector::gethost() const {
    //     return socks5_host_;
    // }
    // int TorConnector::getPort() const {
    //     return socks5_port_;
    // }
    std::string TorConnector::getLastError() const {
        return last_error_;
    }
    void TorConnector::setError(const std::string &error) {
        last_error_ = error;
        std::cerr<<"Torconnector error ::"<<error<<std::endl;
    }
    void TorConnector::winCleanUP(SOCKET sock) {
        closesocket(sock);
        WSACleanup();
    }



}