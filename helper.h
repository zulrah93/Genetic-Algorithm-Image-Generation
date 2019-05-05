#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include <random>
#include <vector>
#include <algorithm> 

using namespace std;

class image
{
    private:
    SDL_Window *window;
    SDL_Surface *img;
    SDL_Surface *windowSurface;

    public:

        image() {
            window = nullptr;
            img = nullptr;
            windowSurface = nullptr;
        }

        image(SDL_Window *window, int w, int h) { // Create blank surface
                this->window = window;
                this->windowSurface = SDL_GetWindowSurface(window);
                this->img = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
        }
        
        image(SDL_Window *window, string path, function<void()> errorCallback)
        {
            this->window = window;
            this->img = SDL_LoadBMP(path.c_str());
            this->windowSurface = SDL_GetWindowSurface(window);
            if (this->img == nullptr || this->window == nullptr || this->windowSurface == nullptr) {
                errorCallback(); // Call the error callback if one of the handles is null
            }
        }

        uint8_t get_pixel_at(uint8_t x, uint8_t y) { //Get the pixel at (x,y)

            uint8_t pixel = static_cast<uint8_t*>(img->pixels)[(x * img->w)+ y];
            uint8_t r, g, b;
            SDL_GetRGB(pixel, img->format, &r, &g, &b);
            return pixel;
        }

        void set_pixel(uint8_t x, uint8_t y, uint8_t pixel) { // Set the pixel at (x,y)
            if (SDL_LockSurface(img) == -1) throw "Surface Couldn't Be Locked!";
            static_cast<uint8_t*>(img->pixels)[(x * img->w)+ y] = pixel;
            SDL_UnlockSurface(img);
        }

        void set_data(uint8_t* &data) {
            if (SDL_LockSurface(img) == -1) throw "Surface Couldn't Be Locked!";
            memcpy(img->pixels, data, (img->w*img->h)*4);
            SDL_UnlockSurface(img);
        }

        void render() {
            SDL_BlitScaled(this->img, nullptr, windowSurface, nullptr);
            SDL_UpdateWindowSurface(window);
        }

        ~image() {
            
        }

        void dispose() {
            if (img && img->refcount > 0)
                SDL_FreeSurface(img);
        }

        size_t data_length() {
            return (img->w * img->h) * sizeof(uint32_t);
        }

        const uint8_t* data() {
            return static_cast<uint8_t*>(img->pixels);
        }

        int get_width() {
            return img->w;
        }

        int get_height() {
            return img->h;
        }

};


class individual
{
     private:
        uint8_t* chromosone;
        size_t length; // Length of the chromosone
        const uint8_t* target;
        image img;
        int fitness;
     public:
        individual(const uint8_t *target, size_t length)
        {
            srand(time(NULL));
            this->fitness = 0;
            this->target = target;
            this->length = length;
            this->chromosone = new uint8_t[length];
            for(size_t i = 0; i < length; i++) // Create random genes
            {
                this->chromosone[i] = rand() % 255;
                if (target[i] != this->chromosone[i])
                    fitness++;
            }
        }

        individual mate(individual parent)
        {
            individual child(target, length);
            srand(time(nullptr));
            double p = rand() / static_cast<double>(RAND_MAX);
            child.fitness = 0;
            for(size_t i = 0; i < length; i++) {
                    if (p < 0.45) { // Insert parents genome
                            child.chromosone[i] = this->chromosone[i];
                    }
                    else if (p < 0.90) {
                        child.chromosone[i] = parent.chromosone[i];
                    }
                    else {
                        child.chromosone[i] = rand() % 255; // Mutation
                    }
                    if (child.chromosone[i] != target[i])
                        child.fitness++;
            }
            return child;
        }

        void render(SDL_Window *window, int w, int h) {
            img = image(window, w, h);
            img.set_data(chromosone);
            img.render();
        }

        int get_fitness() {
            return fitness;
        }

        bool operator<(individual ind) {
            return this->get_fitness() < ind.get_fitness();
        }

        void dispose() {
            img.dispose();
            delete chromosone;
        }

};