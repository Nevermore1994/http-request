//
// Created by Nevermore on 2024/6/11.
// http-request IPAddress
// Copyright (c) 2024 Nevermore All rights reserved.
//
#pragma once

#include <string>
#include <chrono>
#include <vector>

namespace http::util {

struct StringUtil {
    [[maybe_unused]] static std::string randomString(uint32_t length) noexcept;
    [[maybe_unused]] static void toLower(std::string& str) noexcept;
    [[maybe_unused]] static void toUpper(std::string& str) noexcept;
    [[maybe_unused]] static std::string toLower(const std::string& str) noexcept;
    [[maybe_unused]] static std::string toUpper(const std::string& str) noexcept;
    [[maybe_unused]] static std::vector<std::string_view> split(std::string_view str, std::string_view delimiters) noexcept;
    [[maybe_unused]] static std::string& removePrefix(std::string& str, char c) noexcept;
};

struct Time {
    ///microseconds
    struct TimeStamp {
        uint64_t stamp;
#ifdef __clang__
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
#endif
        operator uint64_t() const {
            return stamp;
        }

        [[maybe_unused]] TimeStamp(uint64_t t)
            : stamp(t) {

        }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        [[nodiscard]] std::chrono::milliseconds diff(Time::TimeStamp timeStamp) const noexcept;
        TimeStamp operator+(std::chrono::milliseconds delta) const noexcept;
        TimeStamp operator+=(std::chrono::milliseconds delta) noexcept;
        TimeStamp operator-(std::chrono::milliseconds delta) const noexcept;
        TimeStamp operator-=(std::chrono::milliseconds delta) noexcept;
    };
    static TimeStamp nowTimeStamp() noexcept;
    static std::chrono::milliseconds nowTime() noexcept;
};

} // end of namespace http::util




