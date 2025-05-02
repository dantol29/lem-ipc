#include "../includes/lem_ipc.h"

_Noreturn void exit_error(const char *message)
{
    perror(message);
    exit(1);
}

void cleanup_resources()
{
    write(1, "Cleaning resources...\n", 23);
    if (shmctl(state.shared_memory_id, IPC_RMID, 0) == -1)
        perror("Could not delete shared memory");

    if (semctl(state.semaphores_id, IPC_RMID, 0) == -1)
        perror("Could not delete semaphores");
}

char parse_argc(const int argc, char **argv)
{
    if (argc < 2)
        return '0'; // game drawer

    const int team = atoi(argv[1]);
    if (team == 0 || team > 255)
    {
        write(1, "Incorrect team number\n", 23);
        exit(1);
    };
    return team;
}
