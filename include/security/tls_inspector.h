//
// Created by xint2 on 12/05/2026.
//

#ifndef GAR_TLS_INSPECTOR_H
#define GAR_TLS_INSPECTOR_H
#include <string>

namespace gar::security {
    struct TLSReport {
        bool success;
        std::string subject;
        std::string issuer;
        std::string not_before;
        std::string not_after;
        int days_left;
        std::string error;
    };
    class TLSInspector {
    public:
        TLSReport inspect(const std::string& host, int port = 443, const std::string& socks_host = "127.0.0.1", int socks_port =9050);
    };
}
#endif //GAR_TLS_INSPECTOR_H