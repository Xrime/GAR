//
// Created by xint2 on 08/05/2026.
//

#ifndef GAR_DNS_RESOLVER_H
#define GAR_DNS_RESOLVER_H
#include  <string>
#include <unordered_map>
#include <chrono>
namespace gar::core {
    class dnsResolver {
    public:
        dnsResolver(const std::string& socks_host="127.0.0.1",int socks_port = 9050);
        bool resolve(const std::string& host, std::string& out_ip);
        std::string last_error() const {return last_error_;}
        struct CacheEntry {
            std::string ip;
            std::chrono::steady_clock::time_point expires_at;
        };
        void clear_cache();





    private:
        std::string host_;
        int port_;
        std::string last_error_;
        std::unordered_map<std::string, CacheEntry> cache_;
        std::chrono::seconds ttl_{300};

    };
}
#endif //GAR_DNS_RESOLVER_H