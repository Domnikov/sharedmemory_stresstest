#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Here to change shared memory buffer size
const static int MEM_SZ = 64;

int main(int argc, char** argv)
{
    typedef struct
    {
        pthread_mutex_t mutex;
        char data[MEM_SZ];
    } shared_data_t;

    if(argc != 3)
    {
        printf("Using: \"test_sharedmemory M N\" where M number of processes and N stress test iteration number\n");
        return -1;
    }

    int thread_num = atoi(argv[1]);
    int stress_num = atoi(argv[2]);

    // Create shared memory
    shared_data_t* shmem = (shared_data_t*)mmap(NULL, sizeof (shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Create shared mutex
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shmem->mutex, &mutex_attr);

    // Create processes
    for (int i = 0; i < thread_num; ++i)
    {
        if( fork() == 0 ) break;
    }
    srand (time(NULL));

    // Stress test loop
    for (int i = 0; i < stress_num; ++i)
    {
        pthread_mutex_lock(&shmem->mutex);
        char pattern = shmem->data[0];
        for (int i = 0; i < MEM_SZ; ++i)
        {
            if(pattern != shmem->data[i])
            {
                fprintf(stderr, "BOOM!\n");
                return -1;
            }
        }

        pattern = rand() % 0xFF;

        for (int i = 0; i < MEM_SZ; ++i)
        {
            shmem->data[i] = pattern;
        }
        pthread_mutex_unlock(&shmem->mutex);
    }


    munmap(shmem, MEM_SZ);
    return 0;
}
