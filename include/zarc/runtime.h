#ifndef ZARC_RUNTIME_H
#define ZARC_RUNTIME_H
#include "zarc/composition.h"
#include "zarc/controller.h"
#include "zarc/diagnostics.h"
#include "zarc/health.h"
#include "zarc/observations.h"
#include "zarc/schedule.h"
typedef struct {
 zarc_endpoint_config endpoint;
 uint64_t period,deadline,health_limit,max_validity,config_digest,build_digest;
} zarc_runtime_config;
typedef struct {
 zarc_runtime_config config;
 zarc_composition composition;
 zarc_controller controller;
 zarc_health health;
 zarc_diagnostics diagnostics;
 zarc_observation_stream observations;
 zarc_periodic cycle;
 uint64_t health_sequence;
 bool running,healthy,reported_health;
} zarc_runtime;
/* Single-owner benign indicator reference. The supplied sample is an observation
 * from an independent provider, not a copy of desired state. No hardware policy. */
bool zarc_runtime_init(zarc_runtime *,const zarc_runtime_config *,uint64_t boot,uint64_t now);
bool zarc_runtime_step(zarc_runtime *,uint64_t now,bool sample_present,uint64_t sequence,uint64_t sample_tick,zarc_value);
zarc_conn_status zarc_runtime_local(zarc_runtime *,zarc_value,bool authorized,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *);
bool zarc_runtime_release(zarc_runtime *,const zarc_controller_effect *,uint64_t now);
bool zarc_runtime_stop(zarc_runtime *,uint64_t now,uint64_t deadline);
#endif
