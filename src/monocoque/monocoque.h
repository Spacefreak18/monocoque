/*
 * Public source-tree entry point for libmonocoque.
 *
 * This is an umbrella header for the current API.  It deliberately does not
 * depend on NAppGUI; frontends may use different UI toolkits.
 *
 * TODO: replace internal types with an opaque, installable API before making
 * ABI compatibility guarantees.
 */
#ifndef MONOCOQUE_H
#define MONOCOQUE_H

#ifdef __cplusplus
extern "C" {
#endif

/* A marker for consumers as the public API evolves. */
unsigned int monocoque_api_version(void);

#ifdef __cplusplus
}
#endif

/* Existing public-to-frontends operations and types. */
#include "helper/parameters.h"
#include "helper/confighelper.h"
#include "helper/dirhelper.h"
#include "gameloop/gameloop.h"
#include "gameloop/loopdata.h"
#include "gameloop/tachconfig.h"
#include "devices/simdevice.h"
#include "devices/sound.h"
#include "simulatorapi/simapi/simapi/simdata.h"
#include "slog/slog.h"

#endif /* MONOCOQUE_H */
