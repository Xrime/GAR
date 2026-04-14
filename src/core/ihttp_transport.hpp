#pragma once

#include <string>
#include <map>

namespace gar::core {
    struct HttpResponse {
        int status =0;
        std::string boby;
        std::map<std::string,std::string> headers;

    };
    struct HttpRequest {
        std::string url;
        std::string method = "GET";
        std::map<std::string,std::string > headers;
        std::string body;

    };
    class IHttpTransport {
    public:
        virtual ~IHttpTransport() =default;
        virtual HttpResponse performRequest(const HttpRequest& req) =0;

    };
}