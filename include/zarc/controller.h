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
 bool observed, ready, correlated;
} zarc_endpoint;
typedef struct {
 zarc_link link;
 zarc_endpoint endpoints[ZARC_CONTROLLER_DOMAINS];
 size_t count;
 uint64_t last_tick;
} zarc_controller;
typedef enum { ZARC_QUALITY_UNKNOWN, ZARC_QUALITY_VALID, ZARC_QUALITY_STALE, ZARC_QUALITY_FAULT } zarc_quality;
typedef struct { uint32_t id; zarc_value value; zarc_quality quality; uint64_t boot, generation, sample_tick, source_sequence; bool ready; } zarc_observation;
typedef struct { uint32_t id; zarc_value desired; bool emitted; } zarc_controller_effect;
/* Single owner. The caller supplies verified authority and a reserved effect slot.
 * Local feedback and time progression remain available after link disconnect.
 * This is a benign indicator reference, not an automotive actuator policy. */
bool zarc_controller_init(zarc_controller *, const zarc_endpoint_config *, size_t, uint64_t boot, uint64_t max_validity, uint64_t now);
bool zarc_controller_ready(zarc_controller *, uint32_t id, bool ready);
bool zarc_controller_tick(zarc_controller *, uint64_t now);
zarc_conn_status zarc_controller_open(zarc_controller *, uint64_t session, uint64_t verified_principal, uint8_t major, uint8_t minor, uint64_t now);
void zarc_controller_disconnect(zarc_controller *);
zarc_conn_status zarc_controller_request(zarc_controller *, const zarc_intent *, bool allowed, bool age_known, uint64_t age, uint64_t now, bool effect_slot_reserved, zarc_controller_effect *);
bool zarc_controller_feedback(zarc_controller *, uint32_t id, uint64_t boot, uint64_t sequence, uint64_t sample_tick, uint64_t now, zarc_value);
bool zarc_controller_snapshot(const zarc_controller *, uint32_t id, zarc_observation *);
#endif
