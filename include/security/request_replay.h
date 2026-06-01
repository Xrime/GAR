//
// Created by xint2 on 15/05/2026.
//

#ifndef GAR_REQUEST_REPLAY_H
#define GAR_REQUEST_REPLAY_H
#include <string>
#include <map>

namespace gar::security {
    struct captureRequest {
        std::string method;
        std::string url;
        std::string body;
        std::map<std::string, std::string> headers;

    };
    struct replayResult {
        bool success;
        int status_code;
        size_t body_size;
        std::string error;
    };
    class requestReplay {
    public:
        void capture(const captureRequest& req);
        bool has_capture() const;
        replayResult replay();
    private:
        captureRequest last_;
        bool has_ = false;
    };
}
#endif //GAR_REQUEST_REPLAY_H