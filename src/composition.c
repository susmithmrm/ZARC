#include "zarc/composition.h"

static size_t lookup(const zarc_composition *c, uint32_t id) {
 for (size_t i=0;i<c->count;++i) if(c->services[i].id==id) return i;
 return c->count;
}
static bool dependencies_ready(const zarc_composition *c,size_t i) {
 const zarc_service_descriptor *s=&c->services[i];
 for(size_t j=0;j<s->dependency_count;++j)
  if(c->states[lookup(c,s->dependencies[j])]!=ZARC_SERVICE_READY) return false;
 return true;
}
zarc_composition_status zarc_composition_configure(zarc_composition *c,const zarc_service_descriptor *s,size_t n) {
 if(!c||!s||!n||n>ZARC_SERVICE_LIMIT) return ZARC_COMPOSITION_INVALID;
 if(c->active) return ZARC_COMPOSITION_ACTIVE;
 zarc_composition next={0}; next.count=n;
 for(size_t i=0;i<n;++i) {
  if(!s[i].id||s[i].dependency_count>ZARC_DEPENDENCY_LIMIT||!s[i].init||!s[i].start||
     !s[i].release||!s[i].quiesce||!s[i].suspend||!s[i].resume) return ZARC_COMPOSITION_INVALID;
  for(size_t j=0;j<i;++j) if(s[i].id==s[j].id) return ZARC_COMPOSITION_DUPLICATE;
  next.services[i]=s[i];
 }
 bool ordered[ZARC_SERVICE_LIMIT]={0};
 for(size_t i=0;i<n;++i) for(size_t j=0;j<s[i].dependency_count;++j) {
  if(lookup(&next,s[i].dependencies[j])==n) return ZARC_COMPOSITION_MISSING_DEPENDENCY;
  for(size_t k=0;k<j;++k) if(s[i].dependencies[k]==s[i].dependencies[j]) return ZARC_COMPOSITION_DUPLICATE;
 }
 for(size_t pos=0;pos<n;++pos) {
  bool found=false;
  for(size_t i=0;i<n;++i) {
   if(ordered[i]) continue;
   bool ready=true;
   for(size_t j=0;j<s[i].dependency_count;++j)
    if(!ordered[lookup(&next,s[i].dependencies[j])]) ready=false;
   if(ready) {ordered[i]=true;next.order[pos]=i;found=true;break;}
  }
  if(!found) return ZARC_COMPOSITION_CYCLE;
 }
 next.configured=true;*c=next;return ZARC_COMPOSITION_OK;
}
static void release(zarc_composition *c,size_t i) {
 if(c->acquired[i]) {c->services[i].release(c->services[i].context);c->acquired[i]=false;}
}
zarc_composition_status zarc_composition_start(zarc_composition *c) {
 if(!c||!c->configured) return ZARC_COMPOSITION_INVALID;
 if(c->active) return ZARC_COMPOSITION_ACTIVE;
 bool degraded=false;c->active=true;
 for(size_t p=0;p<c->count;++p) {
  size_t i=c->order[p];zarc_service_descriptor *s=&c->services[i];
  if(!dependencies_ready(c,i)) {c->states[i]=ZARC_SERVICE_BLOCKED;degraded=true;continue;}
  if(!s->init(s->context)) {c->states[i]=ZARC_SERVICE_FAILED;degraded=true;continue;}
  c->acquired[i]=true;
  if(!s->start(s->context)) {release(c,i);c->states[i]=ZARC_SERVICE_FAILED;degraded=true;continue;}
  c->states[i]=ZARC_SERVICE_READY;
 }
 return !zarc_composition_required_ready(c)?ZARC_COMPOSITION_UNAVAILABLE:
  degraded?ZARC_COMPOSITION_DEGRADED:ZARC_COMPOSITION_OK;
}
/* On failure, do not quiesce dependencies below a still-active dependent.
 * Admission closes for every service before callbacks begin. */
zarc_composition_status zarc_composition_quiesce(zarc_composition *c,uint64_t now,uint64_t deadline) {
 if(!c||!c->active||now>=deadline) return ZARC_COMPOSITION_INVALID;
 for(size_t i=0;i<c->count;++i)
  if(c->states[i]==ZARC_SERVICE_READY) c->states[i]=ZARC_SERVICE_QUIESCED;
 for(size_t p=c->count;p>0;--p) {
  size_t i=c->order[p-1];
  if(c->states[i]==ZARC_SERVICE_QUIESCED && !c->services[i].quiesce(c->services[i].context,deadline)) {
   c->states[i]=ZARC_SERVICE_FAILED;
   /* Remaining callbacks must not be skipped on a retry and mistaken for
    * drained work. Mark unprocessed services failed as well. */
   for(size_t q=0;q<p-1;++q) {
    size_t j=c->order[q];if(c->states[j]==ZARC_SERVICE_QUIESCED)c->states[j]=ZARC_SERVICE_FAILED;
   }
   return ZARC_COMPOSITION_DEGRADED;
  }
 }
 return ZARC_COMPOSITION_OK;
}
zarc_composition_status zarc_composition_suspend(zarc_composition *c) {
 if(!c||!c->active) return ZARC_COMPOSITION_INVALID;
 for(size_t i=0;i<c->count;++i) if(c->acquired[i]&&c->states[i]!=ZARC_SERVICE_QUIESCED) return ZARC_COMPOSITION_INVALID;
 for(size_t p=c->count;p>0;--p) {
  size_t i=c->order[p-1];if(!c->acquired[i])continue;
  if(!c->services[i].suspend(c->services[i].context)){c->states[i]=ZARC_SERVICE_FAILED;return ZARC_COMPOSITION_DEGRADED;}
  c->states[i]=ZARC_SERVICE_SUSPENDED;
 }
 return ZARC_COMPOSITION_OK;
}
zarc_composition_status zarc_composition_resume(zarc_composition *c) {
 if(!c||!c->active) return ZARC_COMPOSITION_INVALID;
 for(size_t i=0;i<c->count;++i) if(c->acquired[i]&&c->states[i]!=ZARC_SERVICE_SUSPENDED) return ZARC_COMPOSITION_INVALID;
 bool degraded=false;
 for(size_t p=0;p<c->count;++p) {
  size_t i=c->order[p];if(!c->acquired[i])continue;
  if(!dependencies_ready(c,i)){c->states[i]=ZARC_SERVICE_BLOCKED;degraded=true;continue;}
  if(!c->services[i].resume(c->services[i].context)){c->states[i]=ZARC_SERVICE_FAILED;degraded=true;continue;}
  c->states[i]=ZARC_SERVICE_READY;
 }
 return !zarc_composition_required_ready(c)?ZARC_COMPOSITION_UNAVAILABLE:
  degraded?ZARC_COMPOSITION_DEGRADED:ZARC_COMPOSITION_OK;
}
zarc_composition_status zarc_composition_stop(zarc_composition *c) {
 if(!c||!c->configured) return ZARC_COMPOSITION_INVALID;
 for(size_t i=0;i<c->count;++i) if(c->acquired[i]&&c->states[i]==ZARC_SERVICE_READY) return ZARC_COMPOSITION_INVALID;
 for(size_t p=c->count;p>0;--p){size_t i=c->order[p-1];release(c,i);c->states[i]=ZARC_SERVICE_STOPPED;}
 c->active=false;return ZARC_COMPOSITION_OK;
}
bool zarc_composition_state(const zarc_composition *c,uint32_t id,zarc_service_state *out) {
 if(!c||!c->configured||!out) return false;
 size_t i=lookup(c,id);if(i==c->count)return false;*out=c->states[i];return true;
}
bool zarc_composition_admits(const zarc_composition *c,uint32_t id) {
 zarc_service_state s;return zarc_composition_state(c,id,&s)&&s==ZARC_SERVICE_READY;
}


bool zarc_composition_required_ready(const zarc_composition *c){
 if(!c||!c->configured||!c->active)return false;
 for(size_t i=0;i<c->count;++i)
  if(c->services[i].required&&c->states[i]!=ZARC_SERVICE_READY)return false;
 return true;
}
