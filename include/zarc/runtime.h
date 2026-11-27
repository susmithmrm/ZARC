#ifndef ZARC_RUNTIME_H
#define ZARC_RUNTIME_H
#include "zarc/composition.h"
#include "zarc/controller.h"
#include "zarc/diagnostics.h"
#include "zarc/health.h"
#include "zarc/observations.h"
#include "zarc/schedule.h"
#include "zarc/update.h"
typedef struct {
 zarc_endpoint_config endpoint; /* legacy single endpoint when count is zero */
 uint64_t period,deadline,health_limit,max_validity,config_digest,build_digest;
 uint64_t update_status_max_age; /* zero disables optional local status provider */
 size_t count;
 zarc_endpoint_config endpoints[ZARC_CONTROLLER_DOMAINS];
 bool remote_enabled;
 uint32_t remote_service; /* optional lifecycle service gating remote admission */
 const zarc_service_descriptor *extensions;size_t extension_count;
} zarc_runtime_config;
typedef struct {uint32_t id;uint64_t sequence,tick;zarc_value value;} zarc_runtime_sample;
typedef struct {
 uint64_t completed,missed,skipped,max_release_to_completion;
 uint32_t retained_highwater,effect_highwater,diagnostic_highwater;
} zarc_runtime_metrics;
typedef struct {
 uint32_t id,failures;
 bool ready,pending,effect_owned;
 uint64_t sample_tick,source_sequence;
} zarc_runtime_health_view;
typedef struct {
 zarc_runtime_config config;
 zarc_composition composition;
 zarc_controller controller;
 zarc_health health;
 zarc_diagnostics diagnostics;
 zarc_observation_stream observations;
 zarc_periodic cycle;
 zarc_runtime_metrics metrics;
 zarc_update update_status;
 uint64_t update_revision,update_tick;
 bool update_valid;
 zarc_health_snapshot health_snapshot;
 uint64_t health_sequence,sample_not_before;
 bool running,healthy,suspended;
 bool endpoint_healthy[ZARC_CONTROLLER_DOMAINS],reported_health[ZARC_CONTROLLER_DOMAINS];
} zarc_runtime;
/* Single owner; zero initialize before first init. Samples come from the
 * independent provider. The caller owns physical policy, effect reservation
 * and verified remote identity. This reference operates benign endpoints. */
bool zarc_runtime_init(zarc_runtime *,const zarc_runtime_config *,uint64_t boot,uint64_t now);
bool zarc_runtime_step(zarc_runtime *,uint64_t now,bool sample_present,uint64_t sequence,uint64_t sample_tick,zarc_value);
/* Owner-side read-only snapshots. Metrics count observed runtime work,
 * not a WCET proof or physical resource measurement. */
bool zarc_runtime_health_read(const zarc_runtime *,uint32_t id,zarc_runtime_health_view *);
bool zarc_runtime_metrics_read(const zarc_runtime *,zarc_runtime_metrics *);
/* The serialized, trusted local maintenance owner publishes only reconciled
 * metadata. No flash access, boot request or verification is done here. */
bool zarc_runtime_publish_update(zarc_runtime *,const zarc_update *,uint64_t revision,uint64_t now,bool verified);
void zarc_runtime_invalidate_update(zarc_runtime *);
bool zarc_runtime_read_update(const zarc_runtime *,uint64_t now,zarc_update *,uint64_t *revision,uint64_t *sample_tick);
bool zarc_runtime_remote_ready(const zarc_runtime *);
bool zarc_runtime_refresh(zarc_runtime *,uint64_t now);
bool zarc_runtime_step_batch(zarc_runtime *,uint64_t now,const zarc_runtime_sample *,size_t);
zarc_conn_status zarc_runtime_local(zarc_runtime *,zarc_value,bool authorized,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *);
zarc_conn_status zarc_runtime_local_at(zarc_runtime *,uint32_t,zarc_value,bool authorized,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *);
zarc_conn_status zarc_runtime_open(zarc_runtime *,uint64_t session,uint64_t principal,uint8_t major,uint8_t minor,uint64_t now);
void zarc_runtime_disconnect(zarc_runtime *);
zarc_conn_status zarc_runtime_remote(zarc_runtime *,const zarc_intent *,bool authorized,bool age_known,uint64_t age,uint64_t now,bool reserved,zarc_controller_effect *);
bool zarc_runtime_release(zarc_runtime *,const zarc_controller_effect *,uint64_t now);
/* Logical lifecycle only, provider must be quiescent. Resume requires new
 * ordered observations before admission; native power transitions are separate. */
bool zarc_runtime_suspend(zarc_runtime *,uint64_t now,uint64_t deadline);
bool zarc_runtime_resume(zarc_runtime *,uint64_t now);
bool zarc_runtime_stop(zarc_runtime *,uint64_t now,uint64_t deadline);
#endif
