#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/*

This is a test of pthreads broadcast

gcc -o test_broadcast.out test_broadcast.c -lpthread && ./test_broadcast.out

*/

struct WorkerInfo
{
    int id;
};

pthread_mutex_t m_test;
pthread_cond_t cond_test;

void *worker(void *params)
{
    struct WorkerInfo *info = (struct WorkerInfo *)params;

    printf("Thread id=%i started\n", info->id);
    pthread_mutex_lock(&m_test);
    printf("Thread id=%i locked mutex. Now unlocking and waiting\n", info->id);
    pthread_cond_wait(&cond_test, &m_test);
    printf("Thread id=%i Got signal\n", info->id);

    int randomTime = rand() % 3;
    usleep(1000 + 1000 * randomTime);
    printf("Thread id=%i Done, unlocking\n", info->id);
    pthread_mutex_unlock(&m_test);

    free(params);
    return NULL;
}

int main(int argc, char *argv[])
{

    pthread_mutex_init(&m_test, NULL);
    pthread_cond_init(&cond_test, NULL);

    pthread_mutex_lock(&m_test);

    printf("Main: start\n");

    int threads_count = 4;
    pthread_t *t = (pthread_t *)malloc(sizeof(pthread_t) * threads_count);

    for (int i = 0; i < threads_count; i++)
    {
        struct WorkerInfo *params = malloc(sizeof(struct WorkerInfo));
        params->id = i;

        pthread_create(&t[i], NULL, &worker, (void *)params);
    };

    printf("Main: thread created, unlocking mutex\n");
    pthread_mutex_unlock(&m_test);
    usleep(1200);
    printf("Main: sending broadcast\n");
    pthread_cond_broadcast(&cond_test);

    printf("Main: done, joining threads\n");
    void *ret = NULL;
    for (int i = 0; i < threads_count; i++)
    {
        pthread_join(t[i], ret);
        printf("Main: thread %i joined\n", i);
    }

    printf("Main: all threads joined\n");
    pthread_mutex_destroy(&m_test);

    pthread_cond_destroy(&cond_test);

    free(t);
    printf("Done\n");
}