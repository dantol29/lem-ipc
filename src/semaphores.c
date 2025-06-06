#include "../includes/lem_ipc.h"

union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

int init_semaphore(const t_state *state)
{
    key_t key = ftok("Makefile", 's');
    if (key == -1)
        exit_error(state, "Semaphore key", CLEANUP);

    int semaphores = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semaphores == -1 && errno == EEXIST)
        semaphores = semget(key, 1, IPC_CREAT | 0666);
    if (semaphores == -1)
        exit_error(state, "Semaphore create", CLEANUP);

    if (errno != EEXIST)
    {
        union semun arg;
        arg.val = 1;

        if (semctl(semaphores, 0, SETVAL, arg) == -1)
            exit_error(state, "Semaphore set", CLEANUP);
    }

    return semaphores;
}

// Positive value increments the semaphore value by that amount.
// Negative value decrements the semaphore value by that amount
// Value of zero means to wait for the semaphore value to reach zero.
void update_semaphore(const unsigned short sem_num, const short value, const int sem_id)
{
    struct sembuf s;
    s.sem_num = sem_num;
    s.sem_op = value;
    s.sem_flg = 0; // IPC_NOWAIT - do not block

    if (semop(sem_id, &s, 1) == -1)
        write(1, "Could not change semaphore value\n", 34);
}
