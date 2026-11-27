#include "zarc/observations.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_controller c;zarc_endpoint_config p[]={{1,4},{2,4}};
 CHECK(zarc_controller_init(&c,p,2,7,20,0));
 zarc_observation_stream s;zarc_observation o;uint64_t cursor;zarc_observation_event event;
 CHECK(zarc_observation_stream_init(&s,&c));
 CHECK(zarc_observation_barrier(&s,1,&o,&cursor)&&cursor==0&&o.quality==ZARC_QUALITY_UNKNOWN);
 CHECK(zarc_controller_ready(&c,1,true));
 CHECK(zarc_controller_feedback(&c,1,7,1,1,1,ZARC_OFF));
 CHECK(zarc_observation_stream_sync(&s,&c));
 CHECK(zarc_observation_next(&s,7,cursor,&event)==ZARC_EVENT_AVAILABLE&&event.sequence==1&&event.observation.value==ZARC_OFF);
 cursor=event.sequence;
 CHECK(zarc_controller_feedback(&c,2,7,1,2,2,ZARC_ON));
 CHECK(zarc_observation_stream_sync(&s,&c));
 CHECK(zarc_observation_next(&s,7,cursor,&event)==ZARC_EVENT_AVAILABLE&&event.observation.id==2);
 CHECK(zarc_controller_tick(&c,6)&&zarc_observation_stream_sync(&s,&c));
 CHECK(zarc_observation_barrier(&s,1,&o,&cursor)&&o.quality==ZARC_QUALITY_STALE);
 CHECK(zarc_observation_next(&s,7,cursor,&event)==ZARC_EVENT_EMPTY);
 uint64_t old=cursor;
 for(uint64_t i=2;i<12;++i){
  CHECK(zarc_controller_feedback(&c,1,7,i,i+6,i+6,i%2?ZARC_ON:ZARC_OFF));
  CHECK(zarc_observation_stream_sync(&s,&c));
 }
 CHECK(zarc_observation_next(&s,7,old,&event)==ZARC_EVENT_GAP);
 CHECK(zarc_observation_barrier(&s,1,&o,&cursor));
 CHECK(zarc_observation_next(&s,8,cursor,&event)==ZARC_EVENT_GAP);
 CHECK(zarc_observation_next(&s,7,cursor,&event)==ZARC_EVENT_EMPTY);
 puts("Observation barrier and gap contracts passed");return 0;
}
