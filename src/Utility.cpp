//
// Created by Nevermore on 2024/5/16.
// http-request Utility
// Copyright (c) 2024 Nevermore All rights reserved.
//
#include "Utility.h"
#include <random>

namespace http::util {

using namespace std::string_view_literals;
constexpr static std::string_view kRandomString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"sv;

std::string& StringUtil::removePrefix(std::string& str, char c) noexcept {
    if (!str.empty() && str.front() == c) {
        str.erase(0);
    }
    return str;
}

///C++ 20 replace ranges
std::vector<std::string_view> StringUtil::split(std::string_view strView, std::string_view delimiters) noexcept {
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = 0;

    while ((end = strView.find(delimiters, start)) != std::string::npos) {
        tokens.push_back(strView.substr(start, end - start));
        start = end + delimiters.length();
    }
    // Add the last token
    tokens.push_back(strView.substr(start));

    return tokens;
}

std::string StringUtil::randomString(uint32_t length) noexcept {
    std::string result;
    result.resize(length);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(kRandomString.size()));
    for (uint32_t i = 0; i < length; i++) {
        result[i] = kRandomString[static_cast<uint32_t>(dis(gen))];
    }
    return result;
}


void StringUtil::toLower(std::string& str) noexcept {
    std::for_each(str.begin(), str.end(), [](auto& c) {
        std::tolower(static_cast<unsigned char>(c));
    });
}

void StringUtil::toUpper(std::string& str) noexcept {
    std::for_each(str.begin(), str.end(), [](auto& c) {
        std::toupper(static_cast<unsigned char>(c));
    });
}

std::string StringUtil::toLower(const std::string& str) noexcept {
    auto res(str);
    std::transform(str.cbegin(), str.cend(), res.begin(),[](const unsigned char& c){
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    });
    return res;
}

std::string StringUtil::toUpper(const std::string& str) noexcept {
    auto res(str);
    std::transform(str.cbegin(), str.cend(), res.begin(),[](const unsigned char& c){
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    });
    return res;
}

Time::TimeStamp Time::nowTimeStamp() noexcept {
    using namespace std::chrono;
    auto tp = time_point_cast<microseconds>(system_clock::now());
    return static_cast<TimeStamp>(tp.time_since_epoch().count());
}

std::chrono::milliseconds Time::TimeStamp::diff(Time::TimeStamp timeStamp) const noexcept {
    return std::chrono::milliseconds( (stamp - timeStamp) / 1000);
}

Time::TimeStamp Time::TimeStamp::operator+(std::chrono::milliseconds delta) const noexcept {
    return stamp + delta.count() * 1000;
}

Time::TimeStamp Time::TimeStamp::operator+=(std::chrono::milliseconds delta) noexcept {
    stamp += delta.count() * 1000;
    return stamp;
}

Time::TimeStamp Time::TimeStamp::operator-(std::chrono::milliseconds delta) const noexcept {
    return stamp - delta.count() * 1000;
}

Time::TimeStamp Time::TimeStamp::operator-=(std::chrono::milliseconds delta) noexcept {
    stamp -= delta.count() * 1000;
    return stamp;
}


}
