#pragma once
#include <concepts>

template<typename T, typename... Ts>
concept same_as_any = (... or std::same_as<T, Ts>);