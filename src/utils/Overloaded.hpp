#pragma once

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };