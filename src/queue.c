#include "../includes/lem_ipc.h"

int init_message_queue(const char team)
{
    key_t key = ftok("Makefile", team);
    if (key == -1)
        exit_error("Message queue key", CLEANUP);

    int message_queue_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0666);
    if (message_queue_id == -1 && errno == EEXIST)
        message_queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (message_queue_id == -1)
        exit_error("Message queue create", CLEANUP);

    return message_queue_id;
}

// void enqueue(const char *message, const size_t len)
// {
//     // t_msg
//     if (msgsnd(state.message_queue_id, (void *)message, len, 0) == -1) // IPC_NOWAIT - non-blocking
//         perror("Enqueue message queue");
// }

// void dequeue()
// {
//     if (msgrcv(state.message_queue_id, (void *)b, 32, 0, 0) == -1)
//         perror("Dequeue message queue");
// }

// t_msg *a = malloc(sizeof(t_msg));
// a->mtype = 1;
// a->mtext[0] = 'A';
// a->mtext[1] = 'B';
// a->mtext[2] = '\0';

// t_msg *b = malloc(sizeof(t_msg));
// b->mtype = 1;
// b->mtext[0] = 'C';
// b->mtext[1] = '\0';

// if (msgrcv(message_queue_id, (void *)b, 32, 0, IPC_NOWAIT) == -1)
//     exit_with_error("Could not receiver message from the queue");

// printf("b after: %s\n", b->mtext);

// if (msgctl(message_queue_id, IPC_RMID, 0) == -1)
//     exit_with_error("Could not delete message queue");
