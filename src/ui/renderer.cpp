//
// Created by xint2 on 30/04/2026.
//
#include "../include/ui/renderer.h"
#include <string>
#include <regex>

namespace gar::renderer {
    std::string Renderer::htmlToText(const std::string &html) {
        std::string text = html;


        text = std::regex_replace(text, std::regex("<script[^>]*>[\\s\\S]*?</script>", std::regex::icase), "");
        text = std::regex_replace(text, std::regex("<style[^>]*>[\\s\\S]*?</style>", std::regex::icase), "");

        text = std::regex_replace(text, std::regex("<br\\s*/?>", std::regex::icase), "\n");
        text = std::regex_replace(text, std::regex("</p>", std::regex::icase), "\n");
        text = std::regex_replace(text, std::regex("</h1>", std::regex::icase), "\n\n");
        text = std::regex_replace(text, std::regex("</h2>", std::regex::icase), "\n\n");
        text = std::regex_replace(text, std::regex("</h3>", std::regex::icase), "\n\n");
        text = std::regex_replace(text, std::regex("</div>", std::regex::icase), "\n");
        text = std::regex_replace(text, std::regex("<[^>]+>"), "");
        text = std::regex_replace(text, std::regex("&nbsp;")," ");
        text =std::regex_replace(text, std::regex("&amp;"), "&");
        text = std::regex_replace(text, std::regex("&lt;"), "<");
        text = std::regex_replace(text, std::regex("&gt;"), ">");
        text = std::regex_replace(text,std::regex("&quot;"), "\"");
        text = std::regex_replace(text, std::regex("\n{3,}"), "\n\n");
        text = std::regex_replace(text, std::regex("[\t]{2,}")," ");

        while (!text.empty() && (text.front() == '\n' || text.front()==' ')) {
            text.erase(text.begin());
        }
        while (!text.empty() && (text.back() == '\n' || text.back()== ' ')) {
            text.pop_back();
        }
        return text;
    }

}