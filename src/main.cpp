#include "main.h"

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

    // number of bodies for each process
    int nb_body;

    // number of bodies left (if NB_BODY % world_size != 0)
    int nb_body_left;

    // compute nb_body
    if (current_rank == 0)
    {
        if(world_size > 1)
            nb_body = floor(NB_BODY / (world_size - 1));
        else
            nb_body = NB_BODY;
    }

    // broadcast nb_body to all processes
    MPI_Bcast(&nb_body, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // compute nb_body_left
    nb_body_left = NB_BODY - nb_body * (world_size - 1);

    // buffer for recvcounts and displs (used in MPI_Allgatherv)
    int recvcounts[world_size];
    recvcounts[0] = nb_body_left * DATA_SIZE;

    int displs[world_size];
    displs[0] = 0;

    for (int i = 1; i < world_size; i++)
    {
        recvcounts[i] = nb_body * DATA_SIZE;
        displs[i] = (nb_body_left + nb_body * (i - 1)) * DATA_SIZE;
    }

    if (current_rank == 0 && world_size > 1)
        nb_body = nb_body_left;

    // create nb_body bodies
    std::vector<Body *> bodies;
    for (int i = 0; i < nb_body; ++i)
    {
        Vec2 position = Vec2(randMinmax(0, 10e3), randMinmax(0, 10e3));
        Vec2 velocity = Vec2(0, 0);
        NBodyType mass = randMinmax(10e20, 10e30);

        Body *body = new Body(position, velocity, mass, 0);
        bodies.push_back(body);

        // body->debug();
    }

    // buffers for MPI communication
    NBodyType data[DATA_SIZE * nb_body];
    NBodyType received_data[DATA_SIZE * NB_BODY];

    /* Main loop */
    for (int i = 0; i < NB_ITERATIONS; i++)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<NBodyType> elapsed_time = end - last;

        // if (elapsed_time.count() >= TIME_STEP) // refresh rate
        // {
            /*
                Update bodies position and velocity
                - each body send his mass and position to other bodies in order to compute attractive forces
                - so MPI communication : MPI_Allgather
                - we store data sent and data received in a vector
            */

            // fill data buffer
            for (int i = 0; i < nb_body; i++)
            {
                data[i * DATA_SIZE] = bodies[i]->getId();
                data[i * DATA_SIZE + 1] = bodies[i]->getMass();
                data[i * DATA_SIZE + 2] = bodies[i]->getPosition().x;
                data[i * DATA_SIZE + 3] = bodies[i]->getPosition().y;
            }

            MPI_Allgatherv(
                &data[0],
                DATA_SIZE * nb_body,
                MPI_DOUBLE,
                &received_data[0],
                &recvcounts[0],
                &displs[0],
                MPI_DOUBLE,
                MPI_COMM_WORLD
            );

            /* For each data body received (id, mass and position), compute forces applied on the body */
            for (Body *body : bodies)
            {
                Vec2 forces_on_body(0, 0);

                for (int i = 0; i < DATA_SIZE * NB_BODY; i += DATA_SIZE)
                {
                    NBodyType id = received_data[i];

                    // check if body is not the same
                    if (id == body->getId()) 
                        continue;

                    // get mass and position of the other body
                    NBodyType mass_other = received_data[i + 1];
                    Vec2 position_other(received_data[i + 2], received_data[i + 3]);

                    // compute forces applied on the body
                    forces_on_body += body->computeForces(mass_other, position_other);
                }

                forces_on_body *= -GRAVITY_CONSTANT * body->getMass();
                body->setForces(forces_on_body);

                std::chrono::duration<NBodyType> new_elapsed_time = end - last;

                /* compute the new position and velocity of the body */
                body->computePosition(new_elapsed_time.count());
                body->computeVelocity(new_elapsed_time.count());

                // body->debug();
            }

            last = end;
            std::chrono::duration<NBodyType> elapsed_seconds = end - start;
        // }
    }

    for(Body *body : bodies){
        // body->debug();
        delete body;
    }

    MPI_Finalize();
    return 0;
}