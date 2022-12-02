#include "main.h"

int main(int argc, char *argv[])
{

    auto start = std::chrono::high_resolution_clock::now();
    auto last = start;
    int nb_body;

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



    // DEBUG : set values

    // Body *body = new Body();

    // if(current_rank == 0){                      // terre
    //     body->setPosition(Vec2(4,6));
    //     body->setVelocity(Vec2(6,1));
    //     body->setMass(5.972e24);
    // } else if(current_rank == 1){               // lune
    //     body->setPosition(Vec2(6,2));
    //     body->setVelocity(Vec2(1,6));
    //     body->setMass(7.36e22);
    // } else {                                    // soleil
    //     body->setPosition(Vec2(3,6));
    //     body->setVelocity(Vec2(3,2));
    //     body->setMass(1.98892e30);
    // }


    if(current_rank==0){
        nb_body = floor(NB_BODY / (world_size - 1));

        // Envoi du nombre de points à calculer à chacun des noeuds
        for(int i=1; i<world_size; i++){
            MPI_Send(&nb_body, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int nb_body_left = NB_BODY - nb_body * (world_size - 1);
        nb_body = nb_body_left; // root manage remaining bodies
    } else {
        // Réception du nombre de points à calculer
        MPI_Recv(&nb_body, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    std::cout << "task n° " << current_rank << "manage " << nb_body << "bodies" << std::endl;



    // create nb_body bodies
    for(int i=0; i<nb_body; ++i){
        Vec2 position = Vec2(randMinmax(0, 10e30), randMinmax(0, 10e30));
        Vec2 velocity = Vec2(randMinmax(0, 10e4), randMinmax(0, 10e4));
        NBodyType mass = randMinmax(10e10, 10e30);

        Body *body = new Body(position, velocity, mass, 0);

        body->debug();
    }



    sleep(10);


    /* Main loop */
    // while (true)
    // {
    //     auto end = std::chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> elapsed_time = end - last;

    //     if (elapsed_time.count() >= TIME_STEP) // refresh rate
    //     {
    //         /*
    //             Update bodies position and velocity
    //             - each body send his mass and position to other bodies in order to compute attractive forces
    //             - so MPI communication : MPI_Allgather
    //             - we store data sent and data received in a vector
    //         */
        
    //         std::vector<NBodyType> data;
    //         data.resize(DATA_SIZE);
    //         Vec2 pos = body->getPosition();
    //         data = { body->getId(), body->getMass(), pos.x, pos.y};

    //         std::vector<NBodyType> received_data;
    //         received_data.resize(DATA_SIZE * world_size);
            

    //         MPI_Allgather(data.data(), DATA_SIZE, MPI_DOUBLE, received_data.data(), DATA_SIZE, MPI_DOUBLE, MPI_COMM_WORLD);

    //         Vec2 forces_on_body(0,0);

    //         /* For each data body received (id, mass and position), compute forces applied on the body */
    //         for(int i=0; i<received_data.size(); i+=DATA_SIZE){
    //             NBodyType id = received_data[i];

    //             if(id == body->getId())     // the body also receive his data, no computation in this case
    //                 continue;

    //             NBodyType mass_other = received_data[i+1];
    //             Vec2 position_other (received_data[i+2], received_data[i+3]);
    //             forces_on_body += body->computeForces(mass_other, position_other);
    //         }

    //         forces_on_body *= -GRAVITY_CONSTANT * body->getMass();
    //         /* We now have force applied on the body, we can compute the new position and velocity of the body */
    //         body->setForces(forces_on_body);
    //         //std::cout << "forces on particules : " << forces_on_body.x << " - " << forces_on_body.y << std::endl;
    //         //std::cout << "forces on particules of " << current_rank << " : " << forces_on_body.x << " - " << forces_on_body.y << std::endl;
            
    //         body->computePosition((NBodyType)elapsed_time.count());
    //         body->computeVelocity((NBodyType)elapsed_time.count());



    //         //body->debug();


    //         last = end;
    //         std::chrono::duration<double> elapsed_seconds = end - start;
    //         //std::cout << "elapsed time from start : " << elapsed_seconds.count() << "s" << std::endl;
    //     }
    // }
    

    MPI_Finalize();
    return 0;
}