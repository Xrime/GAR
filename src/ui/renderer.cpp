//
// Created by xint2 on 30/04/2026.
//
#include "../include/ui/renderer.h"
#include <string>
#include  <gumbo.h>
#include <regex>

namespace gar::renderer {
    // std::string Renderer::htmlToText(const std::string &html) {
    //     std::string text = html;
    //
    //
    //     text = std::regex_replace(text, std::regex("<script[^>]*>[\\s\\S]*?</script>", std::regex::icase), "");
    //     text = std::regex_replace(text, std::regex("<style[^>]*>[\\s\\S]*?</style>", std::regex::icase), "");
    //
    //     text = std::regex_replace(text, std::regex("<br\\s*/?>", std::regex::icase), "\n");
    //     text = std::regex_replace(text, std::regex("</p>", std::regex::icase), "\n");
    //     text = std::regex_replace(text, std::regex("</h1>", std::regex::icase), "\n\n");
    //     text = std::regex_replace(text, std::regex("</h2>", std::regex::icase), "\n\n");
    //     text = std::regex_replace(text, std::regex("</h3>", std::regex::icase), "\n\n");
    //     text = std::regex_replace(text, std::regex("</div>", std::regex::icase), "\n");
    //     text = std::regex_replace(text, std::regex("<[^>]+>"), "");
    //     text = std::regex_replace(text, std::regex("&nbsp;")," ");
    //     text =std::regex_replace(text, std::regex("&amp;"), "&");
    //     text = std::regex_replace(text, std::regex("&lt;"), "<");
    //     text = std::regex_replace(text, std::regex("&gt;"), ">");
    //     text = std::regex_replace(text,std::regex("&quot;"), "\"");
    //     text = std::regex_replace(text, std::regex("\n{3,}"), "\n\n");
    //     text = std::regex_replace(text, std::regex("[\t]{2,}")," ");
    //
    //     while (!text.empty() && (text.front() == '\n' || text.front()==' ')) {
    //         text.erase(text.begin());
    //     }
    //     while (!text.empty() && (text.back() == '\n' || text.back()== ' ')) {
    //         text.pop_back();
    //     }
    //     return text;
    // }

    static void appendText(GumboNode* node, std::string&);
    static void appendChildren(GumboVector* children, std::string& out) {
        for (unsigned int i=0; i< children->length; ++i) {
            appendText(static_cast<GumboNode*> (children->data[i]),out);
        }
    }
    void appendText(GumboNode* node, std::string& out) {
        if (node -> type == GUMBO_NODE_TEXT) {
            out.append(node ->v.text.text);
            out.push_back(' ');
        }else if (node-> type == GUMBO_NODE_ELEMENT) {
            GumboTag tag =node ->v.element.tag;
            if (tag==GUMBO_TAG_SCRIPT || tag ==GUMBO_TAG_STYLE) {
                return;
            }
            appendChildren(&node ->v.element.children, out);


            if (tag == GUMBO_TAG_P || tag == GUMBO_TAG_BR ||
                tag == GUMBO_TAG_DIV || tag == GUMBO_TAG_H1 || tag == GUMBO_TAG_H2
                || tag == GUMBO_TAG_H3 || tag == GUMBO_TAG_H4 ||
                tag == GUMBO_TAG_H5 || tag == GUMBO_TAG_H6 ||
                tag == GUMBO_TAG_LI) {
                out.push_back('\n');
                }
        }
    }
    std::string Renderer::htmlToText(const std::string &html) {
        GumboOutput* output =gumbo_parse(html.c_str());
        std::string out;
        out.reserve(html.size()/2);
        appendText(output->root, out);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        return out;
    }




}