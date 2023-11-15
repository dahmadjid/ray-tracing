#pragma once

#include "linear_algebra/Vec4.hpp"
#include "types.hpp"
#include <fstream>
#include <string_view>
#include <memory>
#include <array>
#include "Pack.hpp"


PACK(struct BMPHeader {             // Total: 54 bytes
  u16  type;             // Magic identifier: 0x4d42
  u32  size;             // File size in bytes
  u16  reserved1;        // Not used
  u16  reserved2;        // Not used
  u32  offset;           // Offset to image data in bytes from beginning of file (54 bytes)
  u32  dib_header_size;  // DIB Header size in bytes (40 bytes)
  u32   width_px;         // Width of the image
  u32   height_px;        // Height of image
  u16  num_planes;       // Number of color planes
  u16  bits_per_pixel;   // Bits per pixel
  u32  compression;      // Compression type
  u32  image_size_bytes; // Image size in bytes
  u32   x_resolution_ppm; // Pixels per meter
  u32   y_resolution_ppm; // Pixels per meter
  u32  num_colors;       // Number of colors  
  u32  important_colors; // Important colors 
});

template<u32 width, u32 height>
void write_bmp_image(std::string_view filename, std::array<Vec4<u8>, height * width>* image) {
    BMPHeader header;
    // Fill in BMP header fields (assumed values)
    header.type = 0x4d42;  // Magic identifier: "BM"
    header.size = sizeof(BMPHeader) + height * width * 3; // Total file size
    header.reserved1 = 0;  // Reserved fields set to 0
    header.reserved2 = 0;
    header.offset = sizeof(BMPHeader); // Offset to image data
    header.dib_header_size = 40; // DIB Header size
    header.width_px = width; // Width of the image
    header.height_px = height; // Height of the image
    header.num_planes = 1; // Number of color planes
    header.bits_per_pixel = 24; // Bits per pixel
    header.compression = 0; // Compression type (none)
    header.image_size_bytes = width * height * 3; // Image size in bytes
    header.x_resolution_ppm = 2835; // 72 DPI
    header.y_resolution_ppm = 2835; // 72 DPI
    header.num_colors = 0; // Number of colors in the color palette
    header.important_colors = 0; // All colors are important

    // Open file for writing in binary mode
    std::ofstream file(filename.data(), std::ios::out | std::ios::binary);

    // Write the header to the file
    file.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));

    // Write image data
    for (int y = height; y > 0; --y) {
        for (u32 x = 0; x < width; ++x) {
            // Assuming image is stored row-wise
            Vec4<u8>& pixel = image->operator[](y * width + x);
            u8 rgb[3] = {pixel.y, pixel.x, pixel.w};
            file.write((char*)rgb, 3);
        }
    }

    // Close the file
    file.close();
}