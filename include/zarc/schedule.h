#ifndef ZARC_SCHEDULE_H
#define ZARC_SCHEDULE_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
 uint64_t period,deadline,next_release,last_tick,current_release,current_deadline;
 uint64_t skipped,missed,completed;
 bool configured,pending,exhausted;
} zarc_periodic;
typedef struct {uint64_t release,deadline,skipped;bool due;} zarc_release;
/* Absolute phase-preserving releases. Coalesce missed cycles explicitly; never
 * burst replay. Tick and completion timestamps share a monotonic clock. */
bool zarc_periodic_init(zarc_periodic *,uint64_t first,uint64_t period,uint64_t deadline);
bool zarc_periodic_poll(zarc_periodic *,uint64_t now,zarc_release *);
/* Completion must identify its release; stale completions never close a later job. */
bool zarc_periodic_complete(zarc_periodic *,uint64_t release,uint64_t now,bool *on_time);
#endif
