#include "../../include/core/http_client.h"

#include <chrono>

#include "../../include/core/http_request.h"
#include "../../include/core/http_response.h"
#include "../../include/core/ihttp_transport.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>
#include <curl/curl.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


// static std::string socks5_reply_totext(unsigned char rep) {
//     switch (rep) {
//         case 0x00: return "success";
//         case 0x01: return "socks server failure";
//         case 0x02: return "connection not allowed";
//         case 0x03: return "network not reachable";
//         case 0x04: return "host not reachable";
//         case 0x05: return "connection refused by the host";
//         case 0x06: return "TLL expired";
//         case 0x07: return "command not supported";
//         case 0x08: return "address type not surported";
//         default: return "Unknown socks5 error";
//     }
// }
static void close_socket (SOCKET sock) {
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

}

static size_t write_callback(void* contents, size_t size, size_t num_ofmeb, void* userp) {
    size_t total = size * num_ofmeb;
    std::string* out = (std::string*)userp;
    out ->append((char*)contents, total);
    return total;
}

namespace gar::core {
    HttpClient::HttpClient(const std::string &socks5_host, int socks5_port)
        :is_connected(false),
        last_error(""),
        socks5Host(socks5_host),
        socks5Port(socks5_port)

        {
        std::cout << "Http client created "<<socks5_host<<":"<< socks5_port<<std::endl;

        //create default headers
        setHeader("User-Agent", "GAR");
        setHeader("Connection","close");
    }

    HttpClient::~HttpClient() {
        std::cout << "Httpclient Destroyed"<<std::endl;
    }
    HttpResponse HttpClient::get(const std::string& url) {

        if (url.empty()) {
            seterror("Url can't be empty");
            HttpResponse response;
            response.success=false;
            response.error_message =last_error;
            return response;

            }


        HttpRequest request =buildRequest(url, "GET");
        return performRequest(request);

        // for (const auto& header : headers) {
        //     request.headers[header.first] =header.second;
        // }
        // return performRequest(request);
        }
    HttpResponse HttpClient::post(const std::string &url, const std::string &body, const std::map<std::string, std::string> &headers) {
        if (url.empty()) {
            seterror("Url can't be empty");
            HttpResponse response;
            response.success=false;
            response.error_message =last_error;
            return response;

        }
        HttpRequest request =buildRequest(url,"POST", body);

        for (const auto& header : headers) {
            request.headers[header.first] = header.second;
        }
        return  performRequest(request);

    }


    void HttpClient::setHeader(const std::string &key, const std::string &value) {
        default_headers_[key] =value;
    }


    bool HttpClient::isConnected() const {
        return is_connected;
    }
    std::string HttpClient::getLastError() const {
        return last_error;
    }
    bool HttpClient::parseurl(const std::string &url, std::string &host, std::string &path, int &port) {
        port =80;
        size_t protocol_position = url.find("://");
        if (protocol_position==std::string::npos) {
            seterror("Invalid URL format ://");
            return false;
        }
        std::string protocol = url.substr(0, protocol_position);

        if (protocol =="https") {
            port = 443;
        }else if (protocol =="http") {
            port=80;
        }else {
            seterror("Invalid protocol"+protocol);
            return false;
        }
        size_t host_start = protocol_position + 3;
        size_t host_end = url.find('/', host_start);
        size_t port_position = url.find(':', host_start);

        if ((port_position != std::string::npos) && (host_end== std::string::npos ||port_position<host_end)) {
            host = url.substr(host_start, port_position-host_start);
            size_t port_end = host_end;
            if (port_end == std::string::npos) {
                port_end =url.length();
            }

            std::string port_str = url.substr(port_position+1,port_end - port_position -1);
            try {
                port = std::stoi(port_str);
            }catch (...) {
                seterror("Invalid port number: "+ port_str);
                return false;
            }
        }else {
            if (host_end== std::string::npos) {
                host = url.substr(host_start);
            }
            else {
                host = url.substr(host_start, host_end-host_start);
            }
        }
        if (host_end == std::string::npos) {
            path="/";
        }else {
            path = url.substr(host_end);
            if (path.empty()) {
                path = "/";
            }
        }
        return true;
    }
    HttpRequest HttpClient::buildRequest(const std::string& url, const std::string& method, const std::string& body) {
        HttpRequest request;
        request.url = url;
        request.method =method;
        request.body = body;
        std::string host, path;
        int port;

        if (!parseurl(url, host,path, port)) {
            return request;
        }

        for (const auto& header : default_headers_) {
            request.headers[header.first] = header.second;
        }
         request.headers["Host"] =host;

        if (method == "POST" && !body.empty()) {
            request.headers["Content-Length"] = std::to_string(body.length());
        }
        return request;
    }

    HttpResponse HttpClient::performRequest(const HttpRequest &request) {
        HttpResponse response;
        response.success = false;
        response.status_code = 0;
        response.status_message="";
        response.body ="";

        std::string host, path;
        int port;



        if (!parseurl(request.url, host, path, port)) {
            response.error_message = last_error;
            return response;
        }
        #ifdef _WIN32
                WSADATA wsa_data;
                int wsa_result =WSAStartup(MAKEWORD(2,2), &wsa_data);
                if (wsa_result !=0) {
                    seterror("WSAStartup failed");
                    response.error_message =last_error;
                    return response;
                }
        #endif

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        #ifdef _WIN32
                if (sock == INVALID_SOCKET) {
                    seterror("Unable to create socket");
                    WSACleanup();
                    response.error_message =last_error;
                    return response;
                }
        #else
                if (sock <0) {
                    seterror("Unable to create socket");
                    response.error_message = last_error;
                    return response;
                }
        #endif

        if (port == 443) {
            CURL* curl =curl_easy_init();
            if (!curl) {
                seterror("Fail to initialize curl for https");
                response.error_message =last_error;
                return response;
            }
            std::string response_data;
            std::string proxy =socks5Host +":" +std::to_string(socks5Port);

            curl_easy_setopt(curl , CURLOPT_URL, request.url.c_str());
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

            struct curl_slist* curl_headers = nullptr;
            for (const auto& h : request.headers) {
                std::string line = h.first + ":"+ h.second;
                curl_headers = curl_slist_append(curl_headers, line.c_str());
            }
            if (curl_headers) {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
            }
            CURLcode res = curl_easy_perform(curl);
            std::cout << "[HTTPS] Proxy = " << proxy << std::endl;

            if (res != CURLE_OK) {
                seterror(std::string("Https request failed ") +  curl_easy_strerror(res));
                response.error_message = last_error;
                if (curl_headers) curl_slist_free_all(curl_headers);
                curl_easy_cleanup(curl);
                return response;
            }
            long code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

            response.success =true;
            response.status_code=(int)code;
            response.status_message="ok";
            response.body = response_data;

            if (curl_headers) {
                curl_slist_free_all(curl_headers);
            }
            curl_easy_cleanup(curl);

            std::cout<<"Https connectedr"<<std::endl;
            return response;
        }
        struct sockaddr_in socks5_addr ={};
        socks5_addr.sin_family = AF_INET;
        socks5_addr.sin_port = htons(socks5Port);
        int addr_result = inet_pton(AF_INET, socks5Host.c_str(),&socks5_addr.sin_addr);



        if (addr_result <= 0) {
            seterror("Wrong SOCK5 address");
            close_socket(sock);
            response.error_message = last_error;
            return response;


        }
        //tor connection through sock5s
        int connect_result = connect(sock, (struct sockaddr*)&socks5_addr,sizeof(socks5_addr));
        #ifdef _WIN32
                if (connect_result== SOCKET_ERROR) {
                    seterror("Unable to connect to tor SOCKS5");
                    closesocket(sock);
                    WSACleanup();
                    response.error_message = last_error;
                    return response;
                }
        #else
                if (connect_result < 0) {
                    seterror("Unable to connect to tor SOCKS5");
                    close(sock);
                    response.error_message = last_error;
                    return response;
                }
        #endif
        unsigned char socks5_handshake[] = {0x05,0x01,0x00};
        int handshake_send =send(sock, (const char*)socks5_handshake,3,0);
        if (handshake_send != 3) {
            seterror("Failed to send SOCKS5 handshake");
            close_socket(sock);
            response.error_message = last_error;
            return response;
        }

        unsigned char socks5_response[2] ={0};
        int receive_result =recv(sock, (char*)socks5_response,2,0);

        if (receive_result !=2) {
            seterror("Unable to receive SOCKS5 response");
            close_socket(sock);

            response.error_message = last_error;
            return response;
        }
        if (socks5_response[0] != 0x05 || socks5_response[1] !=0x00) {
            seterror("SOCKS HANDSHAKE FAILED!");
            close_socket(sock);
            response.error_message= last_error;
            return response;

        }



        std::vector<unsigned char> connect_cmd;
        connect_cmd.push_back(0x05);
        connect_cmd.push_back(0x01);
        connect_cmd.push_back(0x00);
        connect_cmd.push_back(0x03);
        connect_cmd.push_back((unsigned char)host.length());

        for (char c: host) {
            connect_cmd.push_back((unsigned char )c);
        }


        // unsigned short port_net = htons(port);
        // connect_cmd.push_back((port_net >>8)& 0xFF);
        // connect_cmd.push_back(port_net & 0xFF);

        connect_cmd.push_back((port >>8)  & 0xFF);
        connect_cmd.push_back(port & 0xFF);

        bool connect_ok = false;
        unsigned char connect_response[10] = {0};

        for (int attempt =1; attempt<=5;attempt++) {
            int connect_send = send(sock , (const char*)connect_cmd.data(), connect_cmd.size(),0);

            if (connect_send != (int)connect_cmd.size()) {
                seterror("Failed to send SOCKS5 connect command");
                break;
            }

            int connect_recv = recv(sock, (char*)connect_response, 10,0);
            std::cout << "HttpClient SOCKS5 attempt: " << attempt << std::endl;
            std::cout << "HttpClient connect recv bytes"<< connect_recv << std::endl;


            if (connect_recv>=2) {
                std::cout << "[HttpClient] Response byte 0: 0x" << std::hex << (int)connect_response[0] << std::dec << std::endl;
                std::cout << "[HttpClient] Response byte 1: 0x" << std::hex << (int)connect_response[1] << std::dec << std::endl;

            }
            if (connect_recv >= 2 && connect_response[0] ==0x05 && connect_response[1]==0x00) {
                connect_ok =true;
                break;
            }
            if (!connect_ok) {
                seterror("SOCKS CONNECT failed after retries");
                close_socket(sock);

                response.error_message =last_error;
                return response;

            }


        }

        //building http request string
        std::stringstream http_request;
        http_request << request.method << " " << path << " HTTP/1.1\r\n";

        for (const auto& header : request.headers) {
            http_request << header.first << ":" << header.second<<"\r\n";
        }
        http_request << "\r\n";

        if (!request.body.empty()) {
            http_request<<request.body;
        }
        std::string http_request_str = http_request.str();
        int http_send = send(sock, http_request_str.c_str(), http_request_str.length(),0);

        if (http_send <= 0) {
            seterror("Failed to send http request");
            close_socket(sock);

            response.error_message =last_error;
            return response;

        }
        std::string full_response;
        char buffer[4096];
        int bytes_received;
        bool headers_found = false;

        while (true) {
            bytes_received = recv(sock, buffer, sizeof(buffer),0);
            if (bytes_received<=0) {
                break;
            }
            full_response.append(buffer, bytes_received);

            if (!headers_found && full_response.find("\r\n\r\n") !=std::string::npos) {
                headers_found =  true;
            }
        }
        close_socket(sock);

        size_t headers_end =full_response.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            seterror("Invalid Http response due to missing headers");
            response.error_message = last_error;
            return response;

        }
        std::string headers_part = full_response.substr(0, headers_end);
        std::string body_par = full_response.substr(headers_end +4);


        size_t first_newline = headers_part.find("\r\n");
        if (first_newline == std::string::npos) {
            seterror("Invalid Http response due to missing status line");
            response.error_message =last_error;
            return response;
        }

        std::string status_line = headers_part.substr(0, first_newline);

        size_t space1 = status_line.find(' ');
        size_t space2 = status_line.find(' ',space1+1);

        if (space1 != std::string::npos && space2 != std::string::npos) {
            std::string status_code_str = status_line.substr(space1 +1, space2 - space1 -1);
            std::string status_message = status_line.substr(space2 + 1);

            try {
                response.status_code = std::stoi(status_code_str);
                response.status_message = status_message;
            }catch (...) {
                seterror("failed to parse status code");
                response.error_message = last_error;
                return response;
            }
        }
        std::istringstream headers_stream(headers_part);
        std::string header_line;
        bool first = true;

        while (std::getline(headers_stream, header_line)) {
            if (first) {
                first = false;
                continue;
            }
            if (!header_line.empty() && header_line.back() == '\r') {
                header_line.pop_back();
            }
            if (header_line.empty()) {
                break;
            }
            size_t colon_pos = header_line.find(':');
            if (colon_pos!= std::string::npos) {
                std::string key = header_line.substr(0, colon_pos);
                std::string value =header_line.substr(colon_pos + 1);

                size_t start = value.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    value = value.substr(start);
                }
                response.headers[key] = value;
            }
        }
        response.body = body_par;
        response.success =true;
        is_connected = true;

        std::cout<<" httpclient response parsed successfully "<<std::endl;
        std::cout<<"Httpclient status: "<< response.status_code<<" "<< response.status_message<<std::endl;
        std::cout<<"Httpclient body size"<<response.body.length()<<"bytes"<<std::endl;

        return response;
    }
    void HttpClient::seterror(const std::string &error) {
        {
            last_error =error;
            std::cerr<< "Httpclient error"<<error<<std::endl;


        }
    }



}

