#include "BMP.hpp"


void write_bmp_image(std::string_view filename, std::vector<Vec4<u8>> image, u32 width, u32 height) {
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
            Vec4<u8>& pixel = image[y * width + x];
            u8 rgb[3] = {pixel.y, pixel.x, pixel.w};
            file.write((char*)rgb, 3);
        }
    }

    // Close the file
    file.close();
}