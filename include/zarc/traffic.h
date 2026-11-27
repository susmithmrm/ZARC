#ifndef ZARC_TRAFFIC_H
#define ZARC_TRAFFIC_H
#include "zarc/connect.h"
#define ZARC_CORE_QUEUE 4u
#define ZARC_BULK_QUEUE 2u
typedef struct {
 zarc_frame core[ZARC_CORE_QUEUE],bulk[ZARC_BULK_QUEUE];
 size_t core_head,core_count,bulk_head,bulk_count,core_peak,bulk_peak;
 uint32_t core_rejected,bulk_rejected;
} zarc_traffic;
/* Serialized C3 worker only; native kernel queues remain the cross-thread API.
 * Core/control/health capacity is independent of bulk capacity. */
bool zarc_traffic_push(zarc_traffic *,const zarc_frame *,bool core);
bool zarc_traffic_pop(zarc_traffic *,zarc_frame *,bool *core);
#endif
