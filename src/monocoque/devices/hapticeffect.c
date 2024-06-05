#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "usbhapticdevice.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"

#define kmhtoms      0.277778
#define minspeedinms 0.5
#define minvelocity  50
#define maxbrake     0
#define maxthrottle  0
#define maxXvelocity 0.001

bool hasTyreDiameter(SimData* simdata)
{
    if (simdata->tyrediameter[0] == -1 || simdata->tyrediameter[1] == -1 || simdata->tyrediameter[2] == -1 || simdata->tyrediameter[3] == -1)
    {
        return false;
    }
    return true;
}

void getTyreDiameter(SimData* simdata)
{
    if(simdata->velocity > minvelocity && simdata->brake <= maxbrake && simdata->gas <= maxthrottle)
    {
        if (simdata->velocityX/simdata->velocity < maxXvelocity)
        {
            double Speedms = kmhtoms * simdata->velocity;
            for(int i = 0; i < 4; i++)
            {
                simdata->tyrediameter[i] = Speedms / simdata->tyreRPS[i] * 2;
            }
            slogi("Successfully set tyre diameters for wheel slip effects.");
        }

    }
}


int slipeffect(SimData* simdata, int effecttype, int tyre, double threshold)
{
    int play = 0;
    double wheelslip[4];
    wheelslip[0] = 0;
    wheelslip[1] = 0;
    wheelslip[2] = 0;
    wheelslip[3] = 0;


    switch (effecttype)
    {
        case (EFFECT_TYRESLIP):
        case (EFFECT_TYRELOCK):
        case (EFFECT_ABSBRAKES):

            if(hasTyreDiameter(simdata)==false)
            {
                getTyreDiameter(simdata);
            }
            if(hasTyreDiameter(simdata)==true)
            {
                double Speedms = kmhtoms * simdata->velocity;
                if (Speedms > minspeedinms)
                {
                    for(int i = 0; i < 4; i++)
                    {
                        wheelslip[i] = (Speedms - simdata->tyrediameter[i] * simdata->tyreRPS[i] / 2) / Speedms;
                    }
                }
                else
                {
                    for(int i = 0; i < 4; i++)
                    {
                        wheelslip[i] = 0;
                    }
                }

            }
            break;
    }

    switch (effecttype)
    {
        case (EFFECT_TYRESLIP):


            if (tyre == FRONTLEFT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[0] < -threshold)
                {
                    play++;
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] < -threshold)
                {
                    play++;
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] < -threshold)
                {
                    play++;
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] < -threshold)
                {
                    play++;
                }
            }


            break;
        case (EFFECT_TYRELOCK):
            if (tyre == FRONTLEFT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[0] > threshold)
                {
                    play++;
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] > threshold)
                {
                    play++;
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] > threshold)
                {
                    play++;
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] > threshold)
                {
                    play++;
                }
            }

            break;
        case (EFFECT_ABSBRAKES):
            break;
    }
    
    return play;
}

