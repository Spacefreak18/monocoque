#ifndef _REVBURNER_H
#define _REVBURNER_H

int revburner_update(TachDevice* tachdevice, int pulses);
int revburner_init(TachDevice* tachdevice);
int revburner_free(TachDevice* tachdevice);

#endif
