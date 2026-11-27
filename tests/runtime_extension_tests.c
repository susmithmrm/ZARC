#include "zarc/runtime.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {bool fail;unsigned started,suspended,resumed,released;} fixture;
static bool init(void *p){return !((fixture *)p)->fail;}
static bool start(void *p){++((fixture *)p)->started;return true;}
static bool quiesce(void *p,uint64_t deadline){return p&&deadline!=0;}
static bool suspend_service(void *p){++((fixture *)p)->suspended;return true;}
static bool resume_service(void *p){++((fixture *)p)->resumed;return true;}
static void release(void *p){++((fixture *)p)->released;}
int main(void){
 fixture f={.fail=true};zarc_runtime r={0};
 zarc_service_descriptor s={.id=3,.required=true,.dependency_count=2,.dependencies={1,2},
  .context=&f,.init=init,.start=start,.quiesce=quiesce,.suspend=suspend_service,.resume=resume_service,.release=release};
 zarc_runtime_config cfg={.endpoint={1,30},.period=10,.deadline=8,.health_limit=25,.max_validity=50,
 .remote_enabled=true,.remote_service=3,.extensions=&s,.extension_count=1};
 CHECK(zarc_runtime_init(&r,&cfg,11,0));
 CHECK(zarc_runtime_step(&r,0,true,1,0,ZARC_OFF)&&!r.healthy&&r.endpoint_healthy[0]);
 CHECK(!zarc_runtime_remote_ready(&r));
 CHECK(zarc_runtime_open(&r,1,7,1,0,0)==ZARC_CONN_UNAVAILABLE);
 zarc_controller_effect e;
 CHECK(zarc_runtime_local(&r,ZARC_ON,true,0,20,true,&e)==ZARC_CONN_ACCEPTED);
 CHECK(zarc_runtime_release(&r,&e,1));
 CHECK(zarc_runtime_step(&r,10,true,2,10,ZARC_ON));
 CHECK(zarc_runtime_stop(&r,11,20));
 f.fail=false;
 CHECK(zarc_runtime_init(&r,&cfg,12,20)&&f.started==1);
 CHECK(zarc_runtime_step(&r,20,true,1,20,ZARC_OFF)&&r.healthy&&zarc_runtime_remote_ready(&r));
 CHECK(zarc_runtime_suspend(&r,21,30)&&f.suspended==1&&!zarc_runtime_remote_ready(&r));
 CHECK(zarc_runtime_resume(&r,30)&&f.resumed==1);
 CHECK(!r.healthy&&zarc_runtime_step(&r,30,true,2,30,ZARC_OFF)&&r.healthy);
 CHECK(zarc_runtime_stop(&r,31,40)&&f.released==1);
 puts("Profile service lifecycle and independent local admission passed");return 0;
}
