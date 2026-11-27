#ifndef ZARC_OBSERVATIONS_H
#define ZARC_OBSERVATIONS_H
#include "zarc/controller.h"
#define ZARC_EVENT_LIMIT 8u
typedef struct {uint64_t sequence;zarc_observation observation;} zarc_observation_event;
typedef struct {
 uint64_t boot,sequence;
 zarc_observation current[ZARC_CONTROLLER_DOMAINS];
 size_t count,retained,head;
 zarc_observation_event history[ZARC_EVENT_LIMIT];
 bool valid;
} zarc_observation_stream;
typedef enum {ZARC_EVENT_AVAILABLE,ZARC_EVENT_EMPTY,ZARC_EVENT_GAP,ZARC_EVENT_INVALID} zarc_event_status;
/* Same sole owner as controller. Synchronize after dispatch/tick and before
 * serving a barrier. History overwrites are explicit gaps, not silent loss. */
bool zarc_observation_stream_init(zarc_observation_stream *,const zarc_controller *);
bool zarc_observation_stream_sync(zarc_observation_stream *,const zarc_controller *);
bool zarc_observation_barrier(const zarc_observation_stream *,uint32_t,zarc_observation *,uint64_t *);
zarc_event_status zarc_observation_next(const zarc_observation_stream *,uint64_t boot,uint64_t cursor,zarc_observation_event *);
#endif
