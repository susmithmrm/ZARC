#ifndef ZARC_HEALTH_H
#define ZARC_HEALTH_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define ZARC_HEALTH_LIMIT 16u
enum { ZARC_HEALTH_ALIVE=1u, ZARC_HEALTH_DEADLINE=2u, ZARC_HEALTH_LOGICAL=4u };
typedef struct { uint32_t id; uint64_t alive_limit, deadline_limit; bool required; } zarc_health_policy;
typedef struct {
 zarc_health_policy policy; uint64_t sequence, last_report, last_completion;
 bool seen, completed, logical;
} zarc_health_entry;
typedef struct {
 zarc_health_entry entries[ZARC_HEALTH_LIMIT]; size_t count;
 uint64_t epoch, last_tick; bool configured;
} zarc_health;
typedef struct { uint32_t alive_failed, deadline_failed, logical_failed; bool required_healthy; } zarc_health_snapshot;
/* The sole owner supplies monotonic ticks and a new epoch on restart.
 * Completion is distinct from heartbeat; reports must be strictly ordered. */
bool zarc_health_init(zarc_health *, const zarc_health_policy *, size_t, uint64_t epoch, uint64_t now);
bool zarc_health_report(zarc_health *, uint32_t id, uint64_t epoch, uint64_t sequence, uint64_t now, bool completed, bool logical);
bool zarc_health_check(zarc_health *, uint64_t now, zarc_health_snapshot *);
#endif
