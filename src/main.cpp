#include "shared_memory.hpp"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>

void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(nullptr, size, protection, visibility, -1, 0);
}


int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("Using: \"test_sharedmemory N\" where N number of processes\n");
        return -1;
    }

    void* shmem = create_shared_memory(128);

    auto thread_num = std::atoi(argv[1]);

    for (int i{0}; i < thread_num; ++i)
    {
        if( fork() ) break;
    }

    return 0;
}
