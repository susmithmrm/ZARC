#ifndef ZARC_NATIVE_OBSERVATION_BUS_H
#define ZARC_NATIVE_OBSERVATION_BUS_H
#include "zarc/observations.h"
/* Dedicated candidate channel, one publisher and one consumer. The synchronous
 * listener only copies to a bounded queue; overflow requires a fresh barrier.
 * Reset requires both owners quiescent, then a new snapshot before events. */
int zarc_native_observation_publish(const zarc_observation_event *);
int zarc_native_observation_read(zarc_observation_event *);
void zarc_native_observation_reset(void);
#endif
