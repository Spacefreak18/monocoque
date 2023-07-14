#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "../src/monocoque/simulatorapi/simapi/simapi/simdata.h"
#include "../src/monocoque/simulatorapi/simapi/simapi/test.h"

#define DATA "Hello, World! From PID %d"

int main(int argc, char* argv[])
{
    int res;
    int fd;
    int len;
    pid_t pid;
    void* addr;

    pid = getpid();

    // get shared memory file descriptor (NOT a file)
    fd = shm_open(TEST_MEM_FILE_LOCATION, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        return 10;
    }

    // extend shared memory object as by default it's initialized with size 0
    //res = ftruncate(fd, STORAGE_SIZE);
    SimData* data = malloc(sizeof(SimData));
    data->velocity=1;
    data->gear=4;
    data->maxrpm=6500;
    data->rpms=1000;
    //SimData.pulses=398273;

    res = ftruncate(fd, sizeof(SimData));
    if (res == -1)
    {
        //perror("ftruncate");
        //return 20;
    }

    data->velocity=atoi(argv[1]);
    data->rpms=atoi(argv[2]);
    data->gear=atoi(argv[3]);
    //SimData.pulses=39993;
    // map .shared memory to process address space
    addr = mmap(NULL, sizeof(SimData), PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        return 30;
    }

    memcpy(addr, data, sizeof(SimData));

    return 0;
}
