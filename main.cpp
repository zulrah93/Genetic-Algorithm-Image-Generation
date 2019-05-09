#include <signal.h>
#include "helper.h"
#include "gif.h"


#define POPULATION_SIZE 5000



bool console_mode = false;

GifWriter writer;

void sighandler(int handler) {
    GifEnd(&writer);
    cout << "Application terminated and ouput.gif was saved!" << endl;
    quit = true;
}

int main(int argc, const char * argv[]) {

    console_mode = (argc > 1 && strcmp(argv[1], "-console") == 0);

    if (console_mode)
        cout << "Using console mode..." << endl;

    if(!console_mode && SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto window = console_mode ? nullptr : SDL_CreateWindow("Genetic Algorithm Image Generation", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if(window == nullptr && !console_mode) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }   

    auto windowSurface = console_mode ? nullptr : SDL_GetWindowSurface(window);

    SDL_Event event;

    image target(window, "apple.bmp", []() {
         std::cout << "Failed to load target image: " << SDL_GetError() << std::endl;
         SDL_Quit();
    });

    GifBegin(&writer, "output.gif", target.get_width(), target.get_height(), 100);
    signal(SIGINT, sighandler);

    for(int i = 0; i < POPULATION_SIZE; i++)
    {   
        auto goal = target.data();
        if (!goal) continue;
        population.emplace_back(goal, target.data_length(), window, target.get_width(), target.get_height());
    }

    sort(population.begin(), population.end()); // Sort by fitness.

    bool found = false;
    uint64_t generation = 0;

    // Thread that does the brunt of the work for the genetic algorithm
    thread populationThread([]() {
            srand(time(nullptr));

            while(!quit) {


                vector<individual> new_gen; 
                int s = (10*POPULATION_SIZE)/100; // 10 percent of the fittest go on to the next gen
                for(int i = 0;i<s;i++) {
                    auto& individual = population[i];
                    new_gen.push_back(individual); 
                }
                
                s = (90*POPULATION_SIZE)/100; 
                for(int i = 0;i<s;i++) {
                    size_t index = rand() % (POPULATION_SIZE/2);
                    size_t index2 = rand() % (POPULATION_SIZE/2);
                    if (index == index2) {
                        index++;
                        index %= POPULATION_SIZE;
                    }
                    auto& parent1 = population[index];
                    auto& parent2 = population[index2];
                    new_gen.push_back(parent1.mate(parent2)); // Mate with two random individuals for this case it is possible for the same individual to mate with itself
                }

                s = (10*POPULATION_SIZE)/100; 
                for(int i = s; i < population.size(); i++) {
                    auto& individual = population[i];
                    individual.dispose(console_mode); // Cleanup memory of the individuals that have ceased to exist
                }

                population = new_gen;

                sort(population.begin(), population.end()); // Sort by fitness. 
            }
    });

    
    thread serverThread(run_server, 8080);
    

    clock_t prev = clock();
    
    int previous_fitness = 0;
    
    while(!quit) {
        while(!console_mode && SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        auto& most_fittest = population[0];

        if (most_fittest.get_fitness() <= 0) {
                quit = true; 
                GifEnd(&writer);
                continue;
        }

        generation++;

        auto current_fitness = most_fittest.get_fitness();

        if (initial_fitness == -1) 
            initial_fitness = current_fitness;
        if (current_fitness < min_fitness)
            min_fitness = current_fitness;
        if (current_fitness > max_fitness)
            max_fitness = current_fitness;

        auto elapsed = static_cast<double>(clock()-prev)/CLOCKS_PER_SEC;

        if (elapsed > 300) { // Every 5 minutes
            if (!console_mode) {
                most_fittest.render(); 
            }
            auto data = most_fittest.data();
            GifWriteFrame(&writer, data, target.get_width(), target.get_height(), 17); // 60 FPS
            prev = clock();
        }

        previous_fitness = current_fitness;
    }

    return 0;
}