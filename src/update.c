#include "zarc/update.h"
#include <stddef.h>
static bool compatible(const zarc_update_policy *p,const zarc_image *i){
 return i->image_id&&i->version>=p->minimum_version&&i->target==p->target&&i->contract==p->contract&&i->config_schema==p->config_schema;
}
static bool same(const zarc_image *a,const zarc_image *b){
 return a->image_id==b->image_id&&a->version==b->version&&a->target==b->target&&a->contract==b->contract&&a->config_schema==b->config_schema;
}
bool zarc_update_state_valid(const zarc_update *u){
 if(!u||!u->configured||!u->policy.target||!u->policy.contract||!u->policy.config_schema||
    !compatible(&u->policy,&u->active)||(unsigned)u->phase>ZARC_UPDATE_CONFIRMING)return false;
 const zarc_image *c=&u->candidate;
 bool empty=!c->image_id&&!c->version&&!c->target&&!c->contract&&!c->config_schema;
 if(u->phase==ZARC_UPDATE_ACTIVE)return empty;
 if(empty)return u->phase==ZARC_UPDATE_RECOVERY_REQUIRED||u->phase==ZARC_UPDATE_UNAVAILABLE;
 return compatible(&u->policy,c)&&c->image_id!=u->active.image_id&&c->version>u->active.version;
}
bool zarc_update_init(zarc_update *u,const zarc_update_policy *p,const zarc_image *i,bool verified){
 if(!u||!p||!i||!p->target||!p->contract||!p->config_schema||!verified||!compatible(p,i))return false;
 *u=(zarc_update){.policy=*p,.active=*i,.phase=ZARC_UPDATE_ACTIVE,.configured=true};return true;
}
zarc_update_status zarc_update_stage(zarc_update *u,const zarc_image *i,bool verified,bool eligible,bool recovery){
 if(!u||!u->configured||!i)return ZARC_UPDATE_INVALID;
 if(u->phase!=ZARC_UPDATE_ACTIVE)return ZARC_UPDATE_CONFLICT;
 if(!verified||!eligible||!recovery||!compatible(&u->policy,i)||i->version<=u->active.version||i->image_id==u->active.image_id)return ZARC_UPDATE_DENIED;
 u->candidate=*i;u->phase=ZARC_UPDATE_STAGED;return ZARC_UPDATE_OK;
}
zarc_update_status zarc_update_trial_request(zarc_update *u,bool eligible){
 if(!u||!u->configured)return ZARC_UPDATE_INVALID;
 if(u->phase!=ZARC_UPDATE_STAGED)return ZARC_UPDATE_CONFLICT;
 if(!eligible)return ZARC_UPDATE_DENIED;
 u->phase=ZARC_UPDATE_TRIAL_REQUESTED;return ZARC_UPDATE_OK;
}
zarc_update_status zarc_update_boot_report(zarc_update *u,const zarc_image *i,bool verified){
 if(!u||!u->configured||!i)return ZARC_UPDATE_INVALID;
 if(!verified||!compatible(&u->policy,i)){u->phase=ZARC_UPDATE_UNAVAILABLE;return ZARC_UPDATE_DENIED;}
 if(u->phase==ZARC_UPDATE_TRIAL_REQUESTED&&same(i,&u->candidate)){
  u->phase=ZARC_UPDATE_TRIAL;return ZARC_UPDATE_OK;
 }
 if(same(i,&u->active)){
  u->candidate=(zarc_image){0};u->phase=ZARC_UPDATE_ACTIVE;return ZARC_UPDATE_OK;
 }
 u->phase=ZARC_UPDATE_UNAVAILABLE;return ZARC_UPDATE_DENIED;
}
zarc_update_status zarc_update_confirm(zarc_update *u,bool healthy,bool compatible_pair,bool eligible){
 if(!u||!u->configured)return ZARC_UPDATE_INVALID;
 if(u->phase!=ZARC_UPDATE_TRIAL)return ZARC_UPDATE_CONFLICT;
 if(!healthy||!compatible_pair||!eligible){u->phase=ZARC_UPDATE_RECOVERY_REQUIRED;return ZARC_UPDATE_DENIED;}
 u->active=u->candidate;u->candidate=(zarc_image){0};u->phase=ZARC_UPDATE_ACTIVE;return ZARC_UPDATE_OK;
}
zarc_update_status zarc_update_prepare_confirmation(zarc_update *u,bool healthy,bool compatible_pair,bool eligible){
 if(!u||!u->configured)return ZARC_UPDATE_INVALID;
 if(u->phase!=ZARC_UPDATE_TRIAL)return ZARC_UPDATE_CONFLICT;
 if(!healthy||!compatible_pair||!eligible){u->phase=ZARC_UPDATE_RECOVERY_REQUIRED;return ZARC_UPDATE_DENIED;}
 u->phase=ZARC_UPDATE_CONFIRMING;return ZARC_UPDATE_OK;
}
zarc_update_status zarc_update_reconcile_confirmation(zarc_update *u,const zarc_image *running,bool verified,bool confirmed){
 if(!u||!u->configured||!running)return ZARC_UPDATE_INVALID;
 if(u->phase!=ZARC_UPDATE_CONFIRMING)return ZARC_UPDATE_CONFLICT;
 if(!verified||!confirmed||!compatible(&u->policy,running)||!same(running,&u->candidate)){
  u->phase=ZARC_UPDATE_RECOVERY_REQUIRED;return ZARC_UPDATE_DENIED;
 }
 u->active=*running;u->candidate=(zarc_image){0};u->phase=ZARC_UPDATE_ACTIVE;return ZARC_UPDATE_OK;
}
zarc_update_status zarc_update_recover_boot(zarc_update *u,const zarc_image *running,bool verified,bool confirmed){
 if(!u||!u->configured||!running)return ZARC_UPDATE_INVALID;
 if(!verified||!compatible(&u->policy,running)){u->phase=ZARC_UPDATE_UNAVAILABLE;return ZARC_UPDATE_DENIED;}
 if(u->phase==ZARC_UPDATE_CONFIRMING&&same(running,&u->candidate))
  return zarc_update_reconcile_confirmation(u,running,true,confirmed);
 if(confirmed&&same(running,&u->active)){
  /* Retain staged metadata when no trial was requested. Otherwise a
   * confirmed previous image establishes the old active state. */
  if(u->phase!=ZARC_UPDATE_STAGED){u->candidate=(zarc_image){0};u->phase=ZARC_UPDATE_ACTIVE;}
  return ZARC_UPDATE_OK;
 }
 if(!confirmed&&u->phase==ZARC_UPDATE_TRIAL_REQUESTED&&same(running,&u->candidate)){
  u->phase=ZARC_UPDATE_TRIAL;return ZARC_UPDATE_OK;
 }
 /* Unexpected confirmed candidate without durable confirmation intent,
  * repeated unconfirmed trial, unknown or unconfirmed old image: no admission. */
 u->phase=ZARC_UPDATE_RECOVERY_REQUIRED;return ZARC_UPDATE_DENIED;
}
void zarc_update_interrupt(zarc_update *u){
 if(!u||!u->configured)return;
 switch(u->phase){
 case ZARC_UPDATE_ACTIVE:case ZARC_UPDATE_STAGED:case ZARC_UPDATE_TRIAL_REQUESTED:case ZARC_UPDATE_TRIAL:
  u->phase=ZARC_UPDATE_RECOVERY_REQUIRED;break;
 default:break;
 }
}
