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

enum exit_modes
{
    DEFAULT,
    CLEANUP
};

typedef struct s_field
{
    const void *field;
    char team;
    int player_x;
    int player_y;
    int player_pos;
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
    long mtype;
    char mtext[32];
} t_msg;

typedef struct s_state
{
    int shared_memory_id;
    int message_queue_id;
    int semaphores_id;
} t_state;

extern t_state state;

int init_semaphore();
int init_message_queue(const char team);
void update_semaphore(const unsigned short sem_num, const short value);
void player_loop(const void *shared_memory, t_field *info);
int place_player(const char team, const void *shared_memory);
int display_shared_memory(const void *shared_memory);
void cleanup_resources();
char parse_argc(const int argc, char **argv);
t_node *a_star(t_field *info, int enemy_pos);
_Noreturn void exit_error(const char *message, const int cleanup);

#endif
