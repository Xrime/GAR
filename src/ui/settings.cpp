#include "../../include/ui/settings.h"
#include <fstream>
#include <sstream>
#include <openssl/lhash.h>

namespace gar::ui {
    Settings LoadSettingsOrDefault(const std::string &path) {
        Settings s;
        std::ifstream file(path);
        if (!file.is_open()) {
            return s;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()|| line[0] == '#') {
                continue;
            }
            size_t eq = line.find('=');
            if (eq==std::string::npos) continue;
            std::string key = line.substr(0,eq);
            std::string val = line.substr(eq+1);
            if (key=="use_proxy") {
                s.use_proxy = (val=="true"|| val=="1");
            }else if (key =="host") {
                s.host = val;
            }else if (key == "port") {
                try {
                    s.port = std::stoi(val);
                }catch (...) {
                    s.port =9050;
                }
            }else if (key=="media_mode") {
                try {
                    s.media_mode = static_cast<MediaMode>(std::stoi(val));
                }catch (...){}
            }else if (key == "video_player_cmd") s.video_player_cmd = val;
            else if (key == "image_player_cmd") s.image_viewer_cmd = val;
            else if (key == "audio_player_cmd") s.audio_player_cmd = val;
            else if (key == "user_agent") s.user_agent = val;
        }
        return s;
    }
    bool SaveSettings(const Settings &s, const std::string &path) {
        std::ofstream file(path);
        if (!file.is_open()) return false;

        file <<"# GAR TUI Configurattion";
        file<< "use_proxy=" << (s.use_proxy ? "1" : "0")<< "\n";
        file<<"host="<< s.host<< "\n";
        file<<"port"<< s.port<<"\n";
        file<<"media_mode"<<static_cast<int>(s.media_mode)<<"\n";
        file<<"video_player_cmd"<< s.video_player_cmd<<"\n";
        file<<"image_player_cmd"<< s.image_viewer_cmd<<"\n";
        file<<"user_agent"<<s.user_agent<<"\n";

        return  true;

    }


}
