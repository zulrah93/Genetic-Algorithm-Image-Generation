#include <iostream>
#include "helper.h"

#define POPULATION_SIZE 100



int main(int argc, const char * argv[]) {

    SDL_Color colors[256];

    for(int i=0;i<256;i++){
        colors[i].r=i;
        colors[i].g=i;
        colors[i].b=i;
    } 

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto window = SDL_CreateWindow("Genetic Algorithm Image Generation", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if(window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }   

    auto windowSurface = SDL_GetWindowSurface(window);
    
    SDL_SetPaletteColors(windowSurface->format->palette, colors, 0, 256);

    SDL_Event event;

    image target(window, "/home/zulrah/projects/Genetic Algorithm Image/red.bmp", []() {
         std::cout << "Failed to load target image: " << SDL_GetError() << std::endl;
         SDL_Quit();
    });

    vector<individual> population;

    for(int i = 0; i < POPULATION_SIZE; i++)
    {   
        auto goal = target.data();
        if (!goal) continue;
        population.emplace_back(goal, target.data_length());
    }

    bool found = false;
    bool quit = false;
    int generation = 0;

    clock_t prev = clock();
    
    while(!quit) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        auto& most_fittest = population[0];

        sort(population.begin(), population.end());

        most_fittest.render(window, target.get_width(), target.get_height()); 

        if (most_fittest.get_fitness() <= 0) {
                continue;
        }

        srand(time(nullptr));
        vector<individual> new_gen; 
        int s = (10*POPULATION_SIZE)/100; // 10 percent of the fittest go on to the next gen
        for(int i = 0;i<s;i++) 
            new_gen.push_back(population[i]); 
        
        s = (90*POPULATION_SIZE)/100; 
        for(int i = 0;i<s;i++) {
            size_t index = rand() % s;
            size_t index2 = rand() % s;
            auto& parent1 = population[index];
            auto& parent2 = population[index2];
            new_gen.push_back(parent1.mate(parent2)); // Mate with two random individuals for this case it is possible for the same individual to mate with itself
        }

        s = (10*POPULATION_SIZE)/100; 
        for(int i = s; i < population.size(); i++)
            population[i].dispose(); // Cleanup memory

        population = new_gen; 

        generation++;

        if (generation == 1 || generation % 5000 == 0) {
            auto elapsed = static_cast<double>(clock()-prev)/CLOCKS_PER_SEC;
            cout << "Generation: " << generation << " Fitness: " << most_fittest.get_fitness() << endl
            << "This Generation Took: " << elapsed << " seconds." << endl;
            prev = clock();
        }
    }

    return 0;
}