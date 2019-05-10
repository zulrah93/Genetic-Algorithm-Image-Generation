#include "image.h"
#include <iostream>


image::image() {
    window = nullptr;
    img = nullptr;
    windowSurface = nullptr;
}

image::image(SDL_Window *window, int32_t w, int32_t h) { // Create blank surface
        this->window = window;
        this->windowSurface = SDL_GetWindowSurface(window);
        this->img = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
}

image::image(SDL_Window *window, string path, function<void()> errorCallback)
{
    this->window = window;
    this->img = SDL_LoadBMP(path.c_str());
    if (window)
        this->windowSurface = SDL_GetWindowSurface(window);
    if (this->img == nullptr) {
        errorCallback(); // Call the error callback if one of the handles is null
    }
}

uint8_t image::get_pixel_at(uint8_t x, uint8_t y) { //Get the pixel at (x,y)

    uint8_t pixel = static_cast<uint8_t*>(img->pixels)[(x * img->w)+ y];
    uint8_t r, g, b;
    SDL_GetRGB(pixel, img->format, &r, &g, &b);
    return pixel;
}

void image::set_pixel(uint8_t x, uint8_t y, uint8_t pixel) { // Set the pixel at (x,y)
    if (SDL_LockSurface(img) == -1) throw "Surface Couldn't Be Locked!";
    static_cast<uint8_t*>(img->pixels)[(x * img->w)+ y] = pixel;
    SDL_UnlockSurface(img);
}

void image::set_data(uint8_t* &data) {
    if (SDL_LockSurface(img) == -1) throw "Surface Couldn't Be Locked!";
    memcpy(img->pixels, data, (img->w*img->h)*4);
    SDL_UnlockSurface(img);
}

void image::render() {
    SDL_BlitScaled(this->img, nullptr, windowSurface, nullptr);
    SDL_UpdateWindowSurface(window);
}

image::~image() {
    
}

void image::dispose() {
    if (img && img->refcount > 0)
        SDL_FreeSurface(img);
}

size_t image::data_length() {
    return (img->w * img->h) * sizeof(uint32_t);
}

const uint8_t* image::data() {
    return static_cast<uint8_t*>(img->pixels);
}

int32_t image::get_width() {
    return img->w;
}

int32_t image::get_height() {
    return img->h;
}

bool image::is_disposed() {
    if (img == nullptr || window == nullptr || windowSurface == nullptr)
        return true;
    return (img && img->refcount <= 0); 
}


individual::individual(const uint8_t *target, size_t length, SDL_Window *window, int32_t w, int32_t h)
{
    this->fitness = 0;
    this->target = target;
    this->length = length;
    this->chromosone = new uint8_t[length];
    this->window = window;
    this->img = new image(window, w, h);
    for(size_t i = 0; i < length; i++) // Create random genes
    {
        this->chromosone[i] = rand() % 255;
        if (target[i] != this->chromosone[i])
            fitness++;
    }
}

individual individual::mate(individual parent)
{
    individual child(target, length, window, img->get_width(), img->get_height());
    float p = rand() / static_cast<float>(RAND_MAX);
    child.fitness = 0;
    for(size_t i = 0; i < length; i++) {
            if (p < 0.45) { // Insert parents genome
                    child.chromosone[i] = this->chromosone[i];
            }
            else if (p < 0.70) {
                child.chromosone[i] = parent.chromosone[i];
            }
            else {
                child.chromosone[i] = rand() % 0xFF; // Mutation
            }
            if (child.chromosone[i] != target[i])
                child.fitness++;
    }
    return child;
}

void individual::render() {
    if (img && !img->is_disposed() && chromosone) {
        img->set_data(chromosone);
        img->render();
    }
}

const uint8_t* individual::data() {
    return chromosone;
}

int32_t individual::get_fitness() {
    return fitness;
}

int32_t individual::get_width()
{
    if (!img) return 0;
    return img->get_width();
}

int32_t individual::get_height()
{
    if (!img) return 0;
    return img->get_height();
}

bool individual::operator<(individual ind) {
    return this->get_fitness() < ind.get_fitness();
}

void individual::dispose(bool console_mode) {
    if (console_mode) {
        img->dispose();
        delete chromosone;
        delete img;
        img = nullptr;
        chromosone = nullptr;
    }
    else {
        if (img->is_disposed()) // Prevent double-free
            return;
        delete chromosone;
        delete img;
        img = nullptr;
        chromosone = nullptr;
    }
}