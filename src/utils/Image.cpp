#include "Image.hpp"
#include <string_view>
#include "Panic.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image(std::string_view filename) {
    img = stbi_load(filename.data(), &width, &height, &channels, 0);
    if(img == nullptr) {
        panic("Failed to load image {}", filename);
    }
}

Image::~Image() {
    stbi_image_free(img);
    img = nullptr;
}