//
// Created by xint2 on 30/04/2026.
//

#ifndef GAR_SECURE_MEMORY_H
#define GAR_SECURE_MEMORY_H
#include <windows.h>
#include <string>
#include <vector>


namespace gar::secure_memory {
    class Securebuffer {
    public:
        explicit Securebuffer(size_t size);
        ~Securebuffer();

        Securebuffer(const Securebuffer&) = delete;
        Securebuffer & operator = (const Securebuffer&) = delete;
        char* data();
        size_t size() const;

    private:
        char* buffer;
        size_t buffer_size;
    };
    class SecureString {
    public:
        SecureString();
        explicit SecureString(const std::string& s);
        ~SecureString();

        SecureString(const SecureString&) = delete;
        SecureString& operator = (const SecureString&)= delete;
        void set(const std::string& s);
        const std::string& str() const;
    private:
        std::string value;
    };
    void SecureZero(void* ptr, size_t size);
}

#endif //GAR_SECURE_MEMORY_H
