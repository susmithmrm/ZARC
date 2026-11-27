#include "observation_fixture.h"
#include "observation_bus.h"
static bool same(const zarc_observation *a,const zarc_observation *b){
 return a->id==b->id&&a->boot==b->boot&&a->value==b->value&&a->quality==b->quality&&
 a->generation==b->generation&&a->sample_tick==b->sample_tick&&a->source_sequence==b->source_sequence&&a->ready==b->ready;
}
bool zarc_profile_observer_barrier(struct zarc_profile_observer *o,const zarc_observation_stream *s){
 if(!o||!s||!s->valid||!s->boot||!s->count||s->count>ZARC_CONTROLLER_DOMAINS)return false;
 struct zarc_profile_observer next={.boot=s->boot,.cursor=s->sequence,.count=s->count,.delivered=o->delivered};
 for(size_t i=0;i<s->count;++i){
  uint64_t cursor;
  if(!zarc_observation_barrier(s,s->current[i].id,&next.cache[i],&cursor)||cursor!=s->sequence)return false;
 }
 zarc_native_observation_reset();next.valid=true;*o=next;return true;
}
bool zarc_profile_observer_pump(struct zarc_profile_observer *o,const zarc_observation_stream *s){
 if(!o||!s||!o->valid)return false;
 for(size_t i=0;i<=ZARC_EVENT_LIMIT;++i){
  zarc_observation_event e,received;
  zarc_event_status result=zarc_observation_next(s,o->boot,o->cursor,&e);
  if(result==ZARC_EVENT_EMPTY)return true;
  if(result!=ZARC_EVENT_AVAILABLE||o->cursor==UINT64_MAX||e.sequence!=o->cursor+1u||
     zarc_native_observation_publish(&e)!=0||zarc_native_observation_read(&received)!=0||
     received.sequence!=e.sequence||!same(&e.observation,&received.observation))break;
  size_t n=0;
  while(n<o->count&&o->cache[n].id!=received.observation.id)++n;
  if(n==o->count||received.observation.boot!=o->boot||
     received.observation.generation<o->cache[n].generation)break;
  o->cache[n]=received.observation;o->cursor=received.sequence;
  if(o->delivered!=UINT64_MAX)++o->delivered;
 }
 o->valid=false;return false;
}
