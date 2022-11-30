#include "main.h"

std::vector<Body *> bodies;

int main(int argc, char *argv[])
{

    auto start = std::chrono::high_resolution_clock::now();
    auto last = start;

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

    srand(time(NULL) + current_rank);

    Body *body = new Body();
    glm::vec2 position = glm::vec2(rand() % 6, rand() % 6);
    glm::vec2 velocity = glm::vec2(rand() % 6, rand() % 6);
    float mass = rand() % 6;

    body->setMass(mass);
    body->setPosition(position);
    body->setVelocity(velocity);
    std::cout << body->getId() << std::endl;
    

    while (true)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = end - last;

        if (elapsed_time.count() >= TIME_STEP)
        {
            // update bodies
            glm::vec2 pos = body->getPosition();
            float data[4] = { body->getId(), body->getMass(), pos[0], pos[1] };
            float received_data[4 * world_size];
            

            MPI_Allgather(&data, 4, MPI_FLOAT, &received_data, 4, MPI_FLOAT, MPI_COMM_WORLD);

            int taille = sizeof(received_data) / sizeof(float);
            for(int i=0; i<taille; i+=4){
                std::cout << received_data[i] << " - " << received_data[i+1] << " - " << received_data[i+2] << " - " << received_data[i+3] << std::endl;
            }

            last = end;
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "elapsed time from start : " << elapsed_seconds.count() << "s" << std::endl;
        }
    }
    

    MPI_Finalize();
    return 0;
}