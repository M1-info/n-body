#include "main.h"
#include <chrono>
#include <unistd.h>

int main(int argc, char *argv[])
{

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
        /* No opengl for the moment
        // init render
        Render render(800, 600);
        render.init();

        // main loop
        render.render();
        */


        auto start = std::chrono::high_resolution_clock::now();
        int seconds_passed = 0;
        
        while (true)
        {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;

            if ((int)elapsed_seconds.count() > seconds_passed)
            {   
                seconds_passed++;
                printf("1 second elapsed\n");
                std::cout << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;
            }
        }
        

    }

    MPI_Finalize();
    return 0;
}