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

    // number of bodies for each process
    int nb_body;

    // number of bodies left (if NB_BODY_TOTAL % world_size != 0)
    int nb_body_left = 0;

    // compute nb_body to work with for each process
    if (current_rank == HOST_RANK)
        nb_body = floor(NB_BODY_TOTAL / world_size);

    // broadcast nb_body to all processes
    MPI_Bcast(&nb_body, 1, MPI_INT, HOST_RANK, MPI_COMM_WORLD);



    // BUG: nb_body_left is not computed correctly, so :
    int real_nb_body_total = nb_body * world_size;


    // if (current_rank == HOST_RANK && world_size > 1){
    //     // compute nb_body_left
    //     nb_body_left = NB_BODY_TOTAL - (nb_body * world_size);
    //     nb_body += nb_body_left;
    // }


    // print nb_body for each process
    // std::cout << current_rank << " -> nb_body: " << nb_body << std::endl;
    // std::cout << current_rank << " -> nb_body_left: " << nb_body_left << std::endl;


    // buffer for recvcounts and displs (used in MPI_Allgatherv)
    int *recvcounts = nullptr;
    recvcounts = (int *)malloc(world_size * sizeof(int));

    int *displs = nullptr;
    displs = (int *)malloc(world_size * sizeof(int));

    if (current_rank == HOST_RANK)
    {
        recvcounts[0] = nb_body * SENDED_DATA_SIZE;
        displs[0] = 0;
        for (int i = 1; i < world_size; i++)
        {
            recvcounts[i] = (nb_body - nb_body_left) * SENDED_DATA_SIZE;
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }

    // broadcast recvcounts and displs to all processes
    MPI_Bcast(recvcounts, world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(displs, world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);


    /*
        Create velocities and positions buffers
        - velocities is the velocity of each body of the current process
        - positions is the position of each body of the current process
        - received_positions is the positions received from all processes (used in MPI_Allgatherv)
        - file_positions is a temporary buffer used to get positions from file
        - ids is the id of each body
        - masses is the mass of each body
    */

    // buffers for MPI communication
    int *ids = nullptr;
    ids = (int *)malloc(real_nb_body_total * sizeof(int));

    double *masses = nullptr;
    masses = (double *)malloc(real_nb_body_total * sizeof(double));

    double *file_positions = nullptr;
    file_positions = (double *)malloc(real_nb_body_total * SENDED_DATA_SIZE * sizeof(double));

    double *velocities = nullptr;
    velocities = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));
    memset(velocities, 0, SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *positions = nullptr;
    positions = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *received_positions = nullptr;
    received_positions = (double *)malloc(SENDED_DATA_SIZE * real_nb_body_total * sizeof(double));

    /* Host get data from file */
    if (current_rank == HOST_RANK)
    {
        FILE *file = fopen(INPUT_FILE, "r");

        int position_count = 0;

        if (file == nullptr)
        {
            std::cerr << "Error: cannot open file " << INPUT_FILE << std::endl;
            exit(1);
        }

        for (int i = 0; i < real_nb_body_total; i++)
        {
            fscanf(file, "%d %lf %lf %lf",
                   &ids[i],
                   &masses[i],
                   &file_positions[position_count + POSITION_X_INDEX],
                   &file_positions[position_count + POSITION_Y_INDEX]);

            position_count += 2;
        }

        fclose(file);
    }

    // scatter file_positions to all processes
    MPI_Scatterv(
        file_positions,
        recvcounts,
        displs,
        MPI_DOUBLE,
        positions,
        recvcounts[current_rank],
        MPI_DOUBLE,
        HOST_RANK,
        MPI_COMM_WORLD);


    // broadcast ids and masses to all processes
    MPI_Bcast(ids, real_nb_body_total, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(masses, real_nb_body_total, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);



    // Start time for perf measurements
    double start_time = MPI_Wtime();


    /* Main loop */
    for (int i = 0; i < NB_ITERATIONS; i++)
    {

        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Allgatherv(
            positions,
            nb_body * SENDED_DATA_SIZE,
            MPI_DOUBLE,
            received_positions,
            recvcounts,
            displs,
            MPI_DOUBLE,
            MPI_COMM_WORLD);

        // if(current_rank == HOST_RANK)
        //     for (int m = 0; m < real_nb_body_total; m++)
        //     {
        //         std::cout << "itr=" << i << " => " << received_positions[m * SENDED_DATA_SIZE + POSITION_X_INDEX] << " " << received_positions[m * SENDED_DATA_SIZE + POSITION_Y_INDEX] << std::endl;
        //     }


        /* For each positions body received (id, mass and position), compute forces applied on the body */
        for (int j = 0; j < nb_body; j++)
        {
            double forces_on_body[2] = {0, 0};

            int id_current = ids[current_rank * nb_body + j];         // get current body id
            double mass_current = masses[current_rank * nb_body + j]; // get current body mass
            double position_current[2] = {  received_positions[id_current * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                            received_positions[id_current * SENDED_DATA_SIZE + POSITION_Y_INDEX]  };

            for (int k = 0; k < real_nb_body_total; k++)
            {
                // get corresponding id of the other body
                int id_other = ids[k];

                // check if body is not the same
                if (id_other == id_current)
                    continue;

                // get mass and position of the other body
                double mass_other = masses[k];
                double position_other[2] = {    received_positions[id_other * SENDED_DATA_SIZE + POSITION_X_INDEX], 
                                                received_positions[id_other * SENDED_DATA_SIZE + POSITION_Y_INDEX] };

                // compute forces applied on the body
                computeForces(position_current, position_other, mass_current, mass_other, &forces_on_body[0]);
                //std::cout << "it:" << i << " => " <<current_rank << " - forces_on_body[" << id_current << " - " << id_other << "] = " << forces_on_body[0] << " " << forces_on_body[1] << std::endl;
            }

            //std::cout << current_rank << " - forces_on_body[" << j << "] = " << forces_on_body[0] << " " << forces_on_body[1] << std::endl;

            forces_on_body[0] *= -GRAVITATIONAL_CONSTANT * mass_current;
            forces_on_body[1] *= -GRAVITATIONAL_CONSTANT * mass_current;

            // print forces

            /* compute the new position and velocity of the body */
            double acceleration[2] = {(forces_on_body[0] * DELTA_T) / mass_current, (forces_on_body[1] * DELTA_T) / mass_current};

            // print acceleration
            //std::cout << current_rank << " - acceleration[" << j << "] = " << acceleration[0] << " " << acceleration[1] << std::endl;

            double velocity[2] = {velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] + acceleration[0],
                                  velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] + acceleration[1]};

            double position[2] = {position_current[POSITION_X_INDEX] + velocity[VELOCITY_X_INDEX] * DELTA_T,
                                  position_current[POSITION_Y_INDEX] + velocity[VELOCITY_Y_INDEX] * DELTA_T};

            // print velocity
            //std::cout << current_rank << " - velocity[" << j << "] = " << velocity[VELOCITY_X_INDEX] << " " << velocity[VELOCITY_Y_INDEX] << std::endl; 

            /* update the body */
            velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] = velocity[VELOCITY_X_INDEX];
            velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] = velocity[VELOCITY_Y_INDEX];
            positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] = position[POSITION_X_INDEX];
            positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] = position[POSITION_Y_INDEX];


            // print position
            //std::cout << current_rank << " - position[" << j << "] = " << position[POSITION_X_INDEX] << " " << position[POSITION_Y_INDEX] << std::endl;
        }
    }





    double end_time = MPI_Wtime();
    if (current_rank == HOST_RANK)
        printf("Time: %f\n", end_time - start_time);

    // write the result in a file
    if (current_rank == HOST_RANK)
    {
        FILE *file = fopen(OUTPUT_FILE, "w");

        if (file == nullptr)
        {
            std::cerr << "Error: cannot open file " << OUTPUT_FILE << std::endl;
            exit(1);
        }

        int position_count = 0;

        for (int i = 0; i < real_nb_body_total; i++)
        {
            fprintf(file, "%d %lf %lf %lf \n", ids[i], masses[i], received_positions[position_count + POSITION_X_INDEX], received_positions[position_count + POSITION_Y_INDEX]);
            position_count += 2;
        }

        fclose(file);
    }

    // free memory
    free(ids);
    free(masses);
    free(recvcounts);
    free(displs);
    free(velocities);
    free(positions);
    free(received_positions);



    MPI_Finalize();
    return 0;
}