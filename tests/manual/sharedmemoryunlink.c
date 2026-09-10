#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

void display(char* prog, char* bytes, int n);

int main(void)
{
    const char* name = "acpmf_physics";    // file name

    shm_unlink(name);
}

