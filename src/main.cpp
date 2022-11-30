#include "main.h"
#include <chrono>
#include <unistd.h>

std::vector<Body *> bodies;

void run()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "good morning" << std::endl;
}

int main(int argc, char *argv[])
{

    // float initialTime = clock();
    // int elapsedTime = 0;

    srand(time(NULL));

    // init MPI
    MPI_Init(&argc, &argv);

    // get number of tasks
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // get current machine rank
    int current_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &current_rank);

    // get current processor name
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int name_length;
    MPI_Get_processor_name(hostname, &name_length);

    printf("Number of tasks = %d. My rank = %d. Running on %s\n", world_size, current_rank, hostname);

    if (current_rank == 0)
    {
        /* No opengl for the moment */

        /* No opengl for the moment
        // init render
        Render render(800, 600);
        render.init();

        // main loop
        render.render();
        */

        for (int i = 0; i < world_size; i++)
        {
            Body *body = new Body();

            glm::vec2 position = glm::vec2(rand() % 6, rand() % 6);
            glm::vec2 velocity = glm::vec2(rand() % 6, rand() % 6);
            float mass = rand() % 6;

            body->setMass(mass);
            body->setPosition(position);
            body->setVelocity(velocity);
            bodies.push_back(body);

            body->debug();
        }

        auto start = std::chrono::high_resolution_clock::now();
        auto last = start;

        while (true)
        {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_time = end - last;

            if (elapsed_time.count() >= TIME_STEP)
            {
                // update bodies

                last = end;
                std::chrono::duration<double> elapsed_seconds = end - start;
                std::cout << "elapsed time from start : " << elapsed_seconds.count() << "s" << std::endl;
            }
        }
    }

    MPI_Finalize();
    return 0;
}