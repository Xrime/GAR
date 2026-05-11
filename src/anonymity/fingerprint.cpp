//
// Created by xint2 on 10/05/2026.
//
#include "../../include/anonymity/fingerprint.h"
#include <random>
namespace gar::anonymity {
    Fingerprint::Fingerprint() {
        load_profiles();
        rotate();
    }
    void Fingerprint::load_profiles() {
        profiles_ ={
            {"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
        "en-US,en;q=0.9", "gzip, deflate, br"},
       {"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:124.0) Gecko/20100101 Firefox/124.0",
        "en-US,en;q=0.5", "gzip, deflate, br"},
       {"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15",
        "en-US,en;q=0.9", "gzip, deflate, br"}
        };
    }
    void Fingerprint::rotate(){
        if (profiles_.empty())  return;
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0,(int)profiles_.size() -1);
        active_ =profiles_[dist(gen)];

    }
    const fingerprintProfile &Fingerprint::current() const {
        return active_;
    }
    std::map<std::string, std::string> Fingerprint::build_headers() const {
        return {
            {"User-Agent", active_.user_agent},
            {"Accept-Language", active_.accept_language},
            {"Accept-Encoding", active_.accept_encoding}
        };
    }
}