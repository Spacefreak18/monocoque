#ifndef _RF2_H
#define _RF2_H

#include <stdbool.h>
#include "simapi/rf2data.h"

#define RF2_TELEMETRY_FILE "rFactor2SMMP_Telemetry"
#define RF2_SCORING_FILE "rFactor2SMMP_Scoring"

typedef struct
{
    bool has_telemetry;
    bool has_scoring;
    void* telemetry_map_addr;
    void* scoring_map_addr;
    struct rF2Telemetry rf2_telemetry;
    //struct rF2Scoring rf2_scoring;
}
RF2Map;

#endif
