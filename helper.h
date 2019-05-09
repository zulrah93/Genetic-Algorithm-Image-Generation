#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include <random>
#include <vector>
#include <algorithm>
#include <thread>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using std::vector;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::to_string;
using std::sort;
using std::thread;
using std::function;

bool quit = false;

int min_fitness = INT32_MAX;
int max_fitness = 0;
int initial_fitness = -1;

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
            if (window)
                this->windowSurface = SDL_GetWindowSurface(window);
            if (this->img == nullptr) {
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

        bool is_disposed() {
            if (img == nullptr || window == nullptr || windowSurface == nullptr)
                return true;
            return (img && img->refcount <= 0); 
        }

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
        individual(const uint8_t *target, size_t length, SDL_Window *window, int w, int h)
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

        individual mate(individual parent)
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

        void render() {
            if (img && !img->is_disposed() && chromosone) {
                img->set_data(chromosone);
                img->render();
            }
        }

        const uint8_t* data() {
            return chromosone;
        }

        int get_fitness() {
            return fitness;
        }

        bool operator<(individual ind) {
            return this->get_fitness() < ind.get_fitness();
        }

        void dispose(bool console_mode) {
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

};

vector<individual> population;

sockaddr_in get_ip_address(const char* hostname, int port){
	sockaddr_in ipa;
	ipa.sin_family = AF_INET;
	ipa.sin_port = htons(port);

	auto host = gethostbyname(hostname);
	if(!host){
        cout << "Issue resolving host!" << endl;
		exit(1);
	}

	auto addr = host->h_addr_list[0];
	memcpy(&ipa.sin_addr.s_addr, addr, sizeof addr);

	return ipa;
}

void run_server(int port) {

    auto tcp = getprotobyname("tcp");
    auto fd = socket(AF_INET, SOCK_STREAM, tcp->p_proto);

    if (fd == -1) {
        cout << "Error: Opening socket!" << endl;
        exit(1);
    }

    auto ip = get_ip_address("plexserver", port);


    if(bind(fd, (sockaddr*)&ip, sizeof(ip)) == -1) {
        cout << "Error: Failed to bind!" << endl;
        exit(1);
    }

    if (listen(fd, 1) == -1) {
        cout << "Error: Failed to listen!" << endl;
    }

    cout << "Listening on port " << port << "!" << endl;

    while(!quit) {

        auto cd = accept(fd, nullptr, nullptr);

        auto& most_fittest = population[0];

        auto message =  // TODO HTML Builder class
        "<html><body>Initial Fitness: " 
        + to_string(initial_fitness) 
        + "<br>Minimum Fitness: " 
        + to_string(min_fitness) + 
        "<br>Maximum Fitness:" + to_string(max_fitness) 
        + "<br>Current Fitness: "  + to_string(most_fittest.get_fitness()) + 
        "</body></html>";

        string http_response =  //Create the HTTP response
        string("HTTP/1.1 200 OK\r\n") +
        "Content-Length: " + to_string(message.size()) + "\r\n" +
        "Content-Type: text/html\r\n\r\n" +
        message;
       
        auto status = send(cd, http_response.c_str(), http_response.size(), MSG_NOSIGNAL); // Send the HTTP response to the client

        shutdown(cd, SHUT_RDWR);

    }

}