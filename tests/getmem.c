#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "../src/monocoque/simulatorapi/simapi/simapi/simdata.h"
#include "../src/monocoque/simulatorapi/simapi/simapi/test.h"

int main(int argc, char* argv[])
{
    int res;
    int fd;
    pid_t pid;
    void* addr;
    SimData* data = malloc(sizeof(SimData));

    pid = getpid();

    // get shared memory file descriptor (NOT a file)
    fd = shm_open(TEST_MEM_FILE_LOCATION, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        return 10;
    }

    // map shared memory to process address space
    addr = mmap(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        return 30;
    }

    // place data into memory

    memcpy(data, addr, sizeof(SimData));

    printf("PID %d: Read from shared memory: \"%s\"\n", pid, data);
    printf("PID %d: velocity: %i\n", pid, data->velocity);
    printf("PID %d: rpms: %i\n", pid, data->rpms);
    printf("PID %d: gear: %i\n", pid, data->gear);

    return 0;
}
