//
// Created by Nevermore on 2024/6/18.
// example SSLManager
// Copyright (c) 2024 Nevermore All rights reserved.
//
#pragma once

#if ENABLE_HTTPS
#include <mutex>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "PlainSocket.h"

namespace http {

using SSLPtr = std::unique_ptr<SSL, decltype(&SSL_free)>;
using SSLContextPtr = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;

class SSLManager {
public:
    static std::function<void(SSLContextPtr&)> configContext;
    static SSLContextPtr& shareContext();

    static SSLPtr create(Socket) noexcept;
    static SocketResult connect(SSLPtr&) noexcept;
    [[nodiscard]] static std::tuple<SocketResult, int64_t> write(const SSLPtr&, const std::string_view&) noexcept;
    [[nodiscard]] static std::tuple<SocketResult, DataPtr> read(const SSLPtr&) noexcept;
    static void close(SSLPtr&) noexcept;
private:
    SSLManager();
    ~SSLManager();
};

} //end of namespace http

#endif