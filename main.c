#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/*

gcc -o main.out main.c -lpthread && ./main.out

*/

struct WorkerInfo
{
    int id;
    int total_blocks_count;

    int *current_free_index;
    pthread_mutex_t *m_current_free_index;

    pthread_mutex_t *m_worker_is_allowed_to_write;
    pthread_cond_t *cond_worker_is_allowed_to_write;
    int *worker_is_allowed_to_write;

    // TODO: Write callback
};

void *worker(void *params)
{
    struct WorkerInfo *info = (struct WorkerInfo *)params;

    printf("Thread id=%i started\n", info->id);

    while (1)
    {
        int chunk_id = -1;
        pthread_mutex_lock(info->m_current_free_index);
        if (*info->current_free_index < info->total_blocks_count)
        {
            chunk_id = *info->current_free_index;
            *info->current_free_index += 1;
        };
        pthread_mutex_unlock(info->m_current_free_index);
        if (chunk_id == -1)
        {
            printf("Thread id=%i finished\n", info->id);
            return NULL;
        };

        printf("Thread id=%i picked up chunk=%i\n", info->id, chunk_id);

        int randomTime = rand() % 3;
        usleep(100 + 100 * randomTime);

        printf("Thread id=%i done chunk=%i\n", info->id, chunk_id);
        pthread_mutex_lock(info->m_worker_is_allowed_to_write);
        while (*info->worker_is_allowed_to_write != chunk_id)
        {
            pthread_cond_wait(info->cond_worker_is_allowed_to_write, info->m_worker_is_allowed_to_write);
        };
        printf("Thread id=%i writing to output chunk=%i\n", info->id, chunk_id);
        usleep(100 + 33 * randomTime);
        *info->worker_is_allowed_to_write += 1;
        pthread_mutex_unlock(info->m_worker_is_allowed_to_write);
        pthread_cond_broadcast(info->cond_worker_is_allowed_to_write);
    };
}

int main(int argc, char *argv[])
{
    int threads_count = 4;

    int current_free_index = 0;

    int worker_is_allowed_to_write = 0;

    int total_blocks_count = 10;

    pthread_mutex_t m_current_free_index;
    pthread_mutex_t m_worker_is_allowed_to_write;

    pthread_mutex_init(&m_current_free_index, NULL);
    pthread_mutex_init(&m_worker_is_allowed_to_write, NULL);

    pthread_cond_t cond_worker_is_allowed_to_write;
    pthread_cond_init(&cond_worker_is_allowed_to_write, NULL);

    pthread_mutex_lock(&m_current_free_index);
    pthread_mutex_lock(&m_worker_is_allowed_to_write);

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * threads_count);
    struct WorkerInfo *threads_params = (struct WorkerInfo *)malloc(sizeof(struct WorkerInfo) * threads_count);

    printf("Creating threads\n");
    for (int i = 0; i < threads_count; i++)
    {
        struct WorkerInfo *params = &threads_params[i];
        params->id = i;
        params->current_free_index = &current_free_index;
        params->m_current_free_index = &m_current_free_index;
        params->total_blocks_count = total_blocks_count;
        params->worker_is_allowed_to_write = &worker_is_allowed_to_write;
        params->m_worker_is_allowed_to_write = &m_worker_is_allowed_to_write;
        params->cond_worker_is_allowed_to_write = &cond_worker_is_allowed_to_write;
        pthread_create(&threads[i], NULL, &worker, (void *)params);
    };

    printf("Threads created, workers will pickup tasks\n");
    pthread_mutex_unlock(&m_current_free_index);

    // TODO: Do we need this part?
    printf("Now allowing threads to write\n");
    pthread_mutex_unlock(&m_worker_is_allowed_to_write);
    pthread_cond_broadcast(&cond_worker_is_allowed_to_write);

    printf("Now waiting for all blocks to finish\n");
    pthread_mutex_lock(&m_worker_is_allowed_to_write);
    while (worker_is_allowed_to_write < total_blocks_count)
    {
        pthread_cond_wait(&cond_worker_is_allowed_to_write, &m_worker_is_allowed_to_write);
    }
    pthread_mutex_unlock(&m_worker_is_allowed_to_write);

    printf("Now joining threads\n");
    for (int i = 0; i < threads_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(threads_params);

    pthread_mutex_destroy(&m_current_free_index);
    pthread_mutex_destroy(&m_worker_is_allowed_to_write);

    pthread_cond_destroy(&cond_worker_is_allowed_to_write);
    printf("Done\n");
}