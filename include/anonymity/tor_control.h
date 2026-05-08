//
// Created by xint2 on 07/05/2026.
//

#ifndef GAR_TOR_CONTROL_H
#define GAR_TOR_CONTROL_H
#include <string>
namespace  gar::anonymity {
    class TorControl {
    public:
        TorControl();
        bool connect(const std:: string& host ="127.0.0.1", int port = 9051);
        bool authenticate(const std::string& password ="");
        bool signal_newnym();
        void disconnect();
        std::string last_error() const {
            return last_error_;}
        bool getinfo(const std::string& key, std::string& value);
    private:
        int sock_;
        std::string last_error_;
        bool send_line (const std::string& line);
        bool read_line(std::string& out);
    };

}
#endif //GAR_TOR_CONTROL_H