//
// Created by Nevermore on 2024/5/10.
// http-request socket
// Copyright (c) 2024 Nevermore All rights reserved.
//
#pragma once
#include "Type.h"
#include "Data.hpp"
#include "Utility.h"
#include <tuple>

#if defined(_WIN32) || defined(__CYGWIN__)
#  pragma push_macro("WIN32_LEAN_AND_MEAN")
#  pragma push_macro("NOMINMAX")
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif // WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif // NOMINMAX
#  include <winsock2.h>
#  if _WIN32_WINNT < _WIN32_WINNT_WINXP
extern "C" char *_strdup(const char *strSource);
#    define strdup _strdup
#    include <wspiapi.h>
#  endif // _WIN32_WINNT < _WIN32_WINNT_WINXP
#  include <ws2tcpip.h>
#  pragma pop_macro("WIN32_LEAN_AND_MEAN")
#  pragma pop_macro("NOMINMAX")

#define GetLastError() WSAGetLastError()
#else
#  include <cerrno>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>

#define GetLastError() errno
#endif // defined(_WIN32) || defined(__CYGWIN__)

namespace http {

struct SocketResult {
    ResultCode resultCode = ResultCode::Success;
    int32_t errorCode = 0;

    [[nodiscard]] bool isSuccess() const {
        return resultCode == ResultCode::Success;
    }
};

using AddressInfoPtr = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;
#define MakeAddressInfoPtr(addrinfo) AddressInfoPtr(addrinfo, &freeaddrinfo)
#define GetAddressFamily(ipVersion) ((ipVersion) == IPVersion::V4 ? AF_INET : ((ipVersion) == IPVersion::V6 ? AF_INET6 : AF_UNSPEC))

class Socket final {
public:

#if defined(_WIN32) || defined(__CYGWIN__)
    using SocketType = SOCKET;
#else
    using SocketType = int;
#endif // defined(_WIN32) || defined(__CYGWIN__)

public:
    explicit Socket(IPVersion ipVersion = IPVersion::V4);
    ~Socket();
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&&) noexcept ;
    Socket& operator=(Socket&&) noexcept;

    ///return ResultCode and error code, error code is last error number
    SocketResult connect(const AddressInfoPtr& address, int64_t timeout) noexcept;
    [[nodiscard]] SocketResult canSend(int64_t timeout) const noexcept;
    [[nodiscard]] SocketResult canReceive(int64_t timeout) const noexcept;
    ///return ResultCode and the number of bytes sent successfully
    [[nodiscard]] std::tuple<SocketResult, uint64_t> send(const std::string_view& data) const noexcept;
    ///return ResultCode and the received data
    [[nodiscard]] std::tuple<SocketResult, DataPtr> receive() const noexcept;
    ///close socket and reset socket
    void close() noexcept;

private:
    ResultCode config() noexcept;
    void checkConnectResult(SocketResult& result, int64_t timeout) const noexcept;
private:

#if defined(_WIN32) || defined(__CYGWIN__)
    static constexpr Socket::SocketType kInvalidSocket = INVALID_SOCKET;
#else
    static constexpr Socket::SocketType kInvalidSocket = kInvalid;
#endif // defined(_WIN32) || defined(__CYGWIN__)
    IPVersion ipVersion_ = IPVersion::V4;
    SocketType socket_ = kInvalidSocket;
};

} //end of namespace http
