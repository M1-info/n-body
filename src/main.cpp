#include "main.h"

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

    std::vector<Body> bodies = std::vector<Body>(2);

    for (auto body : bodies)
    {
        body.setForces(glm::vec2(0.0f));
        body.setMass(1.0f);
    }

    printf("Number of tasks = %d. My rank = %d. Running on %s\n", world_size, current_rank, hostname);

    if (current_rank == 0)
    {
        // init render
        Render render(800, 600);
        render.init();

        // main loop
        render.render();
    }

    MPI_Finalize();
    return 0;
}