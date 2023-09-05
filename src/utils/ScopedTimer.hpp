#pragma once

#include <chrono>
#include <ratio>
#include <type_traits>
#include <concepts>
#include <fmt/format.h>



template <typename ChronoDurationCast>
class ScopedTimer  {
    std::chrono::steady_clock::time_point m_start_time;
    std::string m_name;
    constexpr std::string_view as_string();

public:

    ScopedTimer(std::string_view name) : m_start_time(std::chrono::steady_clock::now()), m_name(name) {}
    ~ScopedTimer() {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = end_time - m_start_time;
        fmt::println("TIMING FOR {} TOOK: {}{}", m_name, std::chrono::duration_cast<ChronoDurationCast>(duration).count(), as_string());
    }
};

template<>
constexpr std::string_view ScopedTimer<std::chrono::nanoseconds>::as_string() {
    return "ns";
}

template<>
constexpr std::string_view ScopedTimer<std::chrono::microseconds>::as_string() {
    return "us";
}

template<>
constexpr std::string_view ScopedTimer<std::chrono::milliseconds>::as_string() {
    return "ms";
}

template<>
constexpr std::string_view ScopedTimer<std::chrono::seconds>::as_string() {
    return "s";
}