#ifndef ZARC_DOMAIN_H
#define ZARC_DOMAIN_H
#include <stdbool.h>
#include <stdint.h>
typedef enum { ZARC_UNKNOWN, ZARC_OFF, ZARC_ON, ZARC_FAULT } zarc_value;
typedef enum { ZARC_ACCEPTED, ZARC_COMPLETED, ZARC_DENIED, ZARC_BUSY, ZARC_CONFLICT, ZARC_UNKNOWN_OUTCOME, ZARC_INVALID } zarc_status;
typedef struct {
    zarc_value observed, desired;
    uint64_t generation, request_id, deadline, last_tick;
    bool pending, retained;
    zarc_status result;
} zarc_domain;
typedef struct { bool emitted; zarc_value desired; } zarc_effect;
/* Single-owner simulated domain. Request IDs increase within the prototype session.
 * The most recent ID is retained; older IDs are refused. Deadline is exclusive.
 * A retry does not alter the original deadline. */
void zarc_init(zarc_domain *domain);
zarc_status zarc_request(zarc_domain *domain, bool allowed, uint64_t id, zarc_value desired, uint64_t tick, uint64_t deadline, zarc_effect *effect);
zarc_status zarc_feedback(zarc_domain *domain, zarc_value value, uint64_t tick);
zarc_status zarc_tick(zarc_domain *domain, uint64_t tick);
#endif
