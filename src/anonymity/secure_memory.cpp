//
// Created by xint2 on 30/04/2026.
//
#include "../../include/anonymity/secure_memory.h"

namespace gar::secure_memory {
    void SecureZero(void *ptr, size_t size) {
        if (ptr && size >0) {
            SecureZeroMemory(ptr, size);
        }
    }

    Securebuffer::Securebuffer(size_t size)
        : buffer(nullptr), buffer_size(size){
        buffer = (char*)malloc(size);
        if (buffer) {
            SecureZero(buffer, size);
            VirtualLock(buffer, size);
        }
    }
    Securebuffer::~Securebuffer() {
        if (buffer) {
            SecureZero(buffer, buffer_size);
            VirtualUnlock(buffer, buffer_size);
            free(buffer);
            buffer = nullptr;
        }
    }
    char *Securebuffer::data() {
        return buffer;
    }
    size_t Securebuffer::size() const {
        return buffer_size;
    }
    SecureString::SecureString() : value(""){

    }
    SecureString::SecureString(const std::string &s) : value(s) {
    }
    SecureString::~SecureString() {
        if (!value.empty()) {
            SecureZero(&value[0], value.size());
        }
    }
    void SecureString::set(const std::string &s) {
        if (!value.empty()) {
            SecureZero(&value[0], value.size());
        }
        value=s;
    }
    const std::string &SecureString::str() const {
        return value;
    }








}



