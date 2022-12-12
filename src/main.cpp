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

    /*
        Compute number of bodies for each process
        - nb_body is the number of bodies for each process
        - nb_body_left is the number of bodies left if NB_BODY_TOTAL % world_size != 0
        We add nb_body_left to the first process (HOST_RANK)
    */

    int nb_body;
    int nb_body_left;

    if (current_rank == HOST_RANK)
        nb_body = floor(NB_BODY_TOTAL / world_size);

    // broadcast nb_body to all processes
    MPI_Bcast(&nb_body, 1, MPI_INT, HOST_RANK, MPI_COMM_WORLD);

    /* Only add nb_body_left to host rank if there are remainders bodies */
    nb_body_left = NB_BODY_TOTAL - (nb_body * world_size);
    if (current_rank == HOST_RANK && world_size > 1)
        nb_body += nb_body_left;

    /* buffer for recvcounts and displs (used in MPI_Allgatherv)
        - recvcounts is the number of elements to receive from each process
        - displs is the displacement relative to recvbuf at which to place the incoming data from each process
    */

    int *recvcounts = nullptr;
    recvcounts = (int *)malloc(world_size * sizeof(int));
    int *displs = nullptr;
    displs = (int *)malloc(world_size * sizeof(int));

    // only the host rank compute recvcounts and displs and then broadcast them to all processes
    if (current_rank == HOST_RANK)
    {
        recvcounts[0] = nb_body * SENDED_DATA_SIZE;
        displs[0] = 0;
        for (int i = 1; i < world_size; i++)
        {
            recvcounts[i] = (nb_body - nb_body_left) * SENDED_DATA_SIZE;
            displs[i] = (nb_body + ((nb_body - nb_body_left) * (i - 1))) * SENDED_DATA_SIZE;
        }
    }
    MPI_Bcast(&recvcounts[0], world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(&displs[0], world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);

    /*
        Create ids and masses, then broadcast them to all processes


    */

    /*
        Create velocities and positions buffers
        - velocities is the velocity of each body of the current process
        - positions is the position of each body of the current process
        - received_positions is the positions received from all processes (used in MPI_Allgatherv)
        - tmp_positions is a temporary buffer used to get positions from file
        - ids is the id of each body
        - masses is the mass of each body
    */
    double *velocities = nullptr;
    velocities = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));
    memset(velocities, 0, SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *positions = nullptr;
    positions = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *received_positions = nullptr;
    received_positions = (double *)malloc(SENDED_DATA_SIZE * NB_BODY_TOTAL * sizeof(double));

    double *tmp_positions = nullptr;
    tmp_positions = (double *)malloc(NB_BODY_TOTAL * 2 * sizeof(double));

    int *ids = nullptr;
    ids = (int *)malloc(NB_BODY_TOTAL * sizeof(int));

    double *masses = nullptr;
    masses = (double *)malloc(NB_BODY_TOTAL * sizeof(double));

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

        for (int i = 0; i < NB_BODY_TOTAL; i++)
        {
            fscanf(file, "%d %lf %lf %lf",
                   &ids[i],
                   &masses[i],
                   &tmp_positions[position_count + POSITION_X_INDEX],
                   &tmp_positions[position_count + POSITION_Y_INDEX]);

            position_count += 2;
        }

        fclose(file);
    }

    /*
        Scatterv allow to scatter data from one process to all processes also if they have different number of elements
        Scatter data from host to all processes
    */
    MPI_Scatterv(tmp_positions,
                 recvcounts,
                 displs,
                 MPI_DOUBLE,
                 positions,
                 recvcounts[current_rank],
                 MPI_DOUBLE,
                 HOST_RANK,
                 MPI_COMM_WORLD);

    // Broadcast ids and masses to all processes
    MPI_Bcast(&ids[0], NB_BODY_TOTAL, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(&masses[0], NB_BODY_TOTAL, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);

    // Start time for perf measurements
    double start_time = MPI_Wtime();

    /* Main loop */
    for (int i = 0; i < NB_ITERATIONS; i++)
    {

        /*
            Allgatherv allow to gather data from all processes also if they have different number of elements
        */
        MPI_Allgatherv(
            &positions[0],              // send buffer
            SENDED_DATA_SIZE * nb_body, // number of elements to send
            MPI_DOUBLE,                 // type of elements to send
            &received_positions[0],     // receive buffer
            &recvcounts[0],             // number of elements to receive from each process
            &displs[0],                 // displacement relative to recvbuf at which to place the incoming data from each process
            MPI_DOUBLE,                 // type of elements to receive
            MPI_COMM_WORLD);            // communicator

        for (int j = 0; j < nb_body; j++)
        {
            double forces_on_body[2] = {0, 0};

            int id_current = ids[current_rank * nb_body + j];         // get current body id
            double mass_current = masses[current_rank * nb_body + j]; // get current body mass

            /* We have to loop over all bodies to can compute the forces applied to the current body */
            for (int k = 0; k < SENDED_DATA_SIZE * NB_BODY_TOTAL; k += SENDED_DATA_SIZE)
            {
                // get corresponding id of the other body
                int id_other = ids[k / SENDED_DATA_SIZE];

                // if the other body is the current body, we skip it
                if (id_other == id_current)
                    continue;

                // get mass and position of the other body
                double mass_other = masses[k];
                double position_other[2] = {received_positions[k + POSITION_X_INDEX], received_positions[k + POSITION_Y_INDEX]};

                // compute forces applied on the body
                double position_current[2] = {positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                              positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX]};

                computeForces(position_current, position_other, mass_current, mass_other, &forces_on_body[0]);
            }

            forces_on_body[0] *= -GRAVITATIONAL_CONSTANT * mass_current;
            forces_on_body[1] *= -GRAVITATIONAL_CONSTANT * mass_current;

            /* compute the new position and velocity of the body */
            double acceleration[2] = {(forces_on_body[0] * DELTA_T) / mass_current, (forces_on_body[1] * DELTA_T) / mass_current};

            double velocity[2] = {velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] + acceleration[0],
                                  velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] + acceleration[1] * DELTA_T};

            double position[2] = {positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] + velocity[0] * DELTA_T,
                                  positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] + velocity[1] * DELTA_T};

            /* update the body */
            velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] = velocity[0];
            velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] = velocity[1];
            positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] = position[0];
            positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] = position[1];
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

        for (int i = 0; i < NB_BODY_TOTAL; i++)
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
    free(tmp_positions);

    MPI_Finalize();
    return 0;
}