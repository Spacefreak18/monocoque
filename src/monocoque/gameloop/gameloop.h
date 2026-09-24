#include "../devices/simdevice.h"
#include "../helper/parameters.h"
#include "loopdata.h"

extern int appstate;

int tester(MonocoqueSettings* ms, SimDevice* devices, int numdevices);
int looper(SimDevice* devices, int numdevices, Parameters* p);

int monocoque_mainloop(MonocoqueSettings* ms);

int start_loop(MonocoqueSettings* ms);
int start_test(test_loop_args* test_loop_data);
int monocoque_mainloop_stop(MonocoqueSettings* ms);
int monocoque_testloop_stop();

const char* get_simd_onoff(void);
const char* get_simexe_name(void);
SimData* get_test_simdata();


void set_basic_simdata(SimData* simdata);
void set_wheel_spin_simdata(SimData* simdata);
void set_wheel_lock_simdata(SimData* simdata);
