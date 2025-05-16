#ifndef LEM_IPC_H
#define LEM_IPC_H

#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib_mlx42/include/MLX42/MLX42.h"

#define FIELD_WIDTH 28
#define FIELD_HEIGHT 20
#define TEAM_SIZE 4
#define TEAM_COUNT 4
#define QUEUE_KEY 29
#define FIELD_SIZE FIELD_WIDTH *FIELD_HEIGHT
#define WALL 100
#define PLAYER_DELAY 300000

enum exit_modes
{
    DEFAULT,
    CLEANUP
};

enum enemy_pos
{
    NOT_FOUND = -2,
    ENEMY_CLOSE = -1
};

typedef struct s_field
{
    const void *field;
    char team;
    int player_x;
    int player_y;
    int player_pos;
    int player_id;
} t_field;

typedef struct s_node
{
    short x;
    short y;
    short f;
    short g;
    struct s_node *parent;
    struct s_node *next;
} t_node;

typedef struct s_msg
{
    long type;
    int enemy_id;
} t_msg;

typedef struct s_state
{
    int shared_memory_id;
    int message_queue_id;
    int semaphores_id;
    char team;
} t_state;

// semaphore.c
int init_semaphore(const t_state *state);
void update_semaphore(const unsigned short sem_num, const short value, const int sem_id);

// queue.c
int init_message_queue(const t_state *state, const char team);
void clean_all_msgq();
void enqueue(const int enemy_id, const int queue_id);
int dequeue(const t_msg *msg, const int queue_id);

// player.c
void player_loop(const t_state *state, const void *shared_memory, t_field *info);
int place_player(const t_state *state, const char team, const void *shared_memory);

// drawer.c
int display_shared_memory(const t_state *state, const void *shared_memory);

// a_star.c
t_node *a_star(const t_field *info, const int enemy_pos);

// utils.c
void cleanup_resources(const t_state *state);
char parse_argv(char **argv);
int get_player_pos(const t_field *info, const char player_id);
_Noreturn void exit_error(const t_state *state, const char *message, const int cleanup);

#endif
