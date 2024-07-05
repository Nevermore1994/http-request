//
// Created by Nevermore on 2024/6/17.
// example Socket
// Copyright (c) 2024 Nevermore All rights reserved.
//
#pragma once

#include "Type.h"
#include "Data.hpp"
#include "Utility.h"
#include <tuple>

#if defined(_WIN32) || defined(__CYGWIN__)
#pragma push_macro("WIN32_LEAN_AND_MEAN")
#pragma push_macro("NOMINMAX")

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>
#if _WIN32_WINNT < _WIN32_WINNT_WINXP
extern "C" char *_strdup(const char *strSource);
    #define strdup _strdup
    #include <wspiapi.h>
#endif // _WIN32_WINNT < _WIN32_WINNT_WINXP

#pragma pop_macro("WIN32_LEAN_AND_MEAN")
#pragma pop_macro("NOMINMAX")

#define GetLastError() WSAGetLastError()

#else
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define GetLastError() errno
#endif // defined(_WIN32) || defined(__CYGWIN__)

namespace http {

#if defined(_WIN32) || defined(__CYGWIN__)
    using Socket = SOCKET;
    static constexpr Socket kInvalidSocket = INVALID_SOCKET;

    #define RetryCode WSAEINTR
    #define BusyCode WSAEWOULDBLOCK
    #define AgainCode WSAEWOULDBLOCK
    #define SocketError SOCKET_ERROR
#else
    using Socket = int;
    static constexpr Socket kInvalidSocket = kInvalid;

    #define RetryCode EINTR
    #define AgainCode EAGAIN
    #define BusyCode EINPROGRESS
    #define SocketError kInvalid
#endif // defined(_WIN32) || defined(__CYGWIN__)

#if defined(__unix__) && !defined(__APPLE__) && !defined(__CYGWIN__)
static constexpr int kNoSignal = MSG_NOSIGNAL;
#else
static constexpr int kNoSignal = 0;
#endif // defined(__unix__) && !defined(__APPLE__)


constexpr int32_t kDefaultReadSize = 4 * 1024; //4kb

enum class SelectType{
    Read,
    Write
};

struct SocketResult {
    ResultCode resultCode = ResultCode::Success;
    int32_t errorCode = 0;

    [[nodiscard]] bool isSuccess() const {
        return resultCode == ResultCode::Success;
    }

    void reset() noexcept {
        resultCode = ResultCode::Success;
        errorCode = 0;
    }
};

SocketResult select(SelectType type, Socket socket, int64_t timeout);

using AddressInfoPtr = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;
#define MakeAddressInfoPtr(addrinfo) AddressInfoPtr(addrinfo, &freeaddrinfo)
#define GetAddressFamily(ipVersion) ((ipVersion) == IPVersion::V4 ? AF_INET : ((ipVersion) == IPVersion::V6 ? AF_INET6 : AF_UNSPEC))

class ISocket {
public:
    explicit ISocket(IPVersion ipVersion = IPVersion::V4);
    virtual ~ISocket() = default;
    ISocket(const ISocket&) = delete;
    ISocket& operator=(const ISocket&) = delete;
    ISocket(ISocket&& rhs) noexcept;
    ISocket& operator=(ISocket&& rhs) noexcept;

    ///return ResultCode and error code, error code is last error number
    virtual SocketResult connect(const AddressInfoPtr& address, int64_t timeout) noexcept;

    ///return ResultCode and the number of bytes sent successfully
    [[nodiscard]] virtual std::tuple<SocketResult, int64_t> send(const std::string_view& data) const noexcept = 0;

    ///return ResultCode and the received data
    [[nodiscard]] virtual std::tuple<SocketResult, DataPtr> receive() const noexcept = 0;

    ///close socket and reset socket
    virtual void close() noexcept;

    [[nodiscard]] virtual SocketResult canSend(int64_t timeout) const noexcept;

    [[nodiscard]] virtual SocketResult canReceive(int64_t timeout) const noexcept;
protected:
    ResultCode config() noexcept;
private:
    void checkConnectResult(SocketResult& result, int64_t timeout) const noexcept;
protected:
    IPVersion ipVersion_ = IPVersion::V4;
    Socket socket_ = kInvalidSocket;
};

inline void freeSocket(ISocket* socket) noexcept {
    delete socket;
}

}//end of namespace http
