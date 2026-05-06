//
// Created by xint2 on 25/04/2026.
//

#ifndef GAR_TERMINAL_UI_H
#define GAR_TERMINAL_UI_H
#include <string>
#include  <vector>
#include "core/http_client.h"
namespace gar::terminal_ui {
    class TerminalUI {
    public:
        TerminalUI(core::HttpClient& client);
        void run();

    private:
        core::HttpClient& http_client;
        std::vector<std::string> history;
        int history_index;

        void showBanner();
        void showHelp();
        void goToURL(const std::string& url, bool add_to_history);
        void goBack();
        void goForward();
        void refreshPage();
        std::vector<std::string> current_links;
        void extract_links(const std::string& html, const std::string& base_url);
        void show_links();
        void open_linkby_index(int index);
        std::string make_absolute_url(const std::string& base_url, const std::string& href);
        std::string last_url;
        int last_status;
        size_t last_size;
        std::vector<std::string> histroy;
        std::vector<std::string> bookmarks;
        bool show_source = false;
        std::string last_html;
        std::map<std::string, std::string> last_headers;
    };
}
#endif //GAR_TERMINAL_UI_H