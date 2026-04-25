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
    };
}
#endif //GAR_TERMINAL_UI_H