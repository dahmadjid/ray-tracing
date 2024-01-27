#pragma once

#include <string_view>
#include "utils/stb_image.h"
#include "types.hpp"

struct Image {
    i32 width;
    i32 height;
    i32 channels;
    u8* img = nullptr;
    Image(std::string_view filename);
    ~Image();
};