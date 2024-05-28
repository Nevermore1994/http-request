//
// Created by Nevermore on 2024/5/22.
// http-request main
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include "Request.h"
#include <condition_variable>
#include <iostream>

using namespace http;
std::condition_variable cond;
std::mutex mutex;
bool isFinished = false;

int main() {
    RequestInfo info;
    info.url = "http://google.com";
    info.methodType = HttpMethodType::Get;
    info.headers = {{"Content-Type", "application/x-www-form-urlencoded"},
    {"User-Agent", "runscope/0.1"},
    {"Accept", "*/*"}};

    int length = 0;
    info.responseHeader = [](std::string_view reqId, http::ResponseHeader &&header){
        std::cout << "retCode :" << static_cast<int> (header.retCode) << ", errorCode:" <<  header.errorCode << ", http code:" << static_cast<int>(header.httpStatusCode) << std::endl;
    };
    info.responseData = [&](std::string_view reqId, DataPtr data) {
        std::cout << data->view();
        length += static_cast<int>(data->view().length());
    };
    info.disconnected = [&](std::string_view reqId) {
        {
            std::lock_guard lock(mutex);
            isFinished = true;
        }
        std::cout << std::endl << "done reqId:" <<  reqId << std::endl;
        std::cout.flush();
        cond.notify_all();
    };

    Request request(std::move(info));
    auto& reqId = request.getReqId();
    std::cout << "reqId:" <<  reqId << std::endl;
    std::unique_lock lock(mutex);
    cond.wait(lock, [&]{ return isFinished; });
    std::cout << "finish:" << length << std::endl;
    return 0;
}