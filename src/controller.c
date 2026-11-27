#include "zarc/controller.h"
static zarc_endpoint *find_endpoint(zarc_controller *c,uint32_t id){
 for(size_t i=0;i<c->count;++i)if(c->endpoints[i].config.id==id)return &c->endpoints[i];
 return NULL;
}
bool zarc_controller_init(zarc_controller *c,const zarc_endpoint_config *p,size_t n,uint64_t boot,uint64_t max_validity,uint64_t now){
 if(!c||!p||!n||n>ZARC_CONTROLLER_DOMAINS)return false;
 zarc_controller next={0};next.count=n;next.last_tick=now;
 if(!zarc_link_init(&next.link,boot,max_validity,now))return false;
 for(size_t i=0;i<n;++i){
  if(!p[i].id||!p[i].feedback_max_age)return false;
  for(size_t j=0;j<i;++j)if(p[j].id==p[i].id)return false;
  next.endpoints[i].config=p[i];zarc_init(&next.endpoints[i].domain);next.endpoints[i].domain.last_tick=now;
 }
 *c=next;return true;
}
bool zarc_controller_ready(zarc_controller *c,uint32_t id,bool ready){
 if(!c||!c->link.boot)return false;
 zarc_endpoint *e=find_endpoint(c,id);if(!e)return false;
 e->ready=ready;return true;
}
static void reconcile(zarc_controller *c,zarc_endpoint *e,uint64_t now){
 if(e->correlated&&!e->domain.pending){
  (void)zarc_link_finish(&c->link,e->session,ZARC_OP_DOMAIN,e->request_id,now,e->domain.result==ZARC_COMPLETED);
  e->correlated=false;
 }
}
bool zarc_controller_tick(zarc_controller *c,uint64_t now){
 if(!c||!c->link.boot||now<c->last_tick)return false;
 if(!zarc_link_tick(&c->link,now))return false;
 for(size_t i=0;i<c->count;++i){zarc_endpoint *e=&c->endpoints[i];
  (void)zarc_tick(&e->domain,now);reconcile(c,e,now);
 }
 c->last_tick=now;return true;
}
zarc_conn_status zarc_controller_open(zarc_controller *c,uint64_t session,uint64_t principal,uint8_t major,uint8_t minor,uint64_t now){
 if(!zarc_controller_tick(c,now))return ZARC_CONN_INVALID;
 return zarc_link_open(&c->link,session,principal,major,minor,now);
}
zarc_conn_status zarc_controller_renew(zarc_controller *c,uint64_t session,uint64_t principal,uint64_t now){
 if(!c||!c->link.connected||c->link.session!=session||c->link.principal!=principal)return ZARC_CONN_UNAVAILABLE;
 if(!zarc_controller_tick(c,now))return ZARC_CONN_INVALID;
 if(session==UINT64_MAX)return ZARC_CONN_UNAVAILABLE;
 for(size_t i=0;i<c->count;++i)
  if(c->endpoints[i].effect_outstanding||c->endpoints[i].domain.pending)return ZARC_CONN_BUSY;
 zarc_link next=c->link;zarc_link_disconnect(&next);
 zarc_conn_status status=zarc_link_open(&next,session+1u,principal,ZARC_CONNECT_MAJOR,ZARC_CONNECT_MINOR,now);
 if(status==ZARC_CONN_COMPLETED)c->link=next;
 return status;
}
void zarc_controller_disconnect(zarc_controller *c){if(c)zarc_link_disconnect(&c->link);}
zarc_conn_status zarc_controller_request(zarc_controller *c,const zarc_intent *q,bool allowed,bool age_known,uint64_t age,uint64_t now,bool reserved,zarc_controller_effect *effect){
 if(!effect)return ZARC_CONN_INVALID;
 *effect=(zarc_controller_effect){0};
 if(!c||!q||q->operation!=ZARC_OP_DOMAIN||(q->desired!=ZARC_OFF&&q->desired!=ZARC_ON)||!zarc_controller_tick(c,now))return ZARC_CONN_INVALID;
 zarc_endpoint *e=find_endpoint(c,q->domain);if(!e)return ZARC_CONN_INVALID;
 /* Stage the ledger and domain transition. No accepted result survives a
  * failed effect reservation or busy domain. Retained retries need no slot. */
 zarc_link candidate=c->link;bool fresh=false;
 zarc_conn_status status=zarc_link_admit(&candidate,q,allowed,age_known,age,now,&fresh);
 if(status!=ZARC_CONN_ACCEPTED||!fresh)return status;
 if(!e->ready)return ZARC_CONN_UNAVAILABLE;
 if(!reserved||e->domain.pending||e->effect_outstanding)return ZARC_CONN_BUSY;
 if(e->internal_id==UINT64_MAX)return ZARC_CONN_UNAVAILABLE;
 zarc_domain next=e->domain;zarc_effect output;
 uint64_t deadline=candidate.entries[candidate.count-1u].deadline;
 if(zarc_request(&next,true,e->internal_id+1u,(zarc_value)q->desired,now,deadline,&output)!=ZARC_ACCEPTED||!output.emitted)return ZARC_CONN_INVALID;
 c->link=candidate;e->domain=next;++e->internal_id;e->admitted_tick=now;e->request_id=q->request_id;e->session=q->session;e->correlated=true;e->effect_outstanding=true;
 *effect=(zarc_controller_effect){.id=q->domain,.desired=output.desired,.emitted=true,.boot=c->link.boot,.token=e->internal_id,.deadline=deadline};return ZARC_CONN_ACCEPTED;
}
bool zarc_controller_feedback(zarc_controller *c,uint32_t id,uint64_t boot,uint64_t seq,uint64_t sample,uint64_t now,zarc_value value){
 if(!c||boot!=c->link.boot||!seq||sample>now||now<c->last_tick||
 (value!=ZARC_OFF&&value!=ZARC_ON&&value!=ZARC_FAULT))return false;
 zarc_endpoint *e=find_endpoint(c,id);if(!e||now-sample>=e->config.feedback_max_age)return false;
 if(e->source_sequence&&(seq<=e->source_sequence||sample<e->source_tick))return false;
 if(!zarc_controller_tick(c,now))return false;
 if(e->domain.pending&&sample<e->admitted_tick){
  /* Delayed pre-admission observation may update the snapshot, but cannot
   * confirm a new command. Fault observations still fail the pending work. */
  if(value==ZARC_FAULT)(void)zarc_feedback(&e->domain,value,now);
  else if(e->domain.observed!=value){e->domain.observed=value;++e->domain.generation;}
 }else (void)zarc_feedback(&e->domain,value,now);
 e->observed=true;e->source_sequence=seq;e->source_tick=sample;reconcile(c,e,now);return true;
}
bool zarc_controller_snapshot(const zarc_controller *c,uint32_t id,zarc_observation *out){
 if(!c||!out||!c->link.boot)return false;
 for(size_t i=0;i<c->count;++i){
  const zarc_endpoint *e=&c->endpoints[i];if(e->config.id!=id)continue;
  zarc_quality q=ZARC_QUALITY_UNKNOWN;
  if(e->observed&&e->domain.observed!=ZARC_UNKNOWN){
   if(e->domain.observed==ZARC_FAULT)q=ZARC_QUALITY_FAULT;
   else if(c->last_tick-e->source_tick>=e->config.feedback_max_age)q=ZARC_QUALITY_STALE;
   else q=ZARC_QUALITY_VALID;
  }
  *out=(zarc_observation){.id=id,.value=e->domain.observed,.quality=q,.boot=c->link.boot,.generation=e->domain.generation,.sample_tick=e->source_tick,.source_sequence=e->source_sequence,.ready=e->ready};return true;
 }
 return false;
}

zarc_conn_status zarc_controller_local(zarc_controller *c,uint32_t id,zarc_value desired,bool allowed,uint64_t now,uint64_t validity,bool reserved,zarc_controller_effect *effect){
 if(!effect)return ZARC_CONN_INVALID;
 *effect=(zarc_controller_effect){0};
 if(!c||(desired!=ZARC_OFF&&desired!=ZARC_ON)||!validity||validity>c->link.max_validity||now>UINT64_MAX-validity||!zarc_controller_tick(c,now))return ZARC_CONN_INVALID;
 zarc_endpoint *e=find_endpoint(c,id);if(!e)return ZARC_CONN_INVALID;
 if(!allowed)return ZARC_CONN_DENIED;
 if(!e->ready)return ZARC_CONN_UNAVAILABLE;
 if(!reserved||e->domain.pending||e->effect_outstanding)return ZARC_CONN_BUSY;
 if(e->internal_id==UINT64_MAX)return ZARC_CONN_UNAVAILABLE;
 zarc_domain next=e->domain;zarc_effect output;
 if(zarc_request(&next,true,e->internal_id+1u,desired,now,now+validity,&output)!=ZARC_ACCEPTED||!output.emitted)return ZARC_CONN_INVALID;
 e->domain=next;++e->internal_id;e->admitted_tick=now;e->correlated=false;e->effect_outstanding=true;
 *effect=(zarc_controller_effect){.id=id,.desired=output.desired,.emitted=true,.boot=c->link.boot,.token=e->internal_id,.deadline=now+validity};return ZARC_CONN_ACCEPTED;
}
zarc_cancel_status zarc_controller_cancel(zarc_controller *c,uint32_t id,uint64_t session,uint64_t request_id,bool authorized,bool removed,uint64_t now){
 if(!authorized)return ZARC_CANCEL_DENIED;
 if(!c||!c->link.connected||session!=c->link.session||!request_id||!zarc_controller_tick(c,now))return ZARC_CANCEL_UNKNOWN;
 zarc_endpoint *e=find_endpoint(c,id);if(!e)return ZARC_CANCEL_UNKNOWN;
 for(size_t i=0;i<c->link.count;++i){
  const zarc_request_entry *r=&c->link.entries[i];
  if(r->intent.session!=session||r->intent.request_id!=request_id||r->intent.operation!=ZARC_OP_DOMAIN||r->intent.domain!=id)continue;
  if(r->status!=ZARC_CONN_ACCEPTED)return ZARC_CANCEL_TERMINAL;
  if(!e->correlated||!e->domain.pending||e->request_id!=request_id||e->session!=session)return ZARC_CANCEL_UNKNOWN;
  if(!removed)return ZARC_CANCEL_IN_FLIGHT;
  e->effect_outstanding=false;e->domain.pending=false;e->domain.result=ZARC_UNKNOWN_OUTCOME;reconcile(c,e,now);return ZARC_CANCEL_REMOVED;
 }
 return ZARC_CANCEL_UNKNOWN;
}

bool zarc_controller_effect_released(zarc_controller *c,const zarc_controller_effect *effect,uint64_t now){
 if(!c||!effect||!effect->emitted||effect->boot!=c->link.boot||!effect->token||now<c->last_tick)return false;
 zarc_endpoint *e=find_endpoint(c,effect->id);
 if(!e||!e->effect_outstanding||effect->token!=e->internal_id||effect->deadline!=e->domain.deadline||
    effect->desired!=e->domain.desired)return false;
 if(!zarc_controller_tick(c,now))return false;
 e->effect_outstanding=false;return true;
}
