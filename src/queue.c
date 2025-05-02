// int value = semctl(state.semaphores, 0, GETVAL);
//     if (value == -1)
//         exit_with_error("Could not get semaphore value");

//     printf("Value: %d\n", value);

// t_msg *a = malloc(sizeof(t_msg));
// a->mtype = 1;
// a->mtext[0] = 'A';
// a->mtext[1] = 'B';
// a->mtext[2] = '\0';

// t_msg *b = malloc(sizeof(t_msg));
// b->mtype = 1;
// b->mtext[0] = 'C';
// b->mtext[1] = '\0';
// int message_queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
// if (message_queue_id == -1)
//     exit_with_error("Could not create message queue");

// if (msgsnd(message_queue_id, (void *)a, 32, IPC_NOWAIT) == -1)
//     exit_with_error("Could not send message queue");

// printf("b before: %s\n", b->mtext);

// if (msgrcv(message_queue_id, (void *)b, 32, 0, IPC_NOWAIT) == -1)
//     exit_with_error("Could not receiver message from the queue");

// printf("b after: %s\n", b->mtext);

// if (msgctl(message_queue_id, IPC_RMID, 0) == -1)
//     exit_with_error("Could not delete message queue");
