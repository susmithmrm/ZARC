#include "zarc/controller.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_controller c;zarc_endpoint_config p[]={{1,20},{2,20}};
 zarc_controller_effect e,first,second;zarc_observation o;
 CHECK(zarc_controller_init(&c,p,2,9,100,0));
 CHECK(zarc_controller_ready(&c,1,true)&&zarc_controller_ready(&c,2,true));
 CHECK(zarc_controller_snapshot(&c,1,&o)&&o.quality==ZARC_QUALITY_UNKNOWN);
 CHECK(zarc_controller_feedback(&c,1,9,1,0,0,ZARC_OFF));
 CHECK(zarc_controller_open(&c,1,10,1,0,0)==ZARC_CONN_COMPLETED);
 zarc_intent q={.session=1,.operation=ZARC_OP_DOMAIN,.request_id=7,.domain=1,.desired=ZARC_ON,.validity=10};
 CHECK(zarc_controller_request(&c,&q,false,true,0,1,true,&e)==ZARC_CONN_DENIED&&!e.emitted);
 CHECK(c.link.count==0&&c.endpoints[0].internal_id==0&&!c.endpoints[0].domain.pending);
 const uint32_t invalid_domains[]={0,3,UINT32_MAX};
 for(size_t i=0;i<sizeof invalid_domains/sizeof invalid_domains[0];++i){
  q.domain=invalid_domains[i];
  CHECK(zarc_controller_request(&c,&q,true,true,0,1,true,&e)==ZARC_CONN_INVALID&&!e.emitted);
  CHECK(c.link.count==0&&c.endpoints[0].internal_id==0&&!c.endpoints[0].domain.pending);
  CHECK(zarc_controller_snapshot(&c,1,&o)&&o.value==ZARC_OFF&&o.quality==ZARC_QUALITY_VALID);
 }
 q.domain=1;
 CHECK(zarc_controller_request(&c,&q,true,true,0,1,false,&e)==ZARC_CONN_BUSY&&!e.emitted&&c.link.count==0);
 CHECK(zarc_controller_request(&c,&q,true,true,0,1,true,&e)==ZARC_CONN_ACCEPTED&&e.emitted);
 first=e;
 CHECK(zarc_controller_request(&c,&q,true,true,0,2,false,&e)==ZARC_CONN_ACCEPTED&&!e.emitted);
 q.request_id=8;CHECK(zarc_controller_request(&c,&q,true,true,0,2,true,&e)==ZARC_CONN_BUSY&&c.link.count==1);
 CHECK(zarc_controller_feedback(&c,1,9,2,0,2,ZARC_ON));
 CHECK(zarc_link_result(&c.link,1,ZARC_OP_DOMAIN,7,2)==ZARC_CONN_ACCEPTED);
 q.domain=2;CHECK(zarc_controller_request(&c,&q,true,true,0,2,true,&e)==ZARC_CONN_ACCEPTED&&e.id==2);
 second=e;
 zarc_controller_disconnect(&c);
 CHECK(zarc_controller_feedback(&c,1,9,3,3,3,ZARC_ON));
 CHECK(zarc_controller_feedback(&c,2,9,1,3,3,ZARC_ON));
 CHECK(zarc_controller_effect_released(&c,&first,3));
 CHECK(zarc_controller_effect_released(&c,&second,3));
 CHECK(zarc_controller_open(&c,2,10,1,0,4)==ZARC_CONN_COMPLETED);
 CHECK(zarc_controller_snapshot(&c,1,&o)&&o.value==ZARC_ON&&o.quality==ZARC_QUALITY_VALID&&o.boot==9);
 CHECK(!zarc_controller_feedback(&c,1,8,4,4,4,ZARC_OFF));
 q.session=2;q.request_id=1;q.domain=1;q.desired=ZARC_OFF;q.validity=2;
 CHECK(zarc_controller_request(&c,&q,true,true,0,4,true,&e)==ZARC_CONN_ACCEPTED);
 CHECK(zarc_controller_feedback(&c,1,9,4,6,6,ZARC_OFF));
 CHECK(zarc_link_result(&c.link,2,ZARC_OP_DOMAIN,1,6)==ZARC_CONN_UNKNOWN_OUTCOME);
 first=e;
 q.request_id=2;
 CHECK(zarc_controller_request(&c,&q,true,true,0,6,true,&e)==ZARC_CONN_BUSY);
 CHECK(zarc_controller_effect_released(&c,&first,6));
 CHECK(!zarc_controller_effect_released(&c,&first,6));
 CHECK(zarc_controller_ready(&c,1,false));
 q.request_id=2;CHECK(zarc_controller_request(&c,&q,true,true,0,7,true,&e)==ZARC_CONN_UNAVAILABLE);
 CHECK(zarc_controller_tick(&c,26));
 CHECK(zarc_controller_snapshot(&c,1,&o)&&o.quality==ZARC_QUALITY_STALE);
 CHECK(zarc_controller_snapshot(&c,2,&o)&&o.quality==ZARC_QUALITY_STALE);
 puts("Integrated controller contracts passed");return 0;
}
