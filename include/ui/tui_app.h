//
// Created by xint2 on 11/06/2026.
//

#ifndef GAR_TUI_APP_H
#define GAR_TUI_APP_H
#include <string>
#include <vector>
#include <memory>
#include  <mutex>
#include <thread>
#include <map>
#include "settings.h"
#include "../include/anonymity/tor_manager.h"
#include "../include/core/http_client.h"

namespace gar::ui {
    struct linkItem {
        std::string url;
        std::string label;
    };
    struct mediaItem {
        std::string type;
        std::string url;
    };
    struct  formInput {
        std::string name;
        std::string value;
        std::string type;
        bool checked = false;
    };
    struct formItem {
        std::string action;
        std::string method;
        std::vector<formInput> inputs;
        int active_radio_index = 0;
    };

    class tuiApp {
    public:
        explicit tuiApp(Settings settings);
        ~tuiApp();
        int Run();

    private:
        std::unique_ptr<gar::anonymity::TorManager> tor_manager_;
        std::unique_ptr<gar::core::HttpClient> http_client_;
        Settings settings_;
        std::string settings_path_ = "gar_tui.conf";

        std::thread t_thread_;
        std::mutex state_mutex_;
        bool t_ready_ =false;
        std::string t_start_error_;
        bool t_start_failed_;
        std::string url_input_;
        std::string current_url_;
        std::string last_capture_url_;
        std::string status_line_;
        std::string page_txt;
        bool is_loading = false;
        gar::core::HttpResponse last_response_;
        std::string last_html_;
        std::map<std::string, std::string >last_headers_;
        long last_load_time_ms_ = 0;
        std::vector<linkItem> links_;
        std::vector<mediaItem> media_;
        std::vector<formItem> forms_;
        std::vector<std::string> history_;
        int history_index_ = -1;
        std::vector<std::string> bookmarks_;
        bool show_settings_= false;
        bool show_security_ = false;
        std::string security_panel_text_;

        void startTorAsync();
        void stopTor();
        static std::string normalizeUrl(std::string input);
        static std::string makeAbsoluteUrl(const std::string& base_url, const std::string& href);
        static std::string htmlToText(const std::string& html);
        void extractLinksFromHtml(const std::string& html, const std::string& base_url);
        void extractMediaFromHtml(const std::string& html, const std::string& base_url);
        void extractFormFromHtml(const std::string& html, const std::string& base_url);
        void Navigate(const std::string& url);
        void submitForm(const formItem& form);
        void goBack();
        void goForward();
        void addBookmark();
        void openMedia(const mediaItem& item);
        void analyzeHeaders();
        void inspectTLS(const std::string& html);
        void requestNewnym();
        void showExitIP();
        void flushDNS();
        void rotateFingerprint();
        void replayLastRequest();
        void saveSettingNow();

    };
}
#endif //GAR_TUI_APP_H