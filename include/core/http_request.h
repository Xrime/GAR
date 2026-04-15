//
// Created by xint2 on 15/04/2026.
//

#ifndef GAR_HTTP_REQUEST_H
#define GAR_HTTP_REQUEST_H
#include <string>
#include <map>

namespace gar::core {
    struct HttpRequest {
        std::string url;

        std::string method;

        std::string body;

        std::map<std::string, std::string> headers;

        int timeout_ms=30000;
    };
}
#endif //GAR_HTTP_REQUEST_H