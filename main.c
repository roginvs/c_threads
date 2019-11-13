#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

void *worker(int id)
{
    printf("Thread id=%i started\n", id);
    int randomTime = rand() % 3;
    usleep(100 + 100 * randomTime);
    printf("Thread id=%i finished\n", id);
    return NULL;
}

int main(int argc, char *argv[])
{
    int threads_count = 4;
    pthread_t *t = (pthread_t *)malloc(sizeof(pthread_t) * threads_count);
    printf("Hello world\n");

    for (int i = 0; i < threads_count; i++)
    {
        pthread_create(&t[i], NULL, &worker, i);
    };

    printf("Created\n");

    for (int i = 0; i < threads_count; i++)
    {
        pthread_join(t[i], NULL);
    }

    printf("Done\n");

    free(t);
}