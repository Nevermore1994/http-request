//
// Created by Nevermore on 2024/6/22.
// http-request main
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include "Request.h"
#include "Utility.h"
#include <condition_variable>
#include <iostream>

using namespace http;
std::condition_variable cond;
std::mutex mutex;
bool isFinished = false;

int main() {
    Request::init();
    RequestInfo info;
    info.url = "https://baidu.com";
    info.methodType = HttpMethodType::Get;
    info.headers = {
    {"Content-Type", "application/json"},
    {"User-Agent",    "runscope/0.1"},
    {"Accept",        "*/*"}};

    int length = 0;
    ResponseHandler handler;
    FILE* file = ::fopen("testx.html", "w+");
    auto startTime = util::Time::nowTimeStamp();
    handler.onConnected = [&](std::string_view reqId) {
        std::cout << "onConnected:" << static_cast<double >(util::Time::nowTimeStamp() - startTime) / 1000.0  << std::endl;
    };
    handler.onParseHeaderDone = [](std::string_view reqId, http::ResponseHeader &&header){
        std::cout << "http code:" << static_cast<int>(header.httpStatusCode) << std::endl;
    };
    handler.onData = [&](std::string_view reqId, DataPtr data) {
        std::cout << data->view();
        length += static_cast<int>(data->view().length());
        fwrite(data->rawData, 1, data->length, file);
    };
    handler.onDisconnected = [&](std::string_view reqId) {
        {
            std::lock_guard lock(mutex);
            isFinished = true;
        }
        std::cout << std::endl << "done reqId:" <<  reqId << ", time:" << static_cast<double >(util::Time::nowTimeStamp() - startTime) / 1000.0  << std::endl;
        std::cout.flush();
        cond.notify_all();
    };
    handler.onError = [](std::string_view reqId, ErrorInfo error) {
        std::cout << "reqId:" << reqId << ", retCode:" << static_cast<int>(error.retCode) << ", errorCode:" << error.errorCode;
    };

    Request request(std::move(info), std::move(handler));
    auto& reqId = request.getReqId();
    std::cout << "reqId:" <<  reqId << std::endl;
    std::unique_lock lock(mutex);
    cond.wait(lock, [&]{ return isFinished; });
    std::cout << "finish:" << length << ", cost time:" << static_cast<double >(util::Time::nowTimeStamp() - startTime) / 1000.0 << std::endl;
    Request::clear();
    return 0;
}