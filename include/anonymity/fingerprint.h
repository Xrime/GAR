//
// Created by xint2 on 10/05/2026.
//

#ifndef GAR_FINGERPRINT_H
#define GAR_FINGERPRINT_H
#include <string>
#include <map>
#include  <vector>

namespace gar::anonymity {
    struct fingerprintProfile {
        std::string user_agent;
        std::string accept_language;
        std::string accept_encoding;
        std::map<std::string, std::string> extra_headers;
    };

    class Fingerprint {
    public:
        Fingerprint();
        const fingerprintProfile& current() const;

        void rotate();
        std::map<std::string, std::string> build_headers() const;
    private:
        std::vector<fingerprintProfile> profiles_;
        fingerprintProfile active_;
        void load_profiles();
    };
}
#endif //GAR_FINGERPRINT_H