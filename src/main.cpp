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


    /* For the moment, 1 body by task */

    // Body *body = new Body();
    // Vec2 position = Vec2(rand() % 6 + 1, rand() % 6 + 1);
    // Vec2 velocity = Vec2(rand() % 6 + 1, rand() % 6 + 1);
    // NBodyType mass = rand() % 6 + 1;

    // body->setMass(mass);
    // body->setPosition(position);
    // body->setVelocity(velocity);

    // body->debug();



    // DEBUG : set values

    Body *body = new Body();

    if(current_rank == 0){                      // terre
        body->setPosition(Vec2(4,6));
        body->setVelocity(Vec2(6,1));
        body->setMass(5.972e24);
    } else if(current_rank == 1){               // lune
        body->setPosition(Vec2(6,2));
        body->setVelocity(Vec2(1,6));
        body->setMass(7.36e22);
    } else {                                    // soleil
        body->setPosition(Vec2(3,6));
        body->setVelocity(Vec2(3,2));
        body->setMass(1.98892e30);
    }



    /* Main loop */
    while (true)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = end - last;

        if (elapsed_time.count() >= TIME_STEP) // refresh rate
        {
            /*
                Update bodies position and velocity
                - each body send his mass and position to other bodies in order to compute attractive forces
                - so MPI communication : MPI_Allgather
                - we store data sent and data received in a vector
            */
        
            std::vector<NBodyType> data;
            data.resize(DATA_SIZE);
            Vec2 pos = body->getPosition();
            data = { body->getId(), body->getMass(), pos.x, pos.y};

            std::vector<NBodyType> received_data;
            received_data.resize(DATA_SIZE * world_size);
            

            MPI_Allgather(data.data(), DATA_SIZE, MPI_DOUBLE, received_data.data(), DATA_SIZE, MPI_DOUBLE, MPI_COMM_WORLD);

            Vec2 forces_on_body(0,0);

            /* For each data body received (id, mass and position), compute forces applied on the body */
            for(int i=0; i<received_data.size(); i+=DATA_SIZE){
                NBodyType id = received_data[i];

                if(id == body->getId())     // the body also receive his data, no computation in this case
                    continue;

                NBodyType mass_other = received_data[i+1];
                Vec2 position_other (received_data[i+2], received_data[i+3]);
                forces_on_body += body->computeForces(mass_other, position_other);
            }

            forces_on_body *= -GRAVITY_CONSTANT * body->getMass();
            /* We now have force applied on the body, we can compute the new position and velocity of the body */
            body->setForces(forces_on_body);
            //std::cout << "forces on particules : " << forces_on_body.x << " - " << forces_on_body.y << std::endl;
            //std::cout << "forces on particules of " << current_rank << " : " << forces_on_body.x << " - " << forces_on_body.y << std::endl;
            
            body->computePosition((NBodyType)elapsed_time.count());
            body->computeVelocity((NBodyType)elapsed_time.count());



            //body->debug();


            last = end;
            std::chrono::duration<double> elapsed_seconds = end - start;
            //std::cout << "elapsed time from start : " << elapsed_seconds.count() << "s" << std::endl;
        }
    }
    

    MPI_Finalize();
    return 0;
}