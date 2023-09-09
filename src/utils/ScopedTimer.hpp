#pragma once

#include <chrono>
#include <fmt/core.h>
#include <ratio>
#include <type_traits>
#include <concepts>
#include <fmt/format.h>


template <typename ChronoDurationCast, bool Verbose = true>
class ScopedTimer  {
    std::chrono::steady_clock::time_point m_start_time;
    std::string m_name;
    constexpr std::string_view as_string() {
        if constexpr (std::is_same<ChronoDurationCast, std::chrono::nanoseconds>()) {
            return "ns";
        } else if constexpr (std::is_same<ChronoDurationCast, std::chrono::microseconds>()) {
            return "us";
        } else if constexpr (std::is_same<ChronoDurationCast, std::chrono::milliseconds>()) {
            return "ms";
        } else if constexpr (std::is_same<ChronoDurationCast, std::chrono::seconds>()) {
            return "s";
        }
    }

public:
    ScopedTimer(std::string_view name) : m_start_time(std::chrono::steady_clock::now()), m_name(name) {}
    ~ScopedTimer() {
        const auto end_time = std::chrono::steady_clock::now();
        const auto duration = end_time - m_start_time;
        if constexpr (Verbose) {
            fmt::print("TIMING FOR {} TOOK: {}{}\n", m_name, std::chrono::duration_cast<ChronoDurationCast>(duration).count(), as_string());
        } else {
            fmt::print("{}{}\n", std::chrono::duration_cast<ChronoDurationCast>(duration).count(), as_string());
        }
    }
};
