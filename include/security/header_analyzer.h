//
// Created by xint2 on 06/05/2026.
//

#ifndef GAR_HEADER_ANALYZER_H
#define GAR_HEADER_ANALYZER_H
#include <map>
#include <string>
#include <vector>
namespace gar::security {
    struct HeaderReport {
        std::vector<std::string> present;
        std::vector<std::string> missing;
    };
    class HeaderAnalzer {
    public:
        static HeaderReport analyze(const std::map<std::string, std::string>& headers);
    };
}
#endif //GAR_HEADER_ANALYZER_H