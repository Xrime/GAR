//
// Created by xint2 on 16/04/2026.
//

#ifndef GAR_TOR_MANAGER_H
#define GAR_TOR_MANAGER_H
#include <string>
#include <memory>

namespace gar::anonymity {
    class TorManager {
    public:
        TorManager();
        ~TorManager();
        bool startTor();
        void stopTor();
        bool isTorRunning() const;
        bool waitForTorReady(int timeout_second =30);

        unsigned long getTorProcessId() const;
        std::string getLastError() const;

    private:
        #ifdef _WIN32
                void* tor_process_handle_;
        #else
                int tor_process_id_;
        #endif

        bool is_running;
        std::string last_error_;
        std::string findTorExecutable();
        bool verifyTorIsRunnig();

        void setError(const std::string& error);

    };
}
#endif //GAR_TOR_MANAGER_H