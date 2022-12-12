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

    /*
        Computation of number of bodies for each process
        We distribute bodies as evenly as possible
        - nb_body => number of bodies for each process
        - nb_body_max => highest number of bodies for a process
        - nb_body_left => number of bodies left after distributing bodies as evenly as possible
    */
    int nb_body = floor(NB_BODY_TOTAL / world_size);

    int nb_body_max;

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

    /*
        - velocities => contains the velocities of all bodies for current process
        - local_forces => contains the forces of all bodies for current process
        - tmp_forces => contains the forces of all bodies for the process that is currently computing forces
        - masses => contains the masses of all bodies
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

    double *masses = nullptr;
    masses = (double *)malloc(NB_BODY_TOTAL * sizeof(double));

    // masses are calculated only on host process and then sent to all processes
    if (current_rank == HOST_RANK)
    {
        for (int i = 0; i < NB_BODY_TOTAL; i++)
        {
            masses[i] = randMinmax(10e2, 10e5);
        }
    }
    MPI_Bcast(&masses[0], NB_BODY_TOTAL, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);

    /* allocate memory for all arrays
        - local_positions => contains the positions of all bodies for current process
        - tmp_positions => contains the positions of all bodies for the process that is currently computing forces
        - local_ids => contains the ids of all bodies for current process
        - tmp_ids => contains the ids of all bodies for the process that is currently computing forces
    */
    double *local_positions = nullptr;
    local_positions = (double *)malloc(nb_body * SENDED_DATA_SIZE * sizeof(double));

    double *tmp_positions = nullptr;
    tmp_positions = (double *)malloc(nb_body_max * SENDED_DATA_SIZE * sizeof(double));

    for (int i = 0; i < nb_body * SENDED_DATA_SIZE; i += SENDED_DATA_SIZE)
    {
        double x = randMinmax(0, 100);
        double y = randMinmax(0, 100);

        local_positions[i + POSITION_X_INDEX] = x;
        local_positions[i + POSITION_Y_INDEX] = y;
        tmp_positions[i + POSITION_X_INDEX] = x;
        tmp_positions[i + POSITION_Y_INDEX] = y;
    }

    int *local_ids = nullptr;
    local_ids = (int *)malloc(nb_body * sizeof(int));

    int *tmp_ids = nullptr;
    tmp_ids = (int *)malloc(nb_body_max * sizeof(int));

    for (int i = 0; i < nb_body; ++i)
    {
        local_ids[i] = current_rank + (i * world_size);
        tmp_ids[i] = current_rank + (i * world_size);
    }

    double start_time = MPI_Wtime();

    /* Main loop */
    for (int i = 0; i < NB_ITERATIONS; i++)
    {

        /* Iterate through all process */
        for (int j = 0; j < world_size; j++)
        {

            /* Compute local_forces using tmp_positions and tmp_ids for each process's body */
            for (int k = 0; k < nb_body; ++k)
            {

                // get position, id and mass of current body
                double current_body_position[2] = {local_positions[k * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                                   local_positions[k * SENDED_DATA_SIZE + POSITION_Y_INDEX]};

                int current_body_id = local_ids[k];
                double current_body_mass = masses[current_body_id];

                for (int l = 0; l < nb_body_max; ++l)
                {
                    // get id of other body
                    int other_body_id = tmp_ids[l];

                    // if other body is the same as current body, skip (no need to compute force on itself)
                    // if other body id is lower than current body id, skip too (the other process will compute the force and it back through the ring communication)
                    if (current_body_id <= other_body_id)
                        continue;

                    // get position and mass of other body
                    double other_body_position[2] = {tmp_positions[l * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                                     tmp_positions[l * SENDED_DATA_SIZE + POSITION_Y_INDEX]};
                    double other_body_mass = masses[other_body_id];

                    // compute force
                    double forces[2];
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
            // send ids, positions and forces to next process
            MPI_Sendrecv_replace(tmp_ids, nb_body_max, MPI_INT, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(tmp_positions, nb_body_max * SENDED_DATA_SIZE, MPI_DOUBLE, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(tmp_forces, nb_body_max * SENDED_DATA_SIZE, MPI_DOUBLE, dest_proc, 0, src_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // last compute forces => local_forces + tmp_forces
        for (int j = 0; j < nb_body; ++j)
        {
            local_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX] += tmp_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX];
            local_forces[j * SENDED_DATA_SIZE + FORCE_Y_INDEX] += tmp_forces[j * SENDED_DATA_SIZE + FORCE_Y_INDEX];

            double mass_current = masses[local_ids[j]];
            local_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX] *= -GRAVITATIONAL_CONSTANT * mass_current;
            local_forces[j * SENDED_DATA_SIZE + FORCE_X_INDEX] *= -GRAVITATIONAL_CONSTANT * mass_current;

            /* compute the new position and velocity of the body */
            double acceleration[2] = {(local_forces[0] * DELTA_T) / mass_current, (local_forces[1] * DELTA_T) / mass_current};

            double velocity[2] = {velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] + acceleration[0],
                                  velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] + acceleration[1] * DELTA_T};

            double position[2] = {local_positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] + velocity[0] * DELTA_T,
                                  local_positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] + velocity[1] * DELTA_T};

            /* update the body positions and velocities */
            velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] = velocity[0];
            velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] = velocity[1];
            local_positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] = position[0];
            local_positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] = position[1];
        }

        // reset forces
        memset(local_forces, 0, SENDED_DATA_SIZE * nb_body * sizeof(double));
        memset(tmp_forces, 0, SENDED_DATA_SIZE * nb_body_max * sizeof(double));
    }

    double end_time = MPI_Wtime();
    if (current_rank == HOST_RANK)
        printf("Time: %f\n", end_time - start_time);

    // free memory
    free(masses);
    free(velocities);
    free(local_forces);
    free(local_positions);
    free(tmp_forces);
    free(tmp_positions);

    MPI_Finalize();
    return 0;
}