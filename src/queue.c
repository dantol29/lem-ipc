#include "../includes/lem_ipc.h"

int init_message_queue(const t_state *state, const char team)
{
    key_t key = ftok("Makefile", team);
    if (key == -1)
        exit_error(state, "Message queue key", CLEANUP);

    int message_queue_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (message_queue_id == -1 && errno == EEXIST)
        message_queue_id = msgget(key, IPC_CREAT | 0666);
    if (message_queue_id == -1)
        exit_error(state, "Message queue create", CLEANUP);

    return message_queue_id;
}

void clean_all_msgq()
{
    int message_queue_id;

    int i = 0;
    while (i < 5)
    {
        key_t key = ftok("Makefile", ++i);
        if (key != -1)
        {
            message_queue_id = msgget(key, IPC_CREAT | 0666);
            if (msgctl(message_queue_id, IPC_RMID, 0) == -1)
                perror("Could not delete message queue");
        }
    }
}

void enqueue(const int enemy_id, const int queue_id)
{
    t_msg msg;
    msg.type = 1;
    msg.enemy_id = enemy_id;

    if (msgsnd(queue_id, (void *)&msg, sizeof(msg.enemy_id), IPC_NOWAIT) == -1) // IPC_NOWAIT - non-blocking
        perror("Enqueue message queue");
}

int dequeue(const t_msg *msg, const int queue_id)
{
    if (msgrcv(queue_id, (void *)msg, sizeof(msg->enemy_id), 0, IPC_NOWAIT) == -1)
    {
        if (errno == ENOMSG)
            return 0;

        perror("Dequeue message queue");
        return -1;
    }

    return 1;
}
