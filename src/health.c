#include "zarc/health.h"
bool zarc_health_init(zarc_health *h,const zarc_health_policy *p,size_t n,uint64_t epoch,uint64_t now){
 if(!h||!p||!n||n>ZARC_HEALTH_LIMIT||!epoch)return false;
 zarc_health x={0};x.count=n;x.epoch=epoch;x.last_tick=now;
 bool required=false;
 for(size_t i=0;i<n;++i){
  if(!p[i].id||!p[i].alive_limit||!p[i].deadline_limit)return false;
  for(size_t j=0;j<i;++j)if(p[i].id==p[j].id)return false;
  x.entries[i].policy=p[i];required=required||p[i].required;
 }
 if(!required)return false;
 x.configured=true;*h=x;return true;
}
bool zarc_health_report(zarc_health *h,uint32_t id,uint64_t epoch,uint64_t sequence,uint64_t now,bool completed,bool logical){
 if(!h||!h->configured||epoch!=h->epoch||now<h->last_tick||!sequence)return false;
 for(size_t i=0;i<h->count;++i){
  zarc_health_entry *e=&h->entries[i];if(e->policy.id!=id)continue;
  if(e->seen&&sequence<=e->sequence)return false;
  e->seen=true;e->sequence=sequence;e->last_report=now;e->logical=logical;
  if(completed){e->completed=true;e->last_completion=now;}
  h->last_tick=now;return true;
 }
 return false;
}
bool zarc_health_check(zarc_health *h,uint64_t now,zarc_health_snapshot *out){
 if(!h||!h->configured||!out||now<h->last_tick)return false;
 zarc_health_snapshot s={0};s.required_healthy=true;
 for(size_t i=0;i<h->count;++i){
  const zarc_health_entry *e=&h->entries[i];uint32_t bit=UINT32_C(1)<<i;
  bool a=!e->seen||now-e->last_report>=e->policy.alive_limit;
  bool d=!e->completed||now-e->last_completion>=e->policy.deadline_limit;
  bool l=!e->seen||!e->logical;
  if(a)s.alive_failed|=bit;
  if(d)s.deadline_failed|=bit;
  if(l)s.logical_failed|=bit;
  if(e->policy.required&&(a||d||l))s.required_healthy=false;
 }
 h->last_tick=now;*out=s;return true;
}
