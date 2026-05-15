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
    static bool has_header(const std::map<std::string, std::string>& h, const std::string& key) {
        auto it = h.find(key);
        return it != h.end() && !it->second.empty();
    }
    HeaderReport HeaderAnalzer::analyze(const std::map<std::string, std::string> &headers) {
        HeaderReport report;
        report.score = 100;

        // const std::vector<std::string> required ={
        //     "content-security-policy",
        //     "strict-transport-security",
        //     "x-content-type-options",
        //     "x-frame-options",
        //     "referrer-policy",
        //     "permissions-policy"
        // };
        std::map<std::string, std::string> lower;
        for (const auto& kv : headers) {
            lower[to_lower(kv.first)] = kv.second;
        }
        // for (const auto& h : required) {
        //     if (lower.find(h) != lower.end()) {
        //         report.present.push_back(h);
        //     }else {
        //         report.missing.push_back(h);
        //     }
        // }
        // return report;

        check_required(lower, report);
        check_misconfig(lower, report);

        if (report.score < 0)report.score = 0;
        if (report.score> 100) report.score = 100;
        return report;
    }
    void HeaderAnalzer::check_required(const std::map<std::string, std::string> &headers, HeaderReport &report) {
        if (!has_header(headers, "strict-transport-security")) {
            report.score -= 20;
            report.issues.push_back({"Strict-Transport-Security", "high", "Missing HSTS. HTTPS connection may be downgraded."});
        }
        if (!has_header(headers, "content-security-policy")) {
            report.score -= 20;
            report.issues.push_back({"Content-Security-Policy", "high",
                "Missing CSP. Increases XSS injection risk."});
        }
        if (!has_header(headers, "x-content-type-options")) {
            report.score -= 10;
            report.issues.push_back({"X-Content-Type-Options", "medium",
                "Missing nosniff protection."});
        }
        if (!has_header(headers, "x-frame-options")) {
            report.score -= 10;
            report.issues.push_back({"X-Frame-Options", "medium",
                "Missing clickjacking protection."});
        }
        if (!has_header(headers, "referrer-policy")) {
            report.score -= 5;
            report.issues.push_back({"Referrer-Policy", "low",
                "Referrer leakage possible."});
        }
        if (!has_header(headers, "permissions-policy")) {
            report.score -= 5;
            report.issues.push_back({"Permissions-Policy", "low",
                "Browser features not restricted."});
        }
    }
    void HeaderAnalzer::check_misconfig(const std::map<std::string, std::string> &headers, HeaderReport &report) {
        auto it = headers.find("strict-transport-security");
        if (it != headers.end() && it-> second.find("max-age=") == std::string::npos) {
            report.score -= 10;
            report.issues.push_back({"Strict-Transport-Security","medium","HSTS present but missing maxi-age"});

        }
    }



}