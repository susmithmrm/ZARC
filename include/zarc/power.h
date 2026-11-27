#ifndef ZARC_POWER_H
#define ZARC_POWER_H
#include <stdbool.h>
#include <stdint.h>
typedef enum {ZARC_POWER_IDLE,ZARC_POWER_WAITING,ZARC_POWER_READY,ZARC_POWER_FALLBACK,ZARC_POWER_CANCELLED} zarc_power_phase;
typedef struct {uint32_t required_mask,ready_mask;uint64_t epoch,request_id,deadline,last_tick;
 zarc_power_phase phase;bool configured;} zarc_power;
/* Guest readiness aggregation only. No rail, reset or electrical commands.
 * FALLBACK signals the product owner to execute its approved policy. */
bool zarc_power_init(zarc_power *,uint32_t required_mask,uint64_t epoch,uint64_t now);
bool zarc_power_request(zarc_power *,uint64_t id,uint64_t now,uint64_t deadline,bool authorized);
bool zarc_power_report(zarc_power *,uint64_t epoch,uint64_t id,unsigned guest,bool ready,uint64_t now);
bool zarc_power_tick(zarc_power *,uint64_t now);
bool zarc_power_cancel(zarc_power *,uint64_t id,uint64_t now,bool authorized);
#endif
