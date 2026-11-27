#include "zarc/runtime.h"
static bool core_init(void *p){
 zarc_runtime *r=p;return r->controller.link.boot!=0;
}
static bool health_init(void *p){return ((zarc_runtime *)p)->health.configured;}
static bool core_start(void *p){return ((zarc_runtime *)p)->observations.valid;}
static bool health_start(void *p){return ((zarc_runtime *)p)->cycle.configured;}
static bool core_quiesce(void *p,uint64_t deadline){
 zarc_runtime *r=p;
 return r->controller.last_tick<deadline&&!r->controller.endpoints[0].domain.pending&&
        !r->controller.endpoints[0].effect_outstanding;
}
static bool health_quiesce(void *p,uint64_t deadline){return ((zarc_runtime *)p)->health.last_tick<deadline;}
/* Reference runtime exposes stop, not suspend/resume until product wake
 * revalidation and native PM are integrated. */
static bool unsupported_transition(void *p){(void)p;return false;}
static void core_release(void *p){
 zarc_runtime *r=p;(void)zarc_controller_ready(&r->controller,r->config.endpoint.id,false);
 zarc_controller_disconnect(&r->controller);
}
static void health_release(void *p){((zarc_runtime *)p)->healthy=false;}
bool zarc_runtime_init(zarc_runtime *r,const zarc_runtime_config *cfg,uint64_t boot,uint64_t now){
 if(!r||!cfg||r->running||!cfg->period||!cfg->deadline||cfg->deadline>cfg->period||
    cfg->health_limit<=cfg->period||cfg->endpoint.feedback_max_age<=cfg->period)return false;
 *r=(zarc_runtime){0};r->config=*cfg;
 const zarc_health_policy policy={.id=2,.alive_limit=cfg->health_limit,.deadline_limit=cfg->health_limit,.required=true};
 if(!zarc_controller_init(&r->controller,&cfg->endpoint,1,boot,cfg->max_validity,now)||
    !zarc_health_init(&r->health,&policy,1,boot,now)||
    !zarc_periodic_init(&r->cycle,now,cfg->period,cfg->deadline)||
    !zarc_observation_stream_init(&r->observations,&r->controller))return false;
 const zarc_service_descriptor services[]={
  {.id=1,.required=true,.context=r,.init=core_init,.start=core_start,.quiesce=core_quiesce,
   .suspend=unsupported_transition,.resume=unsupported_transition,.release=core_release},
  {.id=2,.required=true,.dependency_count=1,.dependencies={1},.context=r,.init=health_init,.start=health_start,
   .quiesce=health_quiesce,.suspend=unsupported_transition,.resume=unsupported_transition,.release=health_release}
 };
 if(zarc_composition_configure(&r->composition,services,2)!=ZARC_COMPOSITION_OK||
    zarc_composition_start(&r->composition)!=ZARC_COMPOSITION_OK)return false;
 r->running=true;return true;
}
static bool update_health(zarc_runtime *r,uint64_t now){
 zarc_health_snapshot health;
 zarc_observation observation;
 if(!zarc_health_check(&r->health,now,&health)||
    !zarc_controller_snapshot(&r->controller,r->config.endpoint.id,&observation))return false;
 bool ready=health.required_healthy&&zarc_composition_required_ready(&r->composition)&&
            observation.quality==ZARC_QUALITY_VALID;
 if(!ready&&(!r->reported_health||r->healthy)){
  const zarc_fault fault={.version=ZARC_DIAG_VERSION,.code=1,.service=2,.epoch=r->controller.link.boot,
   .generation=observation.generation,.timestamp=now,.config_digest=r->config.config_digest,
   .build_digest=r->config.build_digest,.quality=ZARC_DIAG_FAULT,.severity=ZARC_DIAG_ERROR};
  (void)zarc_diag_record(&r->diagnostics,&fault);
 }
 r->healthy=ready;r->reported_health=true;
 return zarc_controller_ready(&r->controller,r->config.endpoint.id,ready)&&
        zarc_observation_stream_sync(&r->observations,&r->controller);
}
bool zarc_runtime_step(zarc_runtime *r,uint64_t now,bool present,uint64_t sequence,uint64_t sample_tick,zarc_value value){
 if(!r||!r->running||!zarc_controller_tick(&r->controller,now))return false;
 zarc_release release;
 if(!zarc_periodic_poll(&r->cycle,now,&release))return false;
 if(release.due){
  bool valid=present&&zarc_controller_feedback(&r->controller,r->config.endpoint.id,
        r->controller.link.boot,sequence,sample_tick,now,value);
  bool on_time=false;
  if(!zarc_periodic_complete(&r->cycle,release.release,now,&on_time)||r->health_sequence==UINT64_MAX)return false;
  if(!zarc_health_report(&r->health,2,r->controller.link.boot,++r->health_sequence,now,
      valid&&on_time,valid&&on_time&&value!=ZARC_FAULT))return false;
 }
 return update_health(r,now);
}
zarc_conn_status zarc_runtime_local(zarc_runtime *r,zarc_value value,bool authorized,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *effect){
 if(!effect)return ZARC_CONN_INVALID;
 *effect=(zarc_controller_effect){0};
 if(!r||!r->running||!zarc_controller_tick(&r->controller,now)||!update_health(r,now))return ZARC_CONN_INVALID;
 if(!r->healthy)return ZARC_CONN_UNAVAILABLE;
 return zarc_controller_local(&r->controller,r->config.endpoint.id,value,authorized,now,validity,reserved,effect);
}
bool zarc_runtime_release(zarc_runtime *r,const zarc_controller_effect *effect,uint64_t now){
 return r&&r->running&&zarc_controller_effect_released(&r->controller,effect,now)&&update_health(r,now);
}
bool zarc_runtime_stop(zarc_runtime *r,uint64_t now,uint64_t deadline){
 if(!r||!r->running||now>=deadline||!zarc_controller_tick(&r->controller,now)||
    r->controller.endpoints[0].domain.pending||r->controller.endpoints[0].effect_outstanding)return false;
 if(zarc_composition_quiesce(&r->composition,now,deadline)!=ZARC_COMPOSITION_OK||
    zarc_composition_stop(&r->composition)!=ZARC_COMPOSITION_OK)return false;
 r->running=false;return true;
}
