//
// Created by xint2 on 15/04/2026.
//

#ifndef GAR_TOR_CONNECTOR_H
#define GAR_TOR_CONNECTOR_H

#include <string>
#include <memory>
#include <psdk_inc/_socket_types.h>

namespace gar::anonymity {
    class TorConnector {
    public:
        TorConnector(
            const std::string& socks5_host = "127.0.00.1",
            int socks5_port =9050);

        ~TorConnector();

        bool testconnection();
        std::string getSocks5Address() const;
        bool isConnected() const;

        std::string gethost() const;
        int getPort() const;

        std::string getLastError() const;

    private:
        std::string socks5_host_;
        int socks5_port_;
        bool is_connected_;
        std::string last_error_;

        bool checkTorResponds();
        void setError(const std::string& error);
        void winCleanUP(SOCKET sock);
    };
}

#endif //GAR_TOR_CONNECTOR_H