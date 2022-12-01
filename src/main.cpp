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
    glm::vec2 position = glm::vec2(rand() % 6 + 1, rand() % 6 + 1);
    glm::vec2 velocity = glm::vec2(rand() % 6 + 1, rand() % 6 + 1);
    float mass = rand() % 6 + 1;

    body->setMass(mass);
    body->setPosition(position);
    body->setVelocity(velocity);

    body->debug();


    while (true)
    {
         auto end = std::chrono::high_resolution_clock::now();
         std::chrono::duration<double> elapsed_time = end - last;

        if (elapsed_time.count() >= TIME_STEP)
        {
            // update bodies
            glm::vec2 pos = body->getPosition();
            std::vector<float> data;
            data.resize(4);
            data = { body->getId(), body->getMass(), pos[0], pos[1] };
            std::vector<float> received_data;
            received_data.resize(4 * world_size);
            

            MPI_Allgather(data.data(), 4, MPI_FLOAT, received_data.data(), 4, MPI_FLOAT, MPI_COMM_WORLD);

            glm::vec2 forces_on_body(0,0);

            int taille = sizeof(received_data) / sizeof(float);
            for(int i=0; i<taille; i+=4){
                float id = received_data[i];

                if(id == body->getId())
                    continue;

                float mass_other = received_data[i+1];
                glm::vec2 position_other (received_data[i+2], received_data[i+3]);

                
                forces_on_body += body->computeForces(mass_other, position_other);
            }

            forces_on_body *= -GRAVITY_CONSTANT * body->getMass();
            body->setForces(forces_on_body);
            //std::cout << "forces on particules : " << forces_on_body.x << " - " << forces_on_body.y << std::endl;
            body->computePosition((float)elapsed_time.count());
            body->computeVelocity((float)elapsed_time.count());

            body->debug();


            last = end;
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "elapsed time from start : " << elapsed_seconds.count() << "s" << std::endl;
        }
    }
    

    MPI_Finalize();
    return 0;
}