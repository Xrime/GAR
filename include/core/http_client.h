#pragma once
#include <memory>
#include <string>
#include <map>


namespace gar::core {
    class IHttpTransport;
    class HttpRequest;
    class HttpResponse;
    class HttpClient {
    public:
        explicit HttpClient(std::unique_ptr<IHttpTransport> transport);

        ~HttpClient();

        HttpResponse get(const std::string & url);
        HttpResponse post(const std::string& url,
                          const std::string& body,
                          const std::map<std::string,std::string> & headers ={});

        void setHeader(const std::string& key, const std::string& value);

        bool isConnected() const;

    private:
        std::unique_ptr<IHttpTransport> transport_;

        std::map<std::string, std::string> default_headers_;

        void validateUrl(const std::string& url) const;
        HttpRequest buildRequest(const std::string& url, const std::string& method);
    };

}
