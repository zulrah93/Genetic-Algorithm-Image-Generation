#include "bitmap.h"

 

#pragma pack(1)

struct rgb {
    uint8_t r, g, b;
};

struct bitmap_header {
    uint16_t   type;           // Magic number for file 
    uint32_t   size;           // Size of file 
    uint16_t   reserved1;      // Reserved 
    uint16_t   reserved2;      // Reserved 
    uint32_t   offset;        // Offset to bitmap data 
    uint32_t   header_size;           // Size of info header 
    int32_t    width;          // Width of image 
    int32_t    height;         // Height of image 
    uint16_t   planes_count;         // Number of color planes 
    uint16_t   bit_count;       // Number of bits per pixel 
    uint32_t   compression;    // Type of compression to use 
    uint32_t   image_data_size;      // Size of image data 
    int32_t    x_pixels_per_meter;  // X pixels per meter 
    int32_t    y_pixels_per_meter;  // Y pixels per meter 
    uint32_t   colors_used;        // Number of colors used 
    uint32_t   number_of_important_colors;   // Number of important colors 
    rgb unused;
};




bitmap::bitmap(const uint8_t* pixels, int32_t width, int32_t height)
{
    this->pixels = pixels;
    this->size = (width * height) * 4;
    this->width = width;
    this->height = height;
}

string bitmap::make()
{
    bitmap_header bmp = {0};
    bmp.width = this->width;
    bmp.height = this->height;
    bmp.type = 0x4D42;
    bmp.offset = sizeof(bitmap_header);
    bmp.bit_count = 24;
    bmp.planes_count = 1;
    bmp.header_size = 40;
    bmp.size = sizeof(bitmap_header) + this->size; // Total size of the bitmap file
    bmp.image_data_size = this->size;
    string result;
    auto data = (char*)&bmp;
    for(size_t i = 0; i < sizeof(bitmap_header); i++)
        result += data[i];
    for(size_t i = 0; i < this->size; i++)
        result += ((char*)this->pixels)[i];
    return result;
}

