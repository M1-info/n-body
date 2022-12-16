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

    // set error handler to return error code instead of aborting
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);


    if(current_rank == HOST_RANK)
    {
        std::cout << "Starting symmetry N-body program" << std::endl;
    }

    /*
        Computation of number of bodies for each process
        We distribute bodies as evenly as possible
        - nb_body => number of bodies for each process
        - nb_body_max => highest number of bodies for a process
        - nb_body_left => number of bodies left after distributing bodies as evenly as possible
    */
    int nb_body = floor(NB_BODY_TOTAL / world_size);

    int nb_body_max = nb_body;

    int nb_body_left = NB_BODY_TOTAL - (nb_body * world_size);

    if (current_rank < nb_body_left)
    {
        nb_body++;
    }

    if (current_rank == HOST_RANK)
    {
        nb_body_max = nb_body;
    }

    // send the highest number of bodies to all processes
    // so that they can allocate the right amount of memory
    MPI_Bcast(&nb_body_max, 1, MPI_INT, HOST_RANK, MPI_COMM_WORLD);

    // create rcvcounts and displs arrays for scatterv
    int *recvcounts_positions = (int *)malloc(world_size * sizeof(int));
    int *displs_positions = (int *)malloc(world_size * sizeof(int));

    int *recvcounts_ids = (int *)malloc(world_size * sizeof(int));
    int *displs_ids = (int *)malloc(world_size * sizeof(int));

    if (current_rank == HOST_RANK)
    {
        recvcounts_positions[0] = nb_body_max * SENDED_DATA_SIZE;
        displs_positions[0] = 0;
        recvcounts_ids[0] = nb_body_max;
        displs_ids[0] = 0;

        for (int i = 1; i < world_size; i++)
        {
            bool has_body_left = i < nb_body_left;

            if (has_body_left || nb_body_left == 0)
            {
                recvcounts_positions[i] = nb_body_max * SENDED_DATA_SIZE;
                recvcounts_ids[i] = nb_body_max;
            }
            else
            {
                recvcounts_positions[i] = (nb_body_max - 1) * SENDED_DATA_SIZE;
                recvcounts_ids[i] = nb_body_max - 1;
            }

            displs_positions[i] = displs_positions[i - 1] + recvcounts_positions[i - 1];
            displs_ids[i] = displs_ids[i - 1] + recvcounts_ids[i - 1];
        }
    }

    // send rcvcounts and displs to all processes
    MPI_Bcast(recvcounts_positions, world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(displs_positions, world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(recvcounts_ids, world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(displs_ids, world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);

    /*
        - velocities => contains the velocities of all bodies for current process
        - local_forces => contains the forces of all bodies for current process
        - tmp_forces => contains the forces of all bodies for the process that is currently computing forces
        - local_positions => contains the positions of all bodies for current process
        - tmp_positions => contains the positions of all bodies for the process that is currently computing forces
        - local_ids => contains the ids of all bodies for current process
        - tmp_ids => contains the ids of all bodies for the process that is currently computing forces
        - local_masses => contains the masses of all bodies for current process
        - tmp_masses => contains the masses of all bodies for the process that is currently computing forces
        - file_positions => contains the positions of all bodies from input file
        - file_ids => contains the ids of all bodies from input file
        - file_masses => contains the masses of all bodies from input file
    */
    double *velocities = nullptr;
    velocities = (double *)malloc(nb_body * SENDED_DATA_SIZE * sizeof(double));
    memset(velocities, 0, SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *local_forces = nullptr;
    local_forces = (double *)malloc(nb_body * SENDED_DATA_SIZE * sizeof(double));
    memset(local_forces, 0, SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *tmp_forces = nullptr;
    tmp_forces = (double *)malloc(nb_body_max * SENDED_DATA_SIZE * sizeof(double));
    memset(tmp_forces, 0, SENDED_DATA_SIZE * nb_body_max * sizeof(double));

    double *local_positions = nullptr;
    local_positions = (double *)malloc(nb_body * SENDED_DATA_SIZE * sizeof(double));

    double *tmp_positions = nullptr;
    tmp_positions = (double *)malloc(nb_body_max * SENDED_DATA_SIZE * sizeof(double));

    int *local_ids = nullptr;
    local_ids = (int *)malloc(nb_body * sizeof(int));

    int *tmp_ids = nullptr;
    tmp_ids = (int *)malloc(nb_body_max * sizeof(int));

    double *local_masses = nullptr;
    local_masses = (double *)malloc(nb_body * sizeof(double));

    double *tmp_masses = nullptr;
    tmp_masses = (double *)malloc(nb_body_max * sizeof(double));

    double *file_positions = nullptr;
    file_positions = (double *)malloc(NB_BODY_TOTAL * SENDED_DATA_SIZE * sizeof(double));

    int *file_ids = nullptr;
    file_ids = (int *)malloc(NB_BODY_TOTAL * sizeof(int));

    double *file_masses = nullptr;
    file_masses = (double *)malloc(NB_BODY_TOTAL * sizeof(double));

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
            fscanf(file, "%i %lf %lf %lf",
                   &file_ids[i],
                   &file_masses[i],
                   &file_positions[position_count + POSITION_X_INDEX],
                   &file_positions[position_count + POSITION_Y_INDEX]);

            position_count += 2;
        }

        fclose(file);
    }

    MPI_Bcast(file_positions, NB_BODY_TOTAL * SENDED_DATA_SIZE, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(file_ids, NB_BODY_TOTAL, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(file_masses, NB_BODY_TOTAL, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);

    for (int i = 0; i < nb_body; i++)
    {
        local_ids[i] = file_ids[i * world_size + current_rank];
        tmp_ids[i] = local_ids[i];

        local_masses[i] = file_masses[i * world_size + current_rank];
        tmp_masses[i] = local_masses[i];

        int file_index_position = i * world_size * SENDED_DATA_SIZE + (current_rank * SENDED_DATA_SIZE);
        local_positions[i * SENDED_DATA_SIZE + POSITION_X_INDEX] = file_positions[file_index_position + POSITION_X_INDEX];
        local_positions[i * SENDED_DATA_SIZE + POSITION_Y_INDEX] = file_positions[file_index_position + POSITION_Y_INDEX];
        tmp_positions[i * SENDED_DATA_SIZE + POSITION_X_INDEX] = local_positions[i * SENDED_DATA_SIZE + POSITION_X_INDEX];
        tmp_positions[i * SENDED_DATA_SIZE + POSITION_Y_INDEX] = local_positions[i * SENDED_DATA_SIZE + POSITION_Y_INDEX];
    }

    double start_time = MPI_Wtime();

    /* Main loop */
    for (int i = 0; i < NB_ITERATIONS; i++)
    {

        /* Iterate through all process */
        for (int j = 0; j < world_size; j++)
        {

            /* Compute local_forces using tmp_positions and tmp_ids for each process's body */
            for (int k = 0; k < nb_body; k++)
            {

                // get position, id and mass of current body
                int current_body_id = local_ids[k];
                double current_body_mass = local_masses[k];
                double current_body_position[2] = {local_positions[k * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                                   local_positions[k * SENDED_DATA_SIZE + POSITION_Y_INDEX]};

                for (int l = 0; l < nb_body_max; l++)
                {

                    // get id of other body
                    int other_body_id = tmp_ids[l];

                    // if other body is the same as current body, skip (no need to compute force on itself)
                    // if other body id is lower than current body id, skip too (the other process will compute the force and it back through the ring communication)
                    if (current_body_id >= other_body_id)
                        continue;

                    // get position and mass of other body
                    double other_body_mass = tmp_masses[l];

                    double other_body_position[2] = {tmp_positions[l * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                                     tmp_positions[l * SENDED_DATA_SIZE + POSITION_Y_INDEX]};

                    // compute force
                    double forces[2] = {0, 0};
                    computeForces(current_body_position, other_body_position, current_body_mass, other_body_mass, &forces[0]);

                    // update local forces and tmp forces
                    local_forces[k * SENDED_DATA_SIZE + FORCE_X_INDEX] += forces[0];
                    local_forces[k * SENDED_DATA_SIZE + FORCE_Y_INDEX] += forces[1];

                    // tmps forces are updated with the opposite force (Newton's third law of motion)
                    tmp_forces[l * SENDED_DATA_SIZE + FORCE_X_INDEX] -= forces[0];
                    tmp_forces[l * SENDED_DATA_SIZE + FORCE_Y_INDEX] -= forces[1];
                }
            }

            /* Communication */

            // send tmp_ids, tmp_pos and tmp_forces to next process
            int dest_proc = (current_rank - 1 + world_size) % world_size;
            int src_proc = (current_rank + 1) % world_size;

            // sendrecv_replace => send and receive in the same buffer
            MPI_Sendrecv_replace(tmp_ids, nb_body_max, MPI_INT, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(tmp_positions, nb_body_max * SENDED_DATA_SIZE, MPI_DOUBLE, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(tmp_forces, nb_body_max * SENDED_DATA_SIZE, MPI_DOUBLE, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(tmp_masses, nb_body_max, MPI_DOUBLE, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // last compute forces => local_forces + tmp_forces
        for (int j = 0; j < nb_body; ++j)
        {
            local_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX] += tmp_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX];
            local_forces[j * SENDED_DATA_SIZE + FORCE_Y_INDEX] += tmp_forces[j * SENDED_DATA_SIZE + FORCE_Y_INDEX];

            double mass_current = local_masses[j];

            local_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX] *= -GRAVITATIONAL_CONSTANT * mass_current;
            local_forces[j * SENDED_DATA_SIZE + FORCE_Y_INDEX] *= -GRAVITATIONAL_CONSTANT * mass_current;

            /* compute the new position and velocity of the body */
            double acceleration[2] = {(local_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX] * DELTA_T) / mass_current,
                                      (local_forces[j * SENDED_DATA_SIZE + FORCE_Y_INDEX] * DELTA_T) / mass_current};

            double velocity[2] = {velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] + acceleration[0],
                                  velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] + acceleration[1]};

            double position[2] = {local_positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] + velocity[0] * DELTA_T,
                                  local_positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] + velocity[1] * DELTA_T};

            /* update the body positions and velocities */
            velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] = velocity[0];
            velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] = velocity[1];

            local_positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] = position[0];
            local_positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] = position[1];

            tmp_positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] = position[0];
            tmp_positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] = position[1];
        }

        // reset forces
        memset(local_forces, 0, SENDED_DATA_SIZE * nb_body * sizeof(double));
        memset(tmp_forces, 0, SENDED_DATA_SIZE * nb_body_max * sizeof(double));
    }

    double end_time = MPI_Wtime();
    if (current_rank == HOST_RANK)
        printf("Time: %f\n", end_time - start_time);

    int *output_ids = (int *)malloc(NB_BODY_TOTAL * sizeof(int));
    double *output_positions = (double *)malloc(NB_BODY_TOTAL * SENDED_DATA_SIZE * sizeof(double));
    double *output_masses = (double *)malloc(NB_BODY_TOTAL * sizeof(double));

    MPI_Gatherv(local_ids, nb_body, MPI_INT, output_ids, recvcounts_ids, displs_ids, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Gatherv(local_positions, nb_body * SENDED_DATA_SIZE, MPI_DOUBLE, output_positions, recvcounts_positions, displs_positions, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);
    MPI_Gatherv(local_masses, nb_body, MPI_DOUBLE, output_masses, recvcounts_ids, displs_ids, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);

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
            fprintf(file, "%d %lf %lf %lf \n", output_ids[i], output_masses[i], output_positions[position_count + POSITION_X_INDEX], output_positions[position_count + POSITION_Y_INDEX]);
            position_count += 2;
        }

        fclose(file);
    }

    // free memory
    free(velocities);
    free(local_forces);
    free(local_positions);
    free(tmp_forces);
    free(tmp_positions);
    free(local_ids);
    free(tmp_ids);
    free(local_masses);
    free(tmp_masses);
    free(file_ids);
    free(file_positions);
    free(file_masses);
    free(recvcounts_ids);
    free(displs_ids);
    free(recvcounts_positions);
    free(displs_positions);
    free(output_ids);
    free(output_positions);
    free(output_masses);

    MPI_Finalize();
    return 0;
}