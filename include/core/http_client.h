#pragma once
#include <string>
#include <map>
#include "http_response.h"
#include "http_request.h"



namespace gar::core {
    class IHttpTransport;
    class HttpRequest;
    class HttpResponse;
    class HttpClient {
    public:
        HttpClient(const std::string& socks5_host = "127.0.0.1", int socks5_port= 9050);

        ~HttpClient();

        HttpResponse get(const std::string & url);
        HttpResponse post(const std::string& url,
                          const std::string& body,
                          const std::map<std::string,std::string> & headers ={});

        void setHeader(const std::string& key, const std::string& value);

        bool isConnected() const;
        std::string getLastError() const;

    private:
        std::map<std::string, std::string> default_headers_;
        bool is_connected;
        std::string last_error;
        std::string socks5Host;
        int socks5Port;


        // void validateUrl(const std::string& url) const; // I replace with parseurl
        bool parseurl(const std::string& url, std::string& host, std::string& path, int& port);
        HttpRequest buildRequest(const std::string& url, const std::string& method, const std::string& body = "");
        HttpResponse performRequest(const HttpRequest& request);

        void seterror(const std::string& error);
    };

}
