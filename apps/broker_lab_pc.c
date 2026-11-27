#include "zarc/domain.h"
#include "loopback.h"
#include "lab_wire.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static bool exchange(zarc_socket s,const zarc_frame *q,zarc_frame *r){
 if(!zarc_net_send(s,q))return false;
 zarc_stream stream={0};uint64_t start=zarc_net_milliseconds();
 while(zarc_net_milliseconds()-start<2000u){
  int ready=zarc_net_readable(s,100);if(ready<0)return false;if(!ready)continue;
  uint8_t bytes[ZARC_FRAME_HEADER+ZARC_PAYLOAD_MAX];int n=zarc_net_read(s,bytes,sizeof bytes);
  if(n<=0)return false;
  size_t used;zarc_stream_status status=zarc_stream_feed(&stream,bytes,(size_t)n,&used,r);
  if(status==ZARC_STREAM_INVALID||used!=(size_t)n)return false;
  if(status==ZARC_STREAM_FRAME)return r->operation==ZARC_OP_RESULT&&r->length>=1&&r->request_id==q->request_id;
 }
 return false;
}
int main(int argc,char **argv){
 if(argc!=3)return 2;
 char *end;unsigned long port=strtoul(argv[1],&end,10);
 bool write=strcmp(argv[2],"on")==0;
 if(!port||port>65535u||*end||(!write&&strcmp(argv[2],"snapshot")!=0))return 2;
 if(!zarc_net_init())return 1;
 zarc_socket s=zarc_net_connect((uint16_t)port);int result=1;
 if(s!=ZARC_BAD_SOCKET){
  zarc_frame q={.operation=ZARC_OP_HELLO,.request_id=1},r;
  if(exchange(s,&q,&r)){
   if(r.payload[0]==ZARC_CONN_BUSY)result=3;
   else if(r.payload[0]==ZARC_CONN_COMPLETED&&r.boot&&r.session){
    q=(zarc_frame){.operation=write?ZARC_OP_DOMAIN:ZARC_OP_SNAPSHOT,.length=write?9u:4u,
      .boot=r.boot,.session=r.session,.request_id=7};
    lab_put(q.payload,4,1);
    if(write){q.payload[4]=ZARC_ON;lab_put(q.payload+5,4,2000);}
    if(exchange(s,&q,&r)&&r.boot==q.boot&&r.session==q.session){
     if(write&&r.payload[0]==ZARC_CONN_ACCEPTED&&r.length==1){puts("ACCEPTED");result=0;}
     else if(!write&&r.payload[0]==ZARC_CONN_COMPLETED&&r.length==11){
      printf("SNAPSHOT %u %u\n",(unsigned)r.payload[1],(unsigned)r.payload[2]);result=0;
     }
    }
   }
  }
 }
 zarc_net_close(s);zarc_net_cleanup();return result;
}
