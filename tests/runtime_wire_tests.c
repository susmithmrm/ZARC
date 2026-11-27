#include "zarc/runtime_wire.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static bool scope(void *ctx,uint64_t principal,uint8_t op,uint32_t id){
 (void)ctx;return principal==7&&id!=2&&op!=ZARC_OP_MODE;
}
int main(void){
 zarc_runtime r={0};
 const zarc_runtime_config cfg={.count=2,.endpoints={{1,30},{2,30}},.remote_enabled=true,
  .period=10,.deadline=8,.health_limit=25,.max_validity=50,.update_status_max_age=20};
 const zarc_runtime_sample s[]={{1,1,0,ZARC_OFF},{2,1,0,ZARC_OFF}};
 zarc_wire_peer peer={.principal=7,.age_known=true,.allow=scope};
 zarc_frame q={.operation=ZARC_OP_HELLO,.request_id=1},out;
 zarc_controller_effect effect;
 CHECK(zarc_runtime_init(&r,&cfg,9,0)&&zarc_runtime_step_batch(&r,0,s,2));
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,0,true,&out,&effect)&&out.payload[0]==ZARC_CONN_COMPLETED);
 q.boot=out.boot;q.session=out.session;q.operation=ZARC_OP_CAPABILITIES;q.request_id=2;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,0,true,&out,&effect)&&out.length==7&&out.payload[1]==1);
 q.operation=ZARC_OP_DOMAIN;q.length=9;q.request_id=3;q.payload[0]=2;q.payload[4]=ZARC_ON;q.payload[5]=20;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,0,true,&out,&effect)&&out.payload[0]==ZARC_CONN_DENIED&&!effect.emitted);
 q.payload[0]=1;peer.age_known=false;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,0,true,&out,&effect)&&out.payload[0]==ZARC_CONN_EXPIRED&&!effect.emitted);
 peer.age_known=true;peer.principal=8;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,0,true,&out,&effect)&&out.payload[0]==ZARC_CONN_UNAVAILABLE);
 peer.principal=7;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,0,true,&out,&effect)&&out.payload[0]==ZARC_CONN_ACCEPTED&&effect.emitted);
 zarc_frame renew={.operation=ZARC_OP_HELLO,.boot=q.boot,.session=q.session,.request_id=99};
 zarc_controller_effect none;
 CHECK(zarc_runtime_dispatch(&r,&peer,&renew,0,false,&out,&none)&&out.payload[0]==ZARC_CONN_BUSY);
 CHECK(out.session==q.session);
 CHECK(zarc_runtime_release(&r,&effect,1));
 const zarc_runtime_sample newer[]={{1,2,10,ZARC_ON},{2,2,10,ZARC_OFF}};
 CHECK(zarc_runtime_step_batch(&r,10,newer,2));
 q.operation=ZARC_OP_REQUEST_STATUS;q.length=8;q.request_id=4;
 for(size_t i=0;i<8;++i)q.payload[i]=0;
 q.payload[0]=3;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_COMPLETED);
 q.operation=ZARC_OP_SNAPSHOT;q.length=4;q.request_id=5;q.payload[0]=2;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_DENIED);
 q.operation=ZARC_OP_HEALTH;q.request_id=6;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_DENIED);
 q.payload[0]=1;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_COMPLETED&&out.length==28);
 CHECK(out.payload[1]==1&&out.payload[2]==1&&out.payload[6]==0&&out.payload[10]==1&&out.payload[11]==0);
 q.operation=ZARC_OP_UPDATE_STATUS;q.length=0;q.request_id=7;
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_UNAVAILABLE);
 const zarc_update_policy policy={1,2,3,1};
 const zarc_image image={11,1,1,2,3};
 zarc_update update;
 CHECK(zarc_update_init(&update,&policy,&image,true));
 CHECK(zarc_runtime_publish_update(&r,&update,1,10,true));
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_COMPLETED&&out.length==51);
 CHECK(out.payload[1]==1&&out.payload[2]==ZARC_UPDATE_ACTIVE&&out.payload[3]==1&&out.payload[11]==11);
 CHECK(!effect.emitted);
 CHECK(!zarc_runtime_publish_update(&r,&update,1,10,true));
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_UNAVAILABLE);
 CHECK(zarc_runtime_publish_update(&r,&update,2,10,true));
 uint64_t rev,tick;
 CHECK(!zarc_runtime_read_update(&r,30,&update,&rev,&tick));
 CHECK(zarc_runtime_dispatch(&r,&peer,&renew,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_COMPLETED&&out.session==2);
 CHECK(r.controller.link.count==0);
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_UNAVAILABLE);
 zarc_runtime_disconnect(&r);
 CHECK(zarc_runtime_dispatch(&r,&peer,&q,10,false,&out,&effect)&&out.payload[0]==ZARC_CONN_UNAVAILABLE);
 puts("Runtime wire scope, freshness and retained results passed");return 0;
}
