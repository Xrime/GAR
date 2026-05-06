//
// Created by xint2 on 06/05/2026.
//
#include "../../include/security/header_analyzer.h"
#include <algorithm>

namespace gar::security {
    static std::string to_lower(std::string s) {
        std::transform(s.begin(), s.end(),s.begin(), ::tolower);
        return s;
    }
    HeaderReport HeaderAnalzer::analyze(const std::map<std::string, std::string> &headers) {
        HeaderReport report;
        const std::vector<std::string> required ={
            "content-security-policy",
            "strict-transport-security",
            "x-content-type-options",
            "x-frame-options",
            "referrer-policy",
            "permissions-policy"
        };
        std::map<std::string, std::string> lower;
        for (const auto& kv : headers) {
            lower[to_lower(kv.first)] = kv.second;
        }
        for (const auto& h : required) {
            if (lower.find(h) != lower.end()) {
                report.present.push_back(h);
            }else {
                report.missing.push_back(h);
            }
        }
        return report;
    }

}