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

extern t_state state;

// semaphore.c
int init_semaphore();
void update_semaphore(const unsigned short sem_num, const short value);

// queue.c
int init_message_queue(const char team);
void enqueue(const int enemy_id);
int dequeue(t_msg *msg);

// player.c
void player_loop(const void *shared_memory, t_field *info);
int place_player(const char team, const void *shared_memory);

// drawer.c
int display_shared_memory(const void *shared_memory);

// a_star.c
t_node *a_star(t_field *info, int enemy_pos);

// utils.c
void cleanup_resources();
char parse_argc(const int argc, char **argv);
int get_player_pos(const t_field *info, const char player_id);
int move_from_enemy(int *player_x, int *player_y, const int enemy_pos);
_Noreturn void exit_error(const char *message, const int cleanup);

#endif
