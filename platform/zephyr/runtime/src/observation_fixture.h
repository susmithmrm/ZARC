#ifndef ZARC_OBSERVATION_FIXTURE_H
#define ZARC_OBSERVATION_FIXTURE_H
#include "zarc/observations.h"
struct zarc_profile_observer {
 uint64_t boot,cursor,delivered;
 zarc_observation cache[ZARC_CONTROLLER_DOMAINS];
 size_t count;
 bool valid;
};
/* Single-owner simulator bridge: no blocking observers or hidden replay. */
bool zarc_profile_observer_barrier(struct zarc_profile_observer *,const zarc_observation_stream *);
bool zarc_profile_observer_pump(struct zarc_profile_observer *,const zarc_observation_stream *);
#endif
