#pragma once

#include <fmt/format.h>
#include <source_location>


// https://buildingblock.ai/panic
template <class... Args>
struct panic_format {
    template <class T>
    consteval panic_format(    // note: consteval is what allows for compile-time checking of the
        const T &s,            //       format string
        std::source_location loc = std::source_location::current()
    ) noexcept : fmt{s}, loc{loc} {}

    fmt::format_string<Args...> fmt;
    std::source_location loc;
};

template <class... Args>
[[noreturn]] void panic(
    panic_format<std::type_identity_t<Args>...> fmt, // std::type_identity_t is needed to prevent
    Args &&...args) noexcept                         // type deduction of the format string's
{                                                  // arguments.
    fmt::println("{}:{} panic: {}",
        fmt.loc.file_name(), 
        fmt.loc.line(),
        fmt::format(fmt.fmt, std::forward<Args>(args)...)
    );
    exit(1);
}