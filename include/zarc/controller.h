#ifndef ZARC_CONTROLLER_H
#define ZARC_CONTROLLER_H
#include "zarc/connect.h"
#include "zarc/domain.h"
#define ZARC_CONTROLLER_DOMAINS 4u
typedef struct {
 uint32_t id;
 uint64_t feedback_max_age;
} zarc_endpoint_config;
typedef struct {
 zarc_endpoint_config config;
 zarc_domain domain;
 uint64_t internal_id, source_sequence, source_tick, admitted_tick, request_id, session;
 bool observed, ready, correlated, effect_outstanding;
} zarc_endpoint;
typedef struct {
 zarc_link link;
 zarc_endpoint endpoints[ZARC_CONTROLLER_DOMAINS];
 size_t count;
 uint64_t last_tick;
} zarc_controller;
typedef enum { ZARC_QUALITY_UNKNOWN, ZARC_QUALITY_VALID, ZARC_QUALITY_STALE, ZARC_QUALITY_FAULT } zarc_quality;
typedef struct { uint32_t id; zarc_value value; zarc_quality quality; uint64_t boot, generation, sample_tick, source_sequence; bool ready; } zarc_observation;
typedef struct { uint32_t id; zarc_value desired; bool emitted; uint64_t boot, token, deadline; } zarc_controller_effect;
/* Single owner. The caller supplies verified authority and a reserved effect slot.
 * Local feedback and time progression remain available after link disconnect.
 * This is a benign indicator reference, not an automotive actuator policy. */
bool zarc_controller_init(zarc_controller *, const zarc_endpoint_config *, size_t, uint64_t boot, uint64_t max_validity, uint64_t now);
bool zarc_controller_ready(zarc_controller *, uint32_t id, bool ready);
bool zarc_controller_tick(zarc_controller *, uint64_t now);
zarc_conn_status zarc_controller_open(zarc_controller *, uint64_t session, uint64_t verified_principal, uint8_t major, uint8_t minor, uint64_t now);
void zarc_controller_disconnect(zarc_controller *);
zarc_conn_status zarc_controller_request(zarc_controller *, const zarc_intent *, bool allowed, bool age_known, uint64_t age, uint64_t now, bool effect_slot_reserved, zarc_controller_effect *);
/* Immutable effect identity is owned until provider proves no future execution.
 * The sole controller owner calls released after synchronized completion or removal. */
bool zarc_controller_effect_released(zarc_controller *,const zarc_controller_effect *,uint64_t now);
bool zarc_controller_feedback(zarc_controller *, uint32_t id, uint64_t boot, uint64_t sequence, uint64_t sample_tick, uint64_t now, zarc_value);
bool zarc_controller_snapshot(const zarc_controller *, uint32_t id, zarc_observation *);
/* Trusted local policy, no remote session required. No preemption of accepted work. */
zarc_conn_status zarc_controller_local(zarc_controller *,uint32_t,zarc_value,bool,uint64_t,uint64_t,bool,zarc_controller_effect *);
typedef enum { ZARC_CANCEL_DENIED,ZARC_CANCEL_UNKNOWN,ZARC_CANCEL_TERMINAL,ZARC_CANCEL_IN_FLIGHT,ZARC_CANCEL_REMOVED } zarc_cancel_status;
/* removed_before_execution is supplied only by the synchronized effect owner. */
zarc_cancel_status zarc_controller_cancel(zarc_controller *,uint32_t,uint64_t,uint64_t,bool,bool,uint64_t);
#endif
