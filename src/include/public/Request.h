//
// Created by Nevermore on 2024/5/16.
// http-request task
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include <thread>
#include <atomic>
#include "Data.hpp"
#include "Type.h"
#include "../Socket.h"
#include "../Url.h"

namespace http {

struct ResponseHeader;

struct RequestInfo {
    ///default true
    bool isAllowRedirect = true;
    ///default V4
    IPVersion ipVersion = IPVersion::Auto;
    std::string url;
    HttpMethodType methodType = HttpMethodType::Unknown;
    std::unordered_map<std::string, std::string> headers;
    DataRefPtr body = nullptr;
    ///default 30s
    std::chrono::milliseconds timeout{60 * 1000};

    [[nodiscard]] inline uint64_t bodySize() const noexcept {
        return body ? body->length : 0;
    }

    [[nodiscard]] inline bool bodyEmpty() const noexcept {
        return !body || body->empty();
    }

    using ResponseHeaderFunc = std::function<void(std::string_view, ResponseHeader&&)>;
    using ResponseDataFunc = std::function<void(std::string_view, DataPtr data)>;
    using DisconnectedFunc = std::function<void(std::string_view)>;

    ResponseHeaderFunc responseHeader = nullptr;
    ResponseDataFunc responseData = nullptr;
    DisconnectedFunc disconnected = nullptr;
};

struct ResponseHeader {
    std::unordered_map<std::string, std::string> headers;
    HttpStatusCode httpStatusCode = HttpStatusCode::Unknown;
    int32_t errorCode{};
    ResultCode retCode = ResultCode::Success;
    std::string reasonPhrase;

    [[nodiscard]] bool isSuccess() const noexcept {
        return retCode == ResultCode::Success;
    }

    [[nodiscard]] bool isNeedRedirect() const noexcept {
        return httpStatusCode == HttpStatusCode::MovedPermanently || httpStatusCode == HttpStatusCode::Found;
    }

    ResponseHeader() = default;
};

class Request {
public:
    [[maybe_unused]] explicit  Request(const RequestInfo& info);
    [[maybe_unused]] explicit Request(RequestInfo&& info);
    ~Request();

    [[maybe_unused]] void cancel() noexcept {
        isValid_ = false;
    }

    [[maybe_unused]] [[nodiscard]] const std::string& getReqId() const {
        return reqId_;
    }
private:
    void config() noexcept;
    void sendRequest() noexcept;
    void redirect(const std::string&) noexcept;
    void process() noexcept;
    int64_t getRemainTime() const noexcept;
    void send() noexcept;
    bool isReceivable() noexcept;
    void receive() noexcept;
    void responseHeader(ResponseHeader&&) noexcept;
    bool parseChunk(DataPtr& data, int64_t& chunkSize, bool& isCompleted) noexcept;
    void handleErrorResponse(ResultCode code, int32_t errorCode) noexcept;
    void disconnected() noexcept;
private:
    uint8_t redirectCount_ = 0;
    std::atomic<bool> isValid_ = true;
    uint64_t startStamp_ = 0;
    RequestInfo info_;
    std::unique_ptr<Socket> socket_ = nullptr;
    std::unique_ptr<Url> url_ = nullptr;
    std::unique_ptr<std::thread> worker_ = nullptr;
    std::string reqId_;
};

}

