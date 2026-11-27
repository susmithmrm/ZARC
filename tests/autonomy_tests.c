#include "zarc/controller.h"
#include "zarc/schedule.h"
#include "zarc/traffic.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_controller c;const zarc_endpoint_config p[]={{1,50},{2,50}};
 zarc_controller_effect e,first,second;zarc_observation o;
 CHECK(zarc_controller_init(&c,p,2,1,100,0));
 CHECK(zarc_controller_ready(&c,1,true)&&zarc_controller_ready(&c,2,true));
 CHECK(zarc_controller_local(&c,1,ZARC_ON,false,1,10,true,&e)==ZARC_CONN_DENIED);
 CHECK(zarc_controller_local(&c,1,ZARC_ON,true,1,10,false,&e)==ZARC_CONN_BUSY);
 CHECK(zarc_controller_local(&c,1,ZARC_ON,true,1,10,true,&e)==ZARC_CONN_ACCEPTED&&e.emitted);
 first=e;
 CHECK(zarc_controller_local(&c,1,ZARC_OFF,true,2,10,true,&e)==ZARC_CONN_BUSY);
 CHECK(zarc_controller_local(&c,2,ZARC_OFF,true,2,10,true,&e)==ZARC_CONN_ACCEPTED);
 second=e;
 CHECK(zarc_controller_feedback(&c,2,1,1,3,3,ZARC_OFF));
 CHECK(zarc_controller_feedback(&c,1,1,1,4,4,ZARC_ON));
 CHECK(zarc_controller_effect_released(&c,&first,4));
 CHECK(zarc_controller_effect_released(&c,&second,4));
 CHECK(zarc_controller_snapshot(&c,1,&o)&&o.value==ZARC_ON&&o.quality==ZARC_QUALITY_VALID);
 CHECK(!c.link.connected&&c.link.count==0);
 CHECK(zarc_controller_open(&c,1,1,1,0,5)==ZARC_CONN_COMPLETED);
 zarc_intent q={.session=1,.request_id=1,.domain=1,.desired=ZARC_OFF,.operation=ZARC_OP_DOMAIN,.validity=10};
 CHECK(zarc_controller_request(&c,&q,true,true,0,5,true,&e)==ZARC_CONN_ACCEPTED);
 CHECK(zarc_controller_local(&c,1,ZARC_ON,true,6,10,true,&e)==ZARC_CONN_BUSY);
 CHECK(zarc_controller_cancel(&c,1,1,1,false,true,6)==ZARC_CANCEL_DENIED);
 CHECK(zarc_controller_cancel(&c,1,1,1,true,false,6)==ZARC_CANCEL_IN_FLIGHT);
 CHECK(zarc_controller_cancel(&c,1,1,1,true,true,6)==ZARC_CANCEL_REMOVED);
 CHECK(zarc_controller_snapshot(&c,1,&o)&&o.value==ZARC_ON);
 CHECK(zarc_link_result(&c.link,1,ZARC_OP_DOMAIN,1,6)==ZARC_CONN_UNKNOWN_OUTCOME);
 CHECK(zarc_controller_local(&c,1,ZARC_OFF,true,7,10,true,&e)==ZARC_CONN_ACCEPTED);
 CHECK(zarc_controller_feedback(&c,1,1,2,8,8,ZARC_OFF));
 CHECK(zarc_controller_effect_released(&c,&e,8));
 zarc_periodic s;zarc_release r;bool on_time;
 CHECK(zarc_periodic_init(&s,10,10,8));
 CHECK(zarc_periodic_poll(&s,9,&r)&&!r.due);
 CHECK(zarc_periodic_poll(&s,10,&r)&&r.due&&r.deadline==18);
 CHECK(zarc_periodic_complete(&s,10,12,&on_time)&&on_time);
 CHECK(zarc_periodic_poll(&s,34,&r)&&r.due&&r.release==30&&r.skipped==1);
 CHECK(zarc_periodic_poll(&s,40,&r)&&r.due&&s.missed==1);
 CHECK(!zarc_periodic_complete(&s,30,41,&on_time)&&s.pending);
 CHECK(zarc_periodic_complete(&s,40,48,&on_time)&&!on_time&&s.missed==2);
 CHECK(!zarc_periodic_poll(&s,47,&r));
 zarc_traffic t={0};zarc_frame f={.operation=ZARC_OP_EVENT},out;bool core;
 for(uint64_t i=0;i<ZARC_BULK_QUEUE;++i){f.request_id=i;CHECK(zarc_traffic_push(&t,&f,false));}
 CHECK(!zarc_traffic_push(&t,&f,false)&&t.bulk_rejected==1);
 f.request_id=99;CHECK(zarc_traffic_push(&t,&f,true));
 CHECK(zarc_traffic_pop(&t,&out,&core)&&core&&out.request_id==99);
 CHECK(zarc_traffic_pop(&t,&out,&core)&&!core&&out.request_id==0);
 CHECK(zarc_traffic_pop(&t,&out,&core)&&!core&&out.request_id==1);
 CHECK(!zarc_traffic_pop(&t,&out,&core));
 puts("Autonomy, cancellation, timing and traffic contracts passed");return 0;
}
