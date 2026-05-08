//
// Created by xint2 on 08/05/2026.
//

#ifndef GAR_DNS_RESOLVER_H
#define GAR_DNS_RESOLVER_H
#include  <string>

namespace gar::core {
    class DnsResolver {
    public:
        dnsResolver(const std::string& socks_host="127.0.0.1",int socks_port = 9050);
        bool resolve(const std::string& host, std::string& out_ip);
        std::string last_error() const {return last_error_;}

    private:
        std::string host_;
        int port_;
        std::string last_error_;
    };
}
#endif //GAR_DNS_RESOLVER_H