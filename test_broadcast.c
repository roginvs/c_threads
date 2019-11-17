#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

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

        int randomTime = rand() % 3;
        usleep(100 + 100 * randomTime);

    return NULL;
}

int main(int argc, char *argv[])
{
    int threads_count = 4;

    int current_free_index = 0;
    int master_waiting_for_block_index = 0;

    struct WorkerResult current_result;

    int total_blocks_count = 10;

    pthread_mutex_t m_waiting_for_block_index;
    pthread_mutex_t m_worker_is_ready;

    pthread_mutex_init(&m_input, NULL);
    pthread_mutex_init(&m_waiting_for_block_index, NULL);
    pthread_mutex_init(&m_worker_is_ready, NULL);

    pthread_cond_t cond_waiting_for_block_changed;
    pthread_cond_t cond_worker_is_ready;
    pthread_cond_init(&cond_waiting_for_block_changed, NULL);
    pthread_cond_init(&cond_worker_is_ready, NULL);

    pthread_mutex_lock(&m_input);
    pthread_mutex_lock(&m_waiting_for_block_index);
    pthread_mutex_lock(&m_worker_is_ready);

    pthread_t *t = (pthread_t *)malloc(sizeof(pthread_t) * threads_count);
    printf("Hello world\n");

    for (int i = 0; i < threads_count; i++)
    {
        struct WorkerInfo *params = malloc(sizeof(struct WorkerInfo));
        params->id = i;
        params->current_free_index = &current_free_index;
        params->master_waiting_for_block_index = &master_waiting_for_block_index;
        params->total_blocks_count = total_blocks_count;
        params->m_input = &m_input;
        params->m_waiting_for_block_index = &m_waiting_for_block_index;
        params->m_worker_is_ready = &m_worker_is_ready;
        pthread_create(&t[i], NULL, &worker, (void *)params);
    };

    printf("Threads created, workers will pickup tasks\n");
    pthread_mutex_unlock(&m_input);

    while (1)
    {
        // This order to keep mutexes lock in the same order

        pthread_cond_wait(&cond_worker_is_ready, &m_worker_is_ready);
        printf("Master processing block %i\n", master_waiting_for_block_index);

        master_waiting_for_block_index++;
        if (master_waiting_for_block_index >= total_blocks_count)
        {
            break;
        }
    };

    pthread_mutex_unlock(&m_waiting_for_block_index);
    pthread_mutex_unlock(&m_worker_is_ready);

    for (int i = 0; i < threads_count; i++)
    {
        pthread_join(t[i], NULL);
    }

    pthread_mutex_destroy(&m_input);
    pthread_mutex_destroy(&m_waiting_for_block_index);
    pthread_mutex_destroy(&m_worker_is_ready);

    pthread_cond_destroy(&cond_waiting_for_block_changed);
    pthread_cond_destroy(&cond_worker_is_ready);
    printf("Done\n");

    free(t);
}