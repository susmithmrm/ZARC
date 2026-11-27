#include "zarc/runtime.h"
static uint64_t sat_add(uint64_t a,uint64_t b){return b>UINT64_MAX-a?UINT64_MAX:a+b;}
static void highwater(zarc_runtime *r){
 uint32_t effects=0;
 for(size_t i=0;i<r->controller.count;++i)if(r->controller.endpoints[i].effect_outstanding)++effects;
 if(effects>r->metrics.effect_highwater)r->metrics.effect_highwater=effects;
 if(r->controller.link.count>r->metrics.retained_highwater)r->metrics.retained_highwater=(uint32_t)r->controller.link.count;
 if(r->diagnostics.count>r->metrics.diagnostic_highwater)r->metrics.diagnostic_highwater=(uint32_t)r->diagnostics.count;
}
static bool idle(const zarc_runtime *r){
 for(size_t i=0;i<r->controller.count;++i)
  if(r->controller.endpoints[i].domain.pending||r->controller.endpoints[i].effect_outstanding)return false;
 return true;
}
static bool core_init(void *p){return ((zarc_runtime *)p)->controller.link.boot!=0;}
static bool health_init(void *p){return ((zarc_runtime *)p)->health.configured;}
static bool core_start(void *p){return ((zarc_runtime *)p)->observations.valid;}
static bool health_start(void *p){return ((zarc_runtime *)p)->cycle.configured;}
static bool core_quiesce(void *p,uint64_t deadline){
 zarc_runtime *r=p;return r->controller.last_tick<deadline&&idle(r);
}
static bool health_quiesce(void *p,uint64_t deadline){return ((zarc_runtime *)p)->health.last_tick<deadline;}
static bool logical_transition(void *p){return p!=NULL;}
static void core_release(void *p){
 zarc_runtime *r=p;
 for(size_t i=0;i<r->controller.count;++i)
  (void)zarc_controller_ready(&r->controller,r->controller.endpoints[i].config.id,false);
 zarc_controller_disconnect(&r->controller);
}
static void health_release(void *p){((zarc_runtime *)p)->healthy=false;}
static bool health_reset(zarc_runtime *r,uint64_t now){
 zarc_health_policy policy[ZARC_CONTROLLER_DOMAINS]={0};
 for(size_t i=0;i<r->controller.count;++i)
  policy[i]=(zarc_health_policy){.id=r->controller.endpoints[i].config.id,
   .alive_limit=r->config.health_limit,.deadline_limit=r->config.health_limit,.required=true};
 return zarc_health_init(&r->health,policy,r->controller.count,r->controller.link.boot,now);
}
bool zarc_runtime_init(zarc_runtime *r,const zarc_runtime_config *cfg,uint64_t boot,uint64_t now){
 if(!r||!cfg||r->running||!cfg->period||!cfg->deadline||cfg->deadline>cfg->period||
    cfg->health_limit<=cfg->period||cfg->count>ZARC_CONTROLLER_DOMAINS||
    cfg->extension_count>ZARC_SERVICE_LIMIT-2u||(!cfg->extensions&&cfg->extension_count)||
    (cfg->remote_service&&!cfg->remote_enabled))return false;
 const size_t n=cfg->count?cfg->count:1u;
 const zarc_endpoint_config *endpoints=cfg->count?cfg->endpoints:&cfg->endpoint;
 for(size_t i=0;i<n;++i)if(endpoints[i].feedback_max_age<=cfg->period)return false;
 *r=(zarc_runtime){0};r->config=*cfg;r->sample_not_before=now;
 if(!zarc_controller_init(&r->controller,endpoints,n,boot,cfg->max_validity,now)||
    !health_reset(r,now)||!zarc_periodic_init(&r->cycle,now,cfg->period,cfg->deadline)||
    !zarc_observation_stream_init(&r->observations,&r->controller))return false;
 zarc_service_descriptor services[ZARC_SERVICE_LIMIT]={
  {.id=1,.required=true,.context=r,.init=core_init,.start=core_start,.quiesce=core_quiesce,
   .suspend=logical_transition,.resume=logical_transition,.release=core_release},
  {.id=2,.required=true,.dependency_count=1,.dependencies={1},.context=r,.init=health_init,.start=health_start,
   .quiesce=health_quiesce,.suspend=logical_transition,.resume=logical_transition,.release=health_release}
 };
 bool remote_found=!cfg->remote_service;
 for(size_t i=0;i<cfg->extension_count;++i){
  services[i+2u]=cfg->extensions[i];
  if(services[i+2u].id==cfg->remote_service)remote_found=true;
 }
 if(!remote_found||zarc_composition_configure(&r->composition,services,cfg->extension_count+2u)!=ZARC_COMPOSITION_OK)return false;
 /* Descriptors copied by composition; do not retain a borrowed descriptor array. */
 r->config.extensions=NULL;r->config.extension_count=0;
 (void)zarc_composition_start(&r->composition);
 if(!zarc_composition_admits(&r->composition,1)||!zarc_composition_admits(&r->composition,2))return false;
 r->running=true;return true;
}
static bool update_health(zarc_runtime *r,uint64_t now){
 zarc_health_snapshot h;
 if(!zarc_health_check(&r->health,now,&h))return false;
 r->health_snapshot=h;
 bool common=!r->suspended&&zarc_composition_admits(&r->composition,1)&&zarc_composition_admits(&r->composition,2);
 r->healthy=common&&zarc_composition_required_ready(&r->composition);
 for(size_t i=0;i<r->controller.count;++i){
  zarc_observation o;
  uint32_t id=r->controller.endpoints[i].config.id,bit=UINT32_C(1)<<i;
  if(!zarc_controller_snapshot(&r->controller,id,&o))return false;
  bool ready=common&&!(bit&(h.alive_failed|h.deadline_failed|h.logical_failed))&&o.quality==ZARC_QUALITY_VALID;
  if(!ready&&(!r->reported_health[i]||r->endpoint_healthy[i])){
   const zarc_fault fault={.version=ZARC_DIAG_VERSION,.code=1,.service=id,.epoch=r->controller.link.boot,
    .generation=o.generation,.timestamp=now,.config_digest=r->config.config_digest,
    .build_digest=r->config.build_digest,.quality=ZARC_DIAG_FAULT,.severity=ZARC_DIAG_ERROR};
   (void)zarc_diag_record(&r->diagnostics,&fault);
  }
  r->endpoint_healthy[i]=ready;r->reported_health[i]=true;r->healthy=r->healthy&&ready;
  if(!zarc_controller_ready(&r->controller,id,ready))return false;
 }
 highwater(r);
 return zarc_observation_stream_sync(&r->observations,&r->controller);
}
bool zarc_runtime_refresh(zarc_runtime *r,uint64_t now){
 return r&&r->running&&!r->suspended&&zarc_controller_tick(&r->controller,now)&&update_health(r,now);
}
bool zarc_runtime_step_batch(zarc_runtime *r,uint64_t now,const zarc_runtime_sample *samples,size_t n){
 if(!r||!r->running||r->suspended||n>r->controller.count||(!samples&&n)||now<r->controller.last_tick)return false;
 /* Validate batch identity before changing runtime state. */
 for(size_t i=0;i<n;++i){
  bool found=false;
  for(size_t j=0;j<r->controller.count;++j)if(samples[i].id==r->controller.endpoints[j].config.id)found=true;
  if(!found)return false;
  for(size_t j=0;j<i;++j)if(samples[j].id==samples[i].id)return false;
 }
 if(!zarc_controller_tick(&r->controller,now))return false;
 const uint64_t old_skipped=r->cycle.skipped,old_missed=r->cycle.missed,old_completed=r->cycle.completed;
 zarc_release release;
 if(!zarc_periodic_poll(&r->cycle,now,&release))return false;
 if(release.due){
  bool valid[ZARC_CONTROLLER_DOMAINS]={0};
  for(size_t i=0;i<n;++i)for(size_t j=0;j<r->controller.count;++j)
   if(samples[i].id==r->controller.endpoints[j].config.id)
    valid[j]=samples[i].tick>=r->sample_not_before&&zarc_controller_feedback(&r->controller,samples[i].id,r->controller.link.boot,
     samples[i].sequence,samples[i].tick,now,samples[i].value)&&samples[i].value!=ZARC_FAULT;
  bool on_time=false;
  if(!zarc_periodic_complete(&r->cycle,release.release,now,&on_time)||r->health_sequence==UINT64_MAX)return false;
  uint64_t response=now-release.release;
  if(response>r->metrics.max_release_to_completion)r->metrics.max_release_to_completion=response;
  ++r->health_sequence;
  for(size_t i=0;i<r->controller.count;++i)
   if(!zarc_health_report(&r->health,r->controller.endpoints[i].config.id,r->controller.link.boot,
      r->health_sequence,now,valid[i]&&on_time,valid[i]&&on_time))return false;
 }
 r->metrics.completed=sat_add(r->metrics.completed,r->cycle.completed-old_completed);
 r->metrics.missed=sat_add(r->metrics.missed,r->cycle.missed-old_missed);
 r->metrics.skipped=sat_add(r->metrics.skipped,r->cycle.skipped-old_skipped);
 return update_health(r,now);
}
bool zarc_runtime_step(zarc_runtime *r,uint64_t now,bool present,uint64_t sequence,uint64_t tick,zarc_value value){
 if(!r||r->controller.count!=1)return false;
 const zarc_runtime_sample s={r->controller.endpoints[0].config.id,sequence,tick,value};
 return zarc_runtime_step_batch(r,now,&s,present?1u:0u);
}
zarc_conn_status zarc_runtime_local_at(zarc_runtime *r,uint32_t id,zarc_value value,bool authorized,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *effect){
 if(!effect)return ZARC_CONN_INVALID;
 *effect=(zarc_controller_effect){0};
 if(!r||!r->running||r->suspended||!zarc_controller_tick(&r->controller,now)||!update_health(r,now))return ZARC_CONN_UNAVAILABLE;
 zarc_conn_status status=zarc_controller_local(&r->controller,id,value,authorized,now,validity,reserved,effect);
 highwater(r);return status;
}
zarc_conn_status zarc_runtime_local(zarc_runtime *r,zarc_value value,bool authorized,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *effect){
 if(!r||r->controller.count!=1){if(effect)*effect=(zarc_controller_effect){0};return ZARC_CONN_INVALID;}
 return zarc_runtime_local_at(r,r->controller.endpoints[0].config.id,value,authorized,now,validity,reserved,effect);
}
bool zarc_runtime_health_read(const zarc_runtime *r,uint32_t id,zarc_runtime_health_view *out){
 if(!r||!r->running||!out)return false;
 for(size_t i=0;i<r->controller.count;++i){
  const zarc_endpoint *e=&r->controller.endpoints[i];
  if(e->config.id!=id)continue;
  uint32_t bit=UINT32_C(1)<<i,failures=0;
  const zarc_health_snapshot *h=&r->health_snapshot;
  if(h->alive_failed&bit)failures|=ZARC_HEALTH_ALIVE;
  if(h->deadline_failed&bit)failures|=ZARC_HEALTH_DEADLINE;
  if(h->logical_failed&bit)failures|=ZARC_HEALTH_LOGICAL;
  if(!r->reported_health[i])failures=ZARC_HEALTH_ALIVE|ZARC_HEALTH_DEADLINE|ZARC_HEALTH_LOGICAL;
  *out=(zarc_runtime_health_view){.id=id,.failures=failures,.ready=e->ready&&!r->suspended,
   .pending=e->domain.pending,.effect_owned=e->effect_outstanding,.sample_tick=e->source_tick,.source_sequence=e->source_sequence};
  return true;
 }
 return false;
}
bool zarc_runtime_metrics_read(const zarc_runtime *r,zarc_runtime_metrics *out){
 if(!r||!r->controller.link.boot||!out)return false;
 *out=r->metrics;return true;
}
bool zarc_runtime_remote_ready(const zarc_runtime *r){
 return r&&r->running&&!r->suspended&&r->config.remote_enabled&&
  zarc_composition_admits(&r->composition,1)&&zarc_composition_admits(&r->composition,2)&&
  (!r->config.remote_service||zarc_composition_admits(&r->composition,r->config.remote_service));
}
zarc_conn_status zarc_runtime_open(zarc_runtime *r,uint64_t session,uint64_t principal,uint8_t major,uint8_t minor,uint64_t now){
 if(!zarc_runtime_remote_ready(r))return ZARC_CONN_UNAVAILABLE;
 if(!zarc_controller_tick(&r->controller,now)||!update_health(r,now))return ZARC_CONN_INVALID;
 return zarc_controller_open(&r->controller,session,principal,major,minor,now);
}
void zarc_runtime_disconnect(zarc_runtime *r){if(r)zarc_controller_disconnect(&r->controller);}
zarc_conn_status zarc_runtime_remote(zarc_runtime *r,const zarc_intent *q,bool authorized,bool known,uint64_t age,uint64_t now,bool reserved,zarc_controller_effect *effect){
 if(!effect)return ZARC_CONN_INVALID;
 *effect=(zarc_controller_effect){0};
 if(!zarc_runtime_remote_ready(r))return ZARC_CONN_UNAVAILABLE;
 if(!zarc_controller_tick(&r->controller,now)||!update_health(r,now))return ZARC_CONN_INVALID;
 zarc_conn_status status=zarc_controller_request(&r->controller,q,authorized,known,age,now,reserved,effect);
 highwater(r);return status;
}
bool zarc_runtime_release(zarc_runtime *r,const zarc_controller_effect *effect,uint64_t now){
 return r&&r->running&&!r->suspended&&zarc_controller_effect_released(&r->controller,effect,now)&&update_health(r,now);
}
bool zarc_runtime_suspend(zarc_runtime *r,uint64_t now,uint64_t deadline){
 if(!r||!r->running||r->suspended||now>=deadline||!zarc_controller_tick(&r->controller,now)||!idle(r))return false;
 if(zarc_composition_quiesce(&r->composition,now,deadline)!=ZARC_COMPOSITION_OK||
    zarc_composition_suspend(&r->composition)!=ZARC_COMPOSITION_OK)return false;
 r->suspended=true;core_release(r);
 for(size_t i=0;i<r->controller.count;++i)r->controller.endpoints[i].observed=false;
 return update_health(r,now);
}
bool zarc_runtime_resume(zarc_runtime *r,uint64_t now){
 if(!r||!r->running||!r->suspended||now<r->controller.last_tick||now>UINT64_MAX-r->config.deadline)return false;
 if(!zarc_controller_tick(&r->controller,now)||!health_reset(r,now)||
    !zarc_periodic_init(&r->cycle,now,r->config.period,r->config.deadline)||
    zarc_composition_resume(&r->composition)!=ZARC_COMPOSITION_OK)return false;
 r->suspended=false;r->sample_not_before=now;return update_health(r,now);
}
bool zarc_runtime_stop(zarc_runtime *r,uint64_t now,uint64_t deadline){
 if(!r||!r->running||now>=deadline||!zarc_controller_tick(&r->controller,now)||!idle(r))return false;
 if(!r->suspended&&zarc_composition_quiesce(&r->composition,now,deadline)!=ZARC_COMPOSITION_OK)return false;
 if(zarc_composition_stop(&r->composition)!=ZARC_COMPOSITION_OK)return false;
 r->running=false;r->suspended=false;return true;
}
