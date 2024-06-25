//
// Created by Nevermore on 2024/6/17.
// example Socket
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include "Socket.h"

namespace http {

#define GetSelectFDSet(type1, type2, FD)  ((type1) == (type2) ? FD : nullptr)
#define GetSelectReadFDSet(type, FD)  GetSelectFDSet(type, SelectType::Read, FD)
#define GetSelectWriteFDSet(type, FD)  GetSelectFDSet(type, SelectType::Write, FD)
SocketResult select(SelectType type, Socket socket, int64_t timeout) {
    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(socket, &fdSet);
    int maxFd = socket + 1;
#if defined(_WIN32) || defined(__CYGWIN__)
    using timeval = TIMEVAL;
    maxFd = 0;
#endif
    timeval selectTimeout {
    static_cast<time_t>(timeout / 1000),
    static_cast<suseconds_t>((timeout % 1000) * 1000)
    };
    SocketResult result;
    int ret = ::select(maxFd, GetSelectReadFDSet(type, &fdSet), GetSelectWriteFDSet(type, &fdSet), nullptr, timeout >= 0 ? &selectTimeout : nullptr);
    if (ret == SocketError) {
        result.errorCode = GetLastError();
        result.resultCode = result.errorCode == RetryCode ? ResultCode::Retry : ResultCode::Failed;
    } else if (ret == 0) {
        result.resultCode = ResultCode::Timeout;
    }
    return result;
}

ISocket::ISocket(IPVersion ipVersion)
: ipVersion_(ipVersion)
, socket_(socket(GetAddressFamily(ipVersion), SOCK_STREAM, IPPROTO_TCP)){

}

ISocket::ISocket(ISocket&& rhs) noexcept
: ipVersion_(rhs.ipVersion_)
, socket_(std::exchange(rhs.socket_, kInvalidSocket)) {

}

ResultCode ISocket::config() noexcept {
    if (socket_ == kInvalidSocket) {
        return ResultCode::CreateSocketFailed;
    }
    ResultCode resultCode = ResultCode::Success;
#if defined(_WIN32) || defined(__CYGWIN__)
    ULONG mode = 1;
    if (ioctlsocket(socket_, FIONBIO, &mode) == SocketError) {
        resultCode = ResultCode::GetFlagsFailed;
    }
#else
    do {
        auto flags = fcntl(socket_, F_GETFL);
        if (flags == -1) {
            resultCode = ResultCode::GetFlagsFailed;
            break;
        }

        if (fcntl(socket_, F_SETFL, flags | O_NONBLOCK) == kInvalid) {
            resultCode = ResultCode::SetFlagsFailed;
            break;
        }
#if  defined(__APPLE__)
        int32_t value = 1;
        if (setsockopt(socket_, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value)) == kInvalid) {
            resultCode = ResultCode::SetNoSigPipeFailed;
        }
#endif
    } while(false);
#endif
    if (resultCode != ResultCode::Success) {
        close();
    }
    return resultCode;
}

ISocket& ISocket::operator=(ISocket&& rhs) noexcept {
    if (this == &rhs) {
        return *this;
    }
    close();
    ipVersion_ = rhs.ipVersion_;
    socket_ = std::exchange(rhs.socket_, kInvalidSocket);
    return *this;
}

void ISocket::close() noexcept {
    if (socket_ == kInvalidSocket) {
        return;
    }
#if defined(_WIN32) || defined(__CYGWIN__)
    closesocket(socket_);
#else
    ::close(socket_);
#endif
    socket_ = kInvalidSocket;
}

SocketResult ISocket::canSend(int64_t timeout) const noexcept {
    return select(SelectType::Write, socket_, timeout);
}

SocketResult ISocket::canReceive(int64_t timeout) const noexcept {
    return select(SelectType::Read, socket_, timeout);;
}

void ISocket::checkConnectResult(SocketResult& result, int64_t timeout) const noexcept {
    using namespace http::util;
    if (result.isSuccess()) {
        return;
    }

    auto checkNeedRetry = [&result](){
        return result.errorCode == RetryCode || result.errorCode == BusyCode;
    };

    if (!checkNeedRetry()) {
        return;
    }

    auto expiredTime = Time::nowTimeStamp() + std::chrono::milliseconds(timeout);
    bool isTimeout = false;
    do {
        auto remainTime = static_cast<int64_t>(expiredTime - Time::nowTimeStamp());
        if (remainTime < 0) {
            isTimeout = true;
            break;
        }
        auto selectResult = select(SelectType::Write, socket_, remainTime);
        result = selectResult;
    } while (!result.isSuccess() && checkNeedRetry());

    if (isTimeout) {
        result.resultCode = ResultCode::Timeout;
        result.errorCode = 0;
        return;
    }
    auto errorLength = static_cast<socklen_t>(sizeof(int));
#if defined(_WIN32) || defined(__CYGWIN__)
    char socketError[errorLength];
#else
    int socketError = 0;
#endif
    if (getsockopt(socket_, SOL_SOCKET, SO_ERROR, &socketError, &errorLength) == SocketError) {
        result.resultCode = ResultCode::ConnectGenericError;
        result.errorCode = GetLastError();
        return;
    }

    int error = 0;
#if defined(_WIN32) || defined(__CYGWIN__)
    std::memcpy(&error, socketError, errorLength);
#else
    error = socketError;
#endif
    if (error != 0) {
        result.resultCode = ResultCode::ConnectGenericError;
        result.errorCode = error;
    } else {
        result.resultCode = ResultCode::Success;
        result.errorCode = 0;
    }
}

SocketResult ISocket::connect(const AddressInfoPtr& address, int64_t timeout) noexcept {
    SocketResult result;
    if (address == nullptr) {
        result.resultCode = ResultCode::ConnectAddressError;
        return result;
    }
    if (address->ai_family != GetAddressFamily(ipVersion_)) {
        result.resultCode = ResultCode::ConnectTypeInconsistent;
        return result;
    }
    result.resultCode = ISocket::config();
    if (!result.isSuccess()) {
        result.errorCode = GetLastError();
        return result;
    }

    auto connectResult = ::connect(socket_, address->ai_addr, address->ai_addrlen);
    if (connectResult == kInvalid) {
        result.errorCode = GetLastError();
    } else {
        result.errorCode = 0;
    }
    result.resultCode = connectResult == kInvalid ? ResultCode::ConnectGenericError : ResultCode::Success;
    checkConnectResult(result, timeout);
    return result;
}

}//end of namespace http