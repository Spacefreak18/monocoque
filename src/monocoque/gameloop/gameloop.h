#include "../devices/simdevice.h"
#include "../helper/parameters.h"

extern int appstate;

int tester(SimDevice* devices, int numdevices);
int looper(SimDevice* devices, int numdevices, Parameters* p);

int monocoque_mainloop(MonocoqueSettings* ms);
int monocoque_mainloop_start(MonocoqueSettings* ms);
int monocoque_mainloop_stop(MonocoqueSettings* ms);
