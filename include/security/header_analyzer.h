//
// Created by xint2 on 06/05/2026.
//

#ifndef GAR_HEADER_ANALYZER_H
#define GAR_HEADER_ANALYZER_H
#include <map>
#include <string>
#include <vector>
namespace gar::security {
    struct HeaderIssue {
        std::string name;
        std::string severity;
        std::string message;
    };
    struct HeaderReport {
        int score;
        std::vector<HeaderIssue> issues;
    };
    class HeaderAnalzer {
    public:
        static HeaderReport analyze(const std::map<std::string, std::string>& headers);

    private:
        static void check_required(const std::map<std::string, std::string>& headers, HeaderReport& report);
        static void check_misconfig(const std::map<std::string, std::string>& headers, HeaderReport& report);
    };
}
#endif //GAR_HEADER_ANALYZER_H