#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

struct WorkerInfo
{
    int id;
    int *currentFreeIndex;
    int *currentReadyIndex;
    int totalLength;
    pthread_mutex_t *m_input;
    pthread_mutex_t *m_output;
};

void *worker(void *params)
{
    struct WorkerInfo *info = (struct WorkerInfo *)params;

    printf("Thread id=%i started\n", info->id);

    while (1)
    {
        int chunk_id = -1;
        pthread_mutex_lock(info->m_input);
        if (*info->currentFreeIndex < info->totalLength)
        {
            chunk_id = *info->currentFreeIndex;
            *info->currentFreeIndex += 1;
        };
        pthread_mutex_unlock(info->m_input);
        if (chunk_id == -1)
        {
            printf("Thread id=%i finished\n", info->id);
            free(params);
            return NULL;
        };

        printf("Thread id=%i picked up chunk=%i\n", info->id, chunk_id);

        int randomTime = rand() % 3;
        usleep(100 + 100 * randomTime);
    };

    return NULL;
}

int main(int argc, char *argv[])
{
    int threads_count = 4;

    int currentFreeIndex = 0;
    int currentReadyIndex = 0;

    int totalLength = 10;

    pthread_mutex_t m_input;
    pthread_mutex_t m_output;

    pthread_mutex_init(&m_input, NULL);
    pthread_mutex_init(&m_output, NULL);

    pthread_mutex_lock(&m_input);

    pthread_t *t = (pthread_t *)malloc(sizeof(pthread_t) * threads_count);
    printf("Hello world\n");

    for (int i = 0; i < threads_count; i++)
    {
        struct WorkerInfo *params = malloc(sizeof(struct WorkerInfo));
        params->id = i;
        params->currentFreeIndex = &currentFreeIndex;
        params->currentReadyIndex = &currentReadyIndex;
        params->totalLength = totalLength;
        params->m_input = &m_input;
        params->m_output = &m_output;
        pthread_create(&t[i], NULL, &worker, (void *)params);
    };

    printf("Threads created, workers will pickup tasks\n");
    pthread_mutex_unlock(&m_input);

    for (int i = 0; i < threads_count; i++)
    {
        pthread_join(t[i], NULL);
    }

    pthread_mutex_destroy(&m_input);
    pthread_mutex_destroy(&m_output);

    printf("Done\n");

    free(t);
}