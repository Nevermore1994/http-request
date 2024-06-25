//
// Created by Nevermore on 2024/6/22.
// http-request main
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include "Data.hpp"
#include "Request.h"
#include <condition_variable>
#include <iostream>
#include <memory>

using namespace http;
std::condition_variable cond;
std::mutex mutex;
bool isFinished = false;

int main() {
    RequestInfo info;
    info.url = "https://ngxpls-acqa.acqa.zoomdev.us/zmail/v1/users/me/settings/vacation";
    info.methodType = HttpMethodType::Put;
    info.headers = {
    {"Content-Type", "application/json"},
    {"Authorization", "Bearer eyJhbGciOiJFUzI1NiIsInN2IjoiMDAwMDAxIiwidHlwIjoiSldUIiwiem1fc2ttIjoiem1fbzJtIn0.eyJhaWQiOiJudVNoeWRka1I1V3g3bjl4bkNoRld3IiwidWlkIjoibDd2VDBzYlRUVHlIb0tSRzBzTW1xQSIsImF1ZCI6ImFzeW5jX2NvbW0iLCJlbWwiOiJyb2xmLnRhbkBhY3FhLnpvb21kZXYudXMiLCJ1biI6InJvbGYudGFuQGFjcWEuem9vbWRldi51cyIsInR5IjoxLCJ0eiI6IkFtZXJpY2EvRGVudmVyIiwiaXNzIjoid2ViIiwiZXhwIjoxNzIyODk3ODU2LCJpYXQiOjE3MTkyOTc4NTYsImxvY2wiOiJlbi1VUyIsInptYWlsIjoicm9sZi50YW5AYWNxYS56b29tZGV2LnVzIiwiY3QiOjB9.M4vx_Y2SMNuC6YfFl_IIn4JbNZteLxs5az8k_5le8vefB89SNT6ZBot8y3HfCGsmTNJjsRo2kEancKdUiZht6g"},
    {"User-Agent",    "runscope/0.1"},
    {"Accept",        "*/*"}};
    info.body = std::make_shared<Data>("{\n"
                                       "    \"enableAutoReply\": false,\n"
                                       "    \"endTime\": \"1718726399000\",\n"
                                       "    \"endTimeISO\": \"2024-06-18T15:59:59.000Z\",\n"
                                       "    \"responseBodyHtml\": \"<div><div><div><div>Eqweqweqw<br></div></div></div></div>\",\n"
                                       "    \"responseSubject\": \"我在测试\",\n"
                                       "    \"restrictToContacts\": false,\n"
                                       "    \"restrictToDomain\": false,\n"
                                       "    \"startTime\": \"1717516800000\",\n"
                                       "    \"startTimeISO\": \"2024-06-04T16:00:00.000Z\"\n"
                                       "}");

    int length = 0;
    ResponseHandler handler;
    FILE* file = ::fopen("testx.html", "w+");
    handler.onConnected = [](std::string_view reqId) {
        ///std::cout << "onConnected:" << reqId << std::endl;
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
        std::cout << std::endl << "done reqId:" <<  reqId << std::endl;
        std::cout.flush();
        cond.notify_all();
    };
    handler.onError = [](std::string_view reqId, ErrorInfo error) {
       // std::cout << "reqId:" << reqId << ", retCode:" << static_cast<int>(error.retCode) << ", errorCode:" << error.errorCode;
    };

    Request request(std::move(info), std::move(handler));
    auto& reqId = request.getReqId();
    std::cout << "reqId:" <<  reqId << std::endl;
    std::unique_lock lock(mutex);
    cond.wait(lock, [&]{ return isFinished; });
    std::cout << "finish:" << length << std::endl;
    return 0;
}