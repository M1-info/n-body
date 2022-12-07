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

    printf("Number of tasks = %d. My rank = %d. Running on %s\n", world_size, current_rank, hostname);

    srand(time(NULL) + current_rank);

    int nb_body;
    int nb_body_left;

    if (current_rank == 0)
    {

        if (world_size > 1)
        {
            /* Multiple tasks -> root manage and other tasks handle bodies */
            nb_body = floor(NB_BODY / (world_size - 1));

            // Say to other tasks how many bodies they have to handle
            for (int i = 1; i < world_size; i++)
            {
                MPI_Send(&nb_body, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

            nb_body_left = NB_BODY - nb_body * (world_size - 1);
        }
        else
        {
            /* Only one task -> root do everything */
            nb_body = NB_BODY;
        }
    }
    else
    {
        // Receive number of bodies to handle
        MPI_Recv(&nb_body, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        nb_body_left = NB_BODY - nb_body * (world_size - 1);
    }

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
    {
        nb_body = nb_body_left; // root handle remaining bodies
    }

    // create nb_body bodies
    std::vector<Body *> bodies;
    for (int i = 0; i < nb_body; ++i)
    {
        Vec2 position = Vec2(randMinmax(0, 10e30), randMinmax(0, 10e30));
        Vec2 velocity = Vec2(randMinmax(0, 10e4), randMinmax(0, 10e4));
        NBodyType mass = randMinmax(10e10, 10e30);

        Body *body = new Body(position, velocity, mass, 0);
        bodies.push_back(body);
    }

    // std::cout << bodies.size() << " bodies for rank " << current_rank << std::endl;

    NBodyType data[DATA_SIZE * nb_body];
    NBodyType received_data[DATA_SIZE * NB_BODY];

    /* Main loop */
    while (true)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<NBodyType> elapsed_time = end - last;

        if (elapsed_time.count() >= TIME_STEP) // refresh rate
        {
            /*
                Update bodies position and velocity
                - each body send his mass and position to other bodies in order to compute attractive forces
                - so MPI communication : MPI_Allgather
                - we store data sent and data received in a vector
            */

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

            // print received data
            for (int i = 0; i < DATA_SIZE * NB_BODY; i += DATA_SIZE)
            {
                std::cout << "received_data[" << i * DATA_SIZE << "] = " << received_data[i * DATA_SIZE] << std::endl;
                std::cout << "received_data[" << i * DATA_SIZE + 1 << "] = " << received_data[i * DATA_SIZE + 1] << std::endl;
                std::cout << "received_data[" << i * DATA_SIZE + 2 << "] = " << received_data[i * DATA_SIZE + 2] << std::endl;
                std::cout << "received_data[" << i * DATA_SIZE + 3 << "] = " << received_data[i * DATA_SIZE + 3] << std::endl;
            }

            /* For each data body received (id, mass and position), compute forces applied on the body */
            for (Body *body : bodies)
            {
                Vec2 forces_on_body(0, 0);

                for (int i = 0; i < DATA_SIZE * NB_BODY; i += DATA_SIZE)
                {
                    NBodyType id = received_data[i];

                    if (id == body->getId()) // the body also receive his data, no computation in this case
                        continue;

                    NBodyType mass_other = received_data[i * DATA_SIZE + 1];
                    Vec2 position_other(received_data[i * DATA_SIZE + 2], received_data[i * DATA_SIZE + 3]);

                    forces_on_body += body->computeForces(mass_other, position_other);
                }

                forces_on_body *= -GRAVITY_CONSTANT * body->getMass();
                /* We now have force applied on the body, we can compute the new position and velocity of the body */
                body->setForces(forces_on_body);

                std::chrono::duration<NBodyType> new_elapsed_time = end - last;
                body->computePosition(new_elapsed_time.count());
                body->computeVelocity(new_elapsed_time.count());
            }

            last = end;
            std::chrono::duration<NBodyType> elapsed_seconds = end - start;
        }
    }

    MPI_Finalize();
    return 0;
}