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

void write_bmp_image(std::string_view filename, std::vector<Vec4<u8>> image, u32 width, u32 height);