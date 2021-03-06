#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include "./crc32.c"
#include "./gzip.chunk.c"

typedef void (*write_handler)(uint8_t *buf, uint32_t len, void *user_data);

struct WorkerInfo
{
    uint32_t id;
    uint32_t total_blocks_count;
    uint32_t input_buf_len;
    uint8_t *input_buf;

    uint32_t *current_free_index;
    pthread_mutex_t *m_current_free_index;

    pthread_mutex_t *m_worker_is_allowed_to_write;
    pthread_cond_t *cond_worker_is_allowed_to_write;
    uint32_t *worker_is_allowed_to_write;

    uint32_t *crc;
    write_handler write;
    void *user_data;
};

int32_t BLOCK_LEN = 128 * 1024;

void *worker(void *params)
{
    struct WorkerInfo *info = (struct WorkerInfo *)params;

    printf("Thread id=%i started\n", info->id);

    while (1)
    {
        // TODO: Use another way
        int32_t chunk_id = -1;
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

        uint32_t outlen;
        uint8_t is_last = chunk_id == info->total_blocks_count - 1;
        uint32_t chunk_length = is_last ? info->input_buf_len - BLOCK_LEN * chunk_id : BLOCK_LEN;

        printf("Thread id=%i range is %i, len=%i\n", info->id, BLOCK_LEN * chunk_id, chunk_length);
        uint32_t crc_block;
        uint8_t *out = compress_chunk((uint8_t *)(info->input_buf + BLOCK_LEN * chunk_id), chunk_length, &outlen, &crc_block,
                                      BLOCK_LEN * chunk_id, is_last ? 0 : info->input_buf_len - BLOCK_LEN * (chunk_id + 1));

        printf("Thread id=%i done chunk=%i\n", info->id, chunk_id);
        pthread_mutex_lock(info->m_worker_is_allowed_to_write);
        while (*info->worker_is_allowed_to_write != chunk_id)
        {
            printf("Thread id=%i not my turn to write, chunk_id=%i cur=%i\n", info->id, chunk_id, *info->worker_is_allowed_to_write);
            pthread_cond_wait(info->cond_worker_is_allowed_to_write, info->m_worker_is_allowed_to_write);
        };
        printf("Thread id=%i writing to output chunk=%i\n", info->id, chunk_id);

        info->write(out, outlen, info->user_data);
        free(out);

        *info->crc = crc32_block_combine(*info->crc, crc_block);

        *info->worker_is_allowed_to_write += 1;
        pthread_mutex_unlock(info->m_worker_is_allowed_to_write);
        pthread_cond_broadcast(info->cond_worker_is_allowed_to_write);
    };
}

void gzip(uint8_t *input_buf, uint32_t input_buf_len, uint32_t threads_count, write_handler write, void *write_user_data)
{
    uint32_t total_blocks_count = input_buf_len / BLOCK_LEN + (input_buf_len % BLOCK_LEN == 0 ? 0 : 1);
    printf("Starting gzip len=%i blocks_count=%i\n", input_buf_len, total_blocks_count);

    uint32_t current_free_index = 0;

    uint32_t worker_is_allowed_to_write = 0;

    uint32_t crc = CRC32_INITIAL;

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
    for (int32_t i = 0; i < threads_count; i++)
    {
        struct WorkerInfo *params = &threads_params[i];
        params->id = i;
        params->current_free_index = &current_free_index;
        params->m_current_free_index = &m_current_free_index;
        params->total_blocks_count = total_blocks_count;
        params->worker_is_allowed_to_write = &worker_is_allowed_to_write;
        params->m_worker_is_allowed_to_write = &m_worker_is_allowed_to_write;
        params->cond_worker_is_allowed_to_write = &cond_worker_is_allowed_to_write;
        params->write = write;
        params->user_data = write_user_data;
        params->input_buf = input_buf;
        params->input_buf_len = input_buf_len;
        params->crc = &crc;
        pthread_create(&threads[i], NULL, &worker, (void *)params);
    };

    printf("Threads created\n");

    printf("Initializing crc table\n");
    crc32_init_table();

    printf("Now threads will pickup tasks\n");
    pthread_mutex_unlock(&m_current_free_index);

    printf("Writing header\n");
    uint8_t header[10] = {
        0x1f, //ID1
        0x8b, //ID2
        8,    // Deflate
        0,    // No flags
        0,    // TODO time
        0,    // TODO time
        0,    // TODO time
        0,    // TODO time
        0,    // no XFL
        3,    // Unix FS
    };

    write(header, 10, write_user_data);

    printf("Now allowing threads to write\n");
    pthread_mutex_unlock(&m_worker_is_allowed_to_write);
    pthread_cond_broadcast(&cond_worker_is_allowed_to_write);

    printf("Now waiting for all blocks to finish\n");
    pthread_mutex_lock(&m_worker_is_allowed_to_write);
    while (worker_is_allowed_to_write < total_blocks_count)
    {
        pthread_cond_wait(&cond_worker_is_allowed_to_write, &m_worker_is_allowed_to_write);
    }
    printf("All threads are done\n");

    if (input_buf_len == 0)
    {
        printf("No input stream, writing zero-length block\n");
        uint8_t zero_length_block[] = {1, 0, 0, 0xFF, 0xFF};
        write(zero_length_block, 5, write_user_data);
    }
    // Currently crc combining does not support small files
    // So, here is small workaround
    crc = input_buf_len > 4 ? crc32_finallize(crc) : crc32(input_buf, input_buf_len);

    uint32_t footer[2] = {crc, input_buf_len};

    write((uint8_t *)footer, 8, write_user_data);

    pthread_mutex_unlock(&m_worker_is_allowed_to_write);

    printf("Now joining threads\n");
    for (int32_t i = 0; i < threads_count; i++)
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