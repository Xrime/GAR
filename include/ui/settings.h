//
// Created by xint2 on 11/06/2026.
//

#ifndef GAR_SETTING_H
#define GAR_SETTING_H
#include <string>

namespace gar::ui {
    enum class MediaMode {
        PrintUrl = 0,
        ExternalPlayer = 1
    };
    struct Settings {
        MediaMode media_mode = MediaMode::PrintUrl;
        std::string video_player_cmd = "mpv";
        std::string image_viewer_cmd = "imv";
        std::string audio_player_cmd = "mpv";

        bool use_proxy = true;
        std::string host= "127.0.0.1";
        int port = 9050;
        std::string user_agent = "GAR";

    };
    Settings LoadSettingsOrDefault(const std::string& path);
    bool SaveSettings(const Settings& s, const std::string& path);

}

#endif //GAR_SETTING_H