#include "zarc/connect.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_link l,other;bool fresh;
 CHECK(zarc_link_init(&l,9,100,0)&&zarc_link_init(&other,9,100,0));
 CHECK(zarc_link_open(&l,1,10,2,0,0)==ZARC_CONN_INCOMPATIBLE);
 CHECK(zarc_link_open(&l,1,10,1,0,0)==ZARC_CONN_COMPLETED);
 CHECK(zarc_link_open(&other,1,11,1,0,0)==ZARC_CONN_COMPLETED);
 zarc_intent q={.session=1,.request_id=7,.domain=1,.desired=1,.operation=ZARC_OP_DOMAIN,.validity=10};
 CHECK(zarc_link_admit(&l,&q,false,true,0,1,&fresh)==ZARC_CONN_DENIED&&!fresh);
 CHECK(zarc_link_admit(&l,&q,true,false,0,1,&fresh)==ZARC_CONN_EXPIRED);
 CHECK(zarc_link_admit(&l,&q,true,true,2,1,&fresh)==ZARC_CONN_ACCEPTED&&fresh);
 CHECK(zarc_link_admit(&l,&q,true,true,3,2,&fresh)==ZARC_CONN_ACCEPTED&&!fresh);
 q.desired=0;CHECK(zarc_link_admit(&l,&q,true,true,3,2,&fresh)==ZARC_CONN_CONFLICT);
 CHECK(zarc_link_admit(&other,&q,true,true,3,2,&fresh)==ZARC_CONN_ACCEPTED&&fresh);
 zarc_link_disconnect(&l);CHECK(zarc_link_open(&l,2,10,1,0,3)==ZARC_CONN_BUSY);
 CHECK(zarc_link_finish(&l,1,ZARC_OP_DOMAIN,7,4,true)==ZARC_CONN_COMPLETED);
 CHECK(zarc_link_open(&l,2,10,1,0,5)==ZARC_CONN_COMPLETED);
 CHECK(zarc_link_admit(&l,&q,true,true,0,5,&fresh)==ZARC_CONN_UNAVAILABLE);
 q.session=2;q.validity=1;
 for(uint64_t i=1;i<=ZARC_REQUEST_LIMIT;++i){q.request_id=i;CHECK(zarc_link_admit(&l,&q,true,true,0,5,&fresh)==ZARC_CONN_ACCEPTED&&fresh);}
 q.request_id=99;CHECK(zarc_link_admit(&l,&q,true,true,0,5,&fresh)==ZARC_CONN_BUSY);
 CHECK(zarc_link_finish(&l,2,ZARC_OP_DOMAIN,1,6,true)==ZARC_CONN_UNKNOWN_OUTCOME);
 CHECK(zarc_link_result(&l,2,ZARC_OP_DOMAIN,2,6)==ZARC_CONN_UNKNOWN_OUTCOME);
 zarc_frame f={.operation=ZARC_OP_DOMAIN,.length=3,.boot=9,.session=2,.request_id=99,.payload={1,2,3}},decoded;
 uint8_t bytes[2*(ZARC_FRAME_HEADER+ZARC_PAYLOAD_MAX)];size_t n,u;
 CHECK(zarc_frame_encode(&f,bytes,sizeof bytes,&n)&&n==35);
 CHECK(bytes[0]=='Z'&&bytes[8]==9&&bytes[24]==99);
 CHECK(!zarc_frame_decode(bytes,n-1,&decoded));
 memcpy(bytes+n,bytes,n);
 zarc_stream s={0};
 CHECK(zarc_stream_feed(&s,bytes,7,&u,&decoded)==ZARC_STREAM_MORE&&u==7);
 CHECK(zarc_stream_feed(&s,bytes+7,2*n-7,&u,&decoded)==ZARC_STREAM_FRAME&&u==n-7&&decoded.payload[2]==3);
 CHECK(zarc_stream_feed(&s,bytes+n,n,&u,&decoded)==ZARC_STREAM_FRAME&&u==n);
 bytes[6]=255;bytes[7]=255;CHECK(zarc_stream_feed(&s,bytes,n,&u,&decoded)==ZARC_STREAM_INVALID);
 zarc_snapshot_cache cache={0};
 CHECK(zarc_cache_snapshot(&cache,9,2,5,10,1));
 CHECK(zarc_cache_event(&cache,9,2,6,11,0));
 CHECK(!zarc_cache_event(&cache,9,2,7,13,1)&&!cache.valid);
 CHECK(zarc_cache_snapshot(&cache,10,3,0,0,0));
 CHECK(!zarc_cache_event(&cache,9,2,8,1,1)&&!cache.valid);
 puts("Connect contracts passed");return 0;
}
