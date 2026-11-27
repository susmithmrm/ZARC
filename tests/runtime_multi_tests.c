#include "zarc/runtime.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_runtime r={0};
 const zarc_runtime_config cfg={.count=2,.endpoints={{1,30},{2,30}},
 .period=10,.deadline=8,.health_limit=25,.max_validity=50,.remote_enabled=true};
 zarc_controller_effect effect,remote;
 zarc_runtime_sample samples[]={{1,1,0,ZARC_OFF},{2,1,0,ZARC_OFF}};
 CHECK(zarc_runtime_init(&r,&cfg,99,0));
 CHECK(!zarc_runtime_step_batch(&r,0,samples,3));
 CHECK(zarc_runtime_step_batch(&r,0,samples,2)&&r.healthy);
 CHECK(zarc_runtime_open(&r,1,7,1,0,1)==ZARC_CONN_COMPLETED);
 CHECK(zarc_runtime_local_at(&r,1,ZARC_ON,true,1,20,true,&effect)==ZARC_CONN_ACCEPTED);
 zarc_intent q={.session=1,.request_id=1,.domain=2,.desired=ZARC_ON,.operation=ZARC_OP_DOMAIN,.validity=20};
 CHECK(zarc_runtime_remote(&r,&q,true,true,0,1,true,&remote)==ZARC_CONN_ACCEPTED);
 CHECK(!zarc_runtime_suspend(&r,2,20));
 zarc_runtime_disconnect(&r);
 samples[0]=(zarc_runtime_sample){1,2,10,ZARC_ON};
 CHECK(zarc_runtime_step_batch(&r,10,samples,1)&&!r.healthy&&r.endpoint_healthy[0]&&!r.endpoint_healthy[1]);
 CHECK(zarc_runtime_release(&r,&effect,10));
 CHECK(zarc_runtime_release(&r,&remote,21));
 CHECK(zarc_runtime_local_at(&r,2,ZARC_OFF,true,21,20,true,&effect)==ZARC_CONN_UNAVAILABLE);
 CHECK(zarc_runtime_suspend(&r,22,29)&&r.suspended);
 CHECK(zarc_runtime_resume(&r,30)&&!r.healthy);
 zarc_observation o;
 CHECK(zarc_controller_snapshot(&r.controller,1,&o)&&o.quality==ZARC_QUALITY_UNKNOWN);
 CHECK(zarc_runtime_step_batch(&r,30,samples,1)&&!r.endpoint_healthy[0]); /* stale replay */
 samples[0]=(zarc_runtime_sample){1,3,40,ZARC_ON};
 samples[1]=(zarc_runtime_sample){2,2,40,ZARC_OFF};
 CHECK(zarc_runtime_step_batch(&r,40,samples,2)&&r.healthy);
 CHECK(zarc_runtime_open(&r,2,7,1,0,40)==ZARC_CONN_COMPLETED);
 CHECK(zarc_runtime_stop(&r,41,50));
 puts("Multi-endpoint runtime isolation and logical lifecycle passed");return 0;
}
