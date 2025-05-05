#include "../includes/lem_ipc.h"

_Noreturn void exit_error(const char *message, const int cleanup)
{
    if (cleanup == CLEANUP)
        cleanup_resources();

    perror(message);
    exit(1);
}

void cleanup_resources()
{
    write(1, "Cleaning resources...\n", 23);

    if (msgctl(state.message_queue_id, IPC_RMID, 0) == -1)
        perror("Could not delete message queue");

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

int get_player_pos(const t_field *info, const char player_id)
{
    int i = 0;

    while (i < FIELD_WIDTH * FIELD_HEIGHT)
    {
        if (*((char *)info->field + i) == player_id)
            return i;
        ++i;
    }

    return -1;
}
