#include "../includes/lem_ipc.h"

t_state state = {0};

static void *init_shared_memory()
{
    key_t key = ftok("Makefile", 'm');
    if (key == -1)
        exit_error("Shared memory key", DEFAULT);

    state.shared_memory_id = shmget(key, FIELD_WIDTH * FIELD_HEIGHT, IPC_CREAT | IPC_EXCL | 0666); // shared memory has to be at least page size
    if (state.shared_memory_id == -1 && errno == EEXIST)
        state.shared_memory_id = shmget(key, FIELD_WIDTH * FIELD_HEIGHT, IPC_CREAT | 0666);
    if (state.shared_memory_id == -1)
        exit_error("Shared memory create", DEFAULT);

    void *shared_memory = shmat(state.shared_memory_id, 0, 0); // attach shared memory to the page table of the process
    if (shared_memory == (void *)-1)
        exit_error("Shared memory map", CLEANUP);

    if (errno != EEXIST)
        memset(shared_memory, 0, FIELD_WIDTH * FIELD_HEIGHT + sizeof(size_t) * 2);

    return shared_memory;
}

int main(int argc, char **argv)
{
    const char team = parse_argc(argc, argv);

    void *shared_memory = init_shared_memory();
    state.semaphores_id = init_semaphore();
    state.message_queue_id = init_message_queue(team);

    if (team == '0')
        return display_shared_memory(shared_memory); // drawer process

    int player_pos = place_player(team, shared_memory);
    if (player_pos != -1)
    {
        t_field info;
        info.field = (size_t *)shared_memory + 2;
        info.team = team;
        info.player_pos = player_pos;
        info.player_id = *((char *)info.field + info.player_pos);

        player_loop(shared_memory, &info);
    }

    if (*(size_t *)shared_memory == 0) // no players left
        cleanup_resources();

    return 0;
}

