#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <functional>

using std::function;
using std::string;

class image
{
    private:
    SDL_Window *window;
    SDL_Surface *img;
    SDL_Surface *windowSurface;

    public:

        image();

        image(SDL_Window *window, int w, int h);
        
        image(SDL_Window *window, string path, function<void()> errorCallback);

        uint8_t get_pixel_at(uint8_t x, uint8_t y);

        void set_pixel(uint8_t x, uint8_t y, uint8_t pixel);

        void set_data(uint8_t* &data);

        void render();

        ~image();

        void dispose();

        size_t data_length();

        const uint8_t* data();

        int get_width();

        int get_height();

        bool is_disposed();

};


class individual
{
     private:
        SDL_Window *window;
        uint8_t* chromosone;
        size_t length; // Length of the chromosone
        const uint8_t* target;
        image *img;
        int fitness;
     public:
        individual(const uint8_t *target, size_t length, SDL_Window *window, int w, int h);

        individual mate(individual parent);

        void render();

        const uint8_t* data();

        int get_fitness();

        bool operator<(individual ind);

        void dispose(bool console_mode);

};