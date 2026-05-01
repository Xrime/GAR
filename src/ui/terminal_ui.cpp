//
// Created by xint2 on 25/04/2026.
//
#include "../../include/ui/terminal_ui.h"

#include <gumbo.h>
#include <iostream>
#include <regex>
#include "../include/ui/renderer.h"
#include <gumbo.h>
#include <functional>

static  std::string normalize_url(std::string input) {
    while (!input.empty() && (input.front()==' ' || input.front()=='\t')) {
        input.erase(input.begin());
    }
    while (!input.empty() && (input.back() ==' ' || input.back() == '\t' || input.back()=='\n')) {
        input.pop_back();
    }
    if (input.empty()) return input;

    if (input.rfind("http://", 0)==0 || input.rfind("https://",0)==0) {
        return input;
    }
    return "https://"+input;
}

namespace gar::terminal_ui {
    std::string TerminalUI::make_absolute_url(const std::string& base_url, const std::string& href) {
        if (href.empty()) return "";
        if (href.find("http://", 0)==0 || href.rfind("https://", 0)==0) {
            return  href;
        }
        if (href.rfind("//",0)==0) {
            return "https:" + href;
        }
        size_t scheme_pos = base_url.find("://");
        if (scheme_pos == std::string::npos) return href;

        size_t host_start = scheme_pos + 3;
        size_t path_start  = base_url.find('/', host_start);

        std::string root = (path_start == std::string::npos) ? base_url : base_url.substr(0,path_start);
        if (path_start == std::string::npos) {
            root = base_url.substr(0, path_start);
        }
        if (!href.empty() && href[0] =='/') {
            return root + href;
        }
        std::string base_dir = base_url;
        size_t last_slash = base_dir.find_last_of('/');

        if (last_slash != std::string::npos && last_slash > host_start) {
            base_dir = base_dir.substr(0, last_slash + 1);

        }else {
            base_dir = root + "/";
        }
        return base_dir + href;
    }

    TerminalUI::TerminalUI(gar::core::HttpClient& client) : http_client(client), history_index(-1) {

    }
    void TerminalUI::showBanner() {
        std::cout<<"\n=======================================\n";
        std::cout<<"                 GAR Browser               \n";
        std::cout<<"===========================================\n";

    }

    void TerminalUI::showHelp() {
        std::cout << "Commands: \n";
        std::cout<< "go <url> - open URL\n";
        std::cout << "back -previous page\n";
        std::cout << "forward -next page\n";
        std::cout << " help - show commad\n";
        std::cout << "quit - exit\n\n";
        std::cout << "open <n> - open link by by number from current page\n";
    }


    void TerminalUI::goToURL(const std::string &url, bool add_to_history) {
        core::HttpRequest request;
        request.url = url;
        request.method = "GET";
        request.headers["User-Agent"] = "GAR/1.0";
        request.headers["Accept"]="*/*";
        request.headers["Connection"] ="close";

        std::cout<<"\nLoading: "<<url<<std::endl;
        gar::core::HttpResponse response =http_client.performRequest(request);

        if (!response.success) {
            std::cout<<"FAILED: "<<response.error_message<<"\n";
            return;
        }
        last_url= url;
        last_status= response.status_code;
        last_size=response.body.size();
        std::cout<<"Status: "<<response.status_code<<" "<< response.status_message<<std::endl;
        std::cout<<"Body size"<< response.body.size()<<"bytes"<<std::endl;
        std::string clean = renderer::Renderer::htmlToText(response.body);
        std::cout<<clean.substr(0,1000000)<<"\n"<<std::endl;
        extract_links(response.body, url);
        show_links();

        std::cout << "\n[status]"<<last_url<<"|"<<last_status<<"|"<<last_size<<"bytes\n"<<std::endl;

        if (add_to_history) {
            if (history_index< (int)history.size()-1) {
                history.erase(history.begin() + history_index + 1, history.end());
            }
            history.push_back(url);
            history_index =(int)history.size()-1;

        }
    }
    void TerminalUI::goBack() {
        if (history_index <= 0) {
            std::cout <<"No back history. \n"<<std::endl;
            return;
        }
        history_index--;
        goToURL(history[history_index], false);

    }
    void TerminalUI::goForward() {
        if (history_index<0 || history_index>= (int)history.size()) {
            std::cout<< "No page to refresh.\n"<<std::endl;
            return;
        }
        goToURL(history[history_index], false);
    }
    void TerminalUI::refreshPage() {
        if (history_index < 0 || history_index>=(int)history.size()) {
            std::cout<<"No page to refresh,\n"<<std::endl;
            return;
        }
        goToURL(history[history_index],false);
    }

    void TerminalUI::run() {
        showBanner();
        showHelp();

        std::string line;
        while (true) {
            std::cout << "GAR";
            std::getline(std::cin, line);

            if (line=="quit") {
                std::cout<<"Exiting GAR..."<<std::endl;
                break;
            }else if (line == "help") {
                showHelp();
            }else if (line == "back") {
                goBack();
            }else if (line == "forward") {
                goForward();
            }else if (line =="refresh") {
                refreshPage();
            }
            else if (line.rfind("open", 0)==0) {
                std::string num = line.substr(5);
                try {
                    int idx = std::stoi(num);
                    open_linkby_index(idx);
                }catch (...) {
                    std::cout<<"invalid number format.\n"<< std::endl;
                }
            }else if (line.rfind("go ", 0) == 0) {
                std::string raw = line.substr(3);
                std::string url = normalize_url(raw);
                if (url.empty()) {
                    std::cout << "Enter a URL.\n"<< std::endl;
                }
                else {
                    goToURL(url, true);
                }

            }
            else if (line.rfind(" http://", 0) ==0 || line.rfind(" https://",0)==0) {
                std::string url = line.substr(0);
                while (!url.empty() && (url[0] == ' ' || url[0] == '\t')) {
                    url.erase(url.begin());
                }
                while (!url.empty() && (url.back() == ' ' || url.back() == '\t')) {
                    url.pop_back();
                }
                goToURL(url, true);
            }
            else if (!line.empty()) {
                std::string url = normalize_url(line);
                if (!url.empty()) {
                    goToURL(url, true);
                }
            }
            else {
                std::cout<<"Umknown command. Type help.\n"<<std::endl;
            }
        }
    }
    void TerminalUI::extract_links(const std::string &html, const std::string &base_url) {
        current_links.clear();

        // std::regex link_regex(R"(<a[^>]*href\s*=\s*["']([^"']+)["'])", std::regex::icase);
        // auto begin = std::sregex_iterator(html.begin(), html.end(), link_regex);
        // auto end = std::sregex_iterator();
        // auto decodeEntities= [](std::string s ) {
        //     size_t pos = 0;
        //     while ((pos = s.find("&amp;", pos))!= std::string::npos) {
        //         s.replace(pos, 5,"&");
        //     }
        //     return s;
        // };


        auto decode_entities= [](std::string s) {
            size_t pos = 0;
            while ((pos = s.find("&amp;", pos)) != std::string::npos) {
                s.replace(pos, 5, "&");
            }
            return s;
        };
        GumboOutput* output = gumbo_parse(html.c_str());
        std::function<void(GumboNode*)> walk = [&](GumboNode* node) {
            if (!node || current_links.size() >= 30) return;

            if (node->type == GUMBO_NODE_ELEMENT) {
                if (node->v.element.tag ==GUMBO_TAG_A) {
                    GumboAttribute* href_attr = gumbo_get_attribute(&node -> v.element.attributes, "href");
                    if (href_attr && href_attr ->value){
                        std::string href = decode_entities(href_attr ->value);
                        std::string full = make_absolute_url(base_url,href);
                        if (!full.empty() && full.rfind("javascript:", 0)!=0 && full.rfind("mailto:",0) !=0) {
                            current_links.push_back(full);
                        }
                    }
                }
                GumboVector* children = &node -> v.element.children;
                for (unsigned int i = 0; i < children->length; ++i) {
                    walk(static_cast<GumboNode*>(children->data[i]));
                    if (current_links.size()>=30)break;
                }

            }
        };
        walk(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);


        //
        //     for (auto it = begin; it != end; ++it) {
        //         std::string href = (*it)[1].str();
        //         href = decodeEntities(href);
        //         std::string full = make_absolute_url(base_url, href);
        //
        //         if (full.empty()) {
        //             continue;
        //         }
        //         if (full.rfind("javascript:",0)==0) {
        //             continue;
        //         }
        //         if (full.rfind("mailto:", 0)==0) {
        //             continue;
        //         }
        //         current_links.push_back(full);
        //
        //         if (current_links.size() >=30) {
        //             break;
        //         }
        //     }
        //}
    }
    void TerminalUI::show_links() {
        if (current_links.empty()) {
            std::cout<<"No links found on this page.\n"<< std::endl;
            return;
        }
        std::cout<<"Links:\n";
        for (size_t i = 0; i < current_links.size(); ++i) {
            std::cout<< "["<< i <<"] " << current_links[i]<< "\n";
        }
        std::cout<< std::endl;
    }void TerminalUI::open_linkby_index(int index) {
        if (index<0 || index >= (int)current_links.size()) {
            std::cout<<"Invalid link index.\n"<< std::endl;
            return;
        }
        goToURL(current_links[index],true);
    }

}