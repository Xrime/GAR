//
// Created by xint2 on 20/05/2026.
//
#include ".././include/security/request_replay.h"
#include "../../include/core/http_client.h"

namespace gar::security {
    void requestReplay::capture(const captureRequest &req) {
        last_ = req;
        has_ = true;
    }
    bool requestReplay::has_capture() const {
        return has_;
    }
    replayResult requestReplay::replay() {
        replayResult r{};
        r.success = false;
        if (!has_) {
            r.success = false;
        }
        gar::core::HttpClient client("127.0.0.1", 9050);
        for (auto& h : last_.headers) {
            client.setHeader(h.first, h.second);
        }
        auto resp = (last_.method=="POST")
        ? client.post(last_.url, last_.body)
        : client.get(last_.url);

        if (!resp.success) {
            r.error =resp.error_message;
            return r;
        }
        r.success = true;
        r.status_code = resp.status_code;
        r.body_size = resp.body.size();
        return r;

    }

}