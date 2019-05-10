#pragma once
#include <cstdint>
#include <stddef.h>
#include <string>

using std::string;

class bitmap
{
    private:
        const uint8_t* pixels;
        size_t size;
        int width;
        int height;
    public:
        bitmap(const uint8_t* pixels, int32_t width, int32_t height);
        string make();

};