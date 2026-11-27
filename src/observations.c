#include "zarc/observations.h"
static bool same(const zarc_observation *a,const zarc_observation *b){
 return a->id==b->id&&a->value==b->value&&a->quality==b->quality&&a->boot==b->boot&&
 a->generation==b->generation&&a->sample_tick==b->sample_tick&&a->source_sequence==b->source_sequence&&a->ready==b->ready;
}
bool zarc_observation_stream_init(zarc_observation_stream *s,const zarc_controller *c){
 if(!s||!c||!c->link.boot||!c->count||c->count>ZARC_CONTROLLER_DOMAINS)return false;
 zarc_observation_stream next={.boot=c->link.boot,.count=c->count,.valid=true};
 for(size_t i=0;i<c->count;++i)
  if(!zarc_controller_snapshot(c,c->endpoints[i].config.id,&next.current[i]))return false;
 *s=next;return true;
}
bool zarc_observation_stream_sync(zarc_observation_stream *s,const zarc_controller *c){
 if(!s||!c||!s->valid||s->boot!=c->link.boot||s->count!=c->count)return false;
 for(size_t i=0;i<s->count;++i){
  zarc_observation o;
  if(!zarc_controller_snapshot(c,s->current[i].id,&o)||o.generation<s->current[i].generation){s->valid=false;return false;}
  if(same(&o,&s->current[i]))continue;
  if(s->sequence==UINT64_MAX){s->valid=false;return false;}
  size_t index=(s->head+s->retained)%ZARC_EVENT_LIMIT;
  if(s->retained==ZARC_EVENT_LIMIT){index=s->head;s->head=(s->head+1u)%ZARC_EVENT_LIMIT;}
  else ++s->retained;
  s->history[index]=(zarc_observation_event){.sequence=++s->sequence,.observation=o};
  s->current[i]=o;
 }
 return true;
}
bool zarc_observation_barrier(const zarc_observation_stream *s,uint32_t id,zarc_observation *out,uint64_t *sequence){
 if(!s||!s->valid||!out||!sequence)return false;
 for(size_t i=0;i<s->count;++i)if(s->current[i].id==id){
  *out=s->current[i];*sequence=s->sequence;return true;
 }
 return false;
}
zarc_event_status zarc_observation_next(const zarc_observation_stream *s,uint64_t boot,uint64_t cursor,zarc_observation_event *out){
 if(!s||!s->valid||!out)return ZARC_EVENT_INVALID;
 if(boot!=s->boot||cursor>s->sequence)return ZARC_EVENT_GAP;
 if(cursor==s->sequence)return ZARC_EVENT_EMPTY;
 if(!s->retained||cursor<s->sequence-(uint64_t)s->retained)return ZARC_EVENT_GAP;
 size_t offset=(size_t)(cursor-(s->sequence-(uint64_t)s->retained));
 *out=s->history[(s->head+offset)%ZARC_EVENT_LIMIT];return ZARC_EVENT_AVAILABLE;
}
