//
// Created by Nevermore on 2024/5/10.
// http-request Socket
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include "Socket.h"

namespace http {

#if defined(__unix__) && !defined(__APPLE__) && !defined(__CYGWIN__)
    static constexpr int kNoSignal = MSG_NOSIGNAL;
#else
    static constexpr int kNoSignal = 0;
#endif // defined(__unix__) && !defined(__APPLE__)

#if defined(_WIN32) || defined(__CYGWIN__)

#define RetryCode WSAEINTR
#define BusyCode WSAEWOULDBLOCK
#define AgainCode WSAEWOULDBLOCK
#define SocketError SOCKET_ERROR
#else

#define RetryCode EINTR
#define AgainCode EAGAIN
#define BusyCode EINPROGRESS
#define SocketError kInvalid
#endif

constexpr int32_t kDefaultReadSize = 4 * 1024; //4kb

enum class SelectType{
    Read,
    Write
};

#define GetSelectFDSet(type1, type2, FD)  ((type1) == (type2) ? FD : nullptr)
#define GetSelectReadFDSet(type, FD)  GetSelectFDSet(type, SelectType::Read, FD)
#define GetSelectWriteFDSet(type, FD)  GetSelectFDSet(type, SelectType::Write, FD)
SocketResult select(SelectType type, Socket::SocketType socket, int64_t timeout) {
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

Socket::Socket(IPVersion ipVersion)
    : ipVersion_(ipVersion)
    , socket_(socket(GetAddressFamily(ipVersion), SOCK_STREAM, IPPROTO_TCP)){

}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& rhs) noexcept
    : socket_(std::exchange(rhs.socket_, kInvalidSocket)){

}

Socket& Socket::operator=(Socket&& rhs) noexcept {
    if (this == &rhs) {
        return *this;
    }
    close();
    socket_ = std::exchange(rhs.socket_, kInvalidSocket);
    return *this;
}

ResultCode Socket::config() noexcept {
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

void Socket::checkConnectResult(SocketResult& result, int64_t timeout) const noexcept {
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

SocketResult Socket::connect(const AddressInfoPtr& address, int64_t timeout) noexcept {
    SocketResult result;
    if (address == nullptr) {
        result.resultCode = ResultCode::ConnectAddressError;
        return result;
    }
    if (address->ai_family != GetAddressFamily(ipVersion_)) {
        result.resultCode = ResultCode::ConnectTypeInconsistent;
        return result;
    }
    result.resultCode = config();
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

SocketResult Socket::canSend(int64_t timeout) const noexcept {
    auto result = select(SelectType::Write, socket_, timeout);
    return result;
}

SocketResult Socket::canReceive(int64_t timeout) const noexcept {
    auto result = select(SelectType::Read, socket_, timeout);
    return result;
}

std::tuple<SocketResult, uint64_t> Socket::send(const std::string_view& dataView) const noexcept {
    ssize_t sendResult = 0;
    SocketResult result;
    do {
        sendResult = ::send(socket_, dataView.data(), dataView.length(), kNoSignal);
        if (sendResult == SocketError) {
            result.errorCode = GetLastError();
        } else {
            result.errorCode = 0;
        }
    } while (result.errorCode == RetryCode);

    if (sendResult == kInvalid) {
        sendResult = 0;
        result.resultCode = result.errorCode == RetryCode ? ResultCode::Retry : ResultCode::Failed;
    }
    return {result, sendResult};
}

std::tuple<SocketResult, DataPtr> Socket::receive() const noexcept {
    SocketResult result;
    auto data = std::make_unique<Data>(kDefaultReadSize);
    ssize_t receiveSize = 0;
    do {
        receiveSize = ::recv(socket_, data->rawData, data->capacity, kNoSignal);
        if (receiveSize == kInvalid) {
            result.errorCode = GetLastError();
        } else {
            data->length = static_cast<uint64_t>(receiveSize);
            result.errorCode = 0;
        }
    } while(result.errorCode == RetryCode);
    if (receiveSize == kInvalid) {
        result.resultCode = result.errorCode == AgainCode ? ResultCode::Retry : ResultCode::Failed;
    } else if (data->empty()) {
        result.resultCode = ResultCode::Disconnected;
    } else {
        result.resultCode = ResultCode::Success;
    }
    return {result, std::move(data)};
}

void Socket::close() noexcept {
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

} //end of namespace http
