//
// Created by xint2 on 25/04/2026.
//
#include "../../include/ui/terminal_ui.h"
#include <iostream>


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
        std::cout<<"Status: "<<response.status_code<<" "<< response.status_message<<std::endl;
        std::cout<<"Body size"<< response.body.size()<<"bytes"<<std::endl;
        std::cout<<"Preview:\n"<< response.body.substr(0,700)<<"\n"<<std::endl;

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
            }else if (line.rfind("go ", 0) == 0) {
                std::string raw = line.substr(3);
                std::string url = normalize_url(raw);
                if (url.empty()) {
                    std::cout << "Enter a URL.\n"<< std::endl;
                }else {
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
            }else {
                std::cout<<"Umknown command. Type help.\n"<<std::endl;
                }
            }
        }

    }
