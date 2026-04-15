//
// Created by xint2 on 15/04/2026.
//

#ifndef GAR_HTTP_RESPONSE_H
#define GAR_HTTP_RESPONSE_H
#include <string>
#include <map>

namespace gar::core {
    struct HttpResponse {
        int status_code;

        std::string status_message;

        std::string body;

        std::map<std::string ,std::string>headers;

        bool success;

        std::string error_message;

    };
}
#endif //GAR_HTTP_RESPONSE_H