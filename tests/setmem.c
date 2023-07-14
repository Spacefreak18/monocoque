#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

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

    fd = shm_open(TEST_MEM_FILE_LOCATION, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        return 10;
    }

    SimData* data = malloc(sizeof(SimData));

    data->gear=0;
    data->rpms=0;
    data->maxrpm=0;
    data->velocity=0;
    //SimData.pulses=0;

    res = ftruncate(fd, sizeof(SimData));
    if (res == -1)
    {
        perror("ftruncate");
        return 20;
    }

    data->gear=1;
    data->rpms=500;
    data->maxrpm=6500;
    data->velocity=25;
    //SimData.pulses=38792;

    // map .shared memory to process address space
    addr = mmap(NULL, sizeof(SimData), PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        return 30;
    }

    printf("size of int %i\n", sizeof(int));
    printf("size of struct %i\n", sizeof(SimData));
    memcpy(addr, data, sizeof(SimData));
    //memcpy(addr, &SimData, sizeof(int)*3);

    printf("PID %d: velocity: %i\n", pid, data->velocity);
    printf("PID %d: rpms: %i\n", pid, data->rpms);
    printf("PID %d: gear: %i\n", pid, data->gear);
    sleep(4);


    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);
    char ch;

    int go=1;
    while (go>0)
    {

        struct termios info;
        tcgetattr(0, &info);
        info.c_lflag &= (~ICANON & ~ECHO);
        info.c_cc[VMIN] = 1;
        info.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &info);
        char ch;
        scanf("%c", &ch);


        if (ch == 'q')
        {
            go=0;
        }

    }
    tcsetattr(0, TCSANOW, &canonicalmode);

    fd = shm_unlink(TEST_MEM_FILE_LOCATION);
    if (fd == -1)
    {
        perror("unlink");
        return 100;
    }

    return 0;
}
