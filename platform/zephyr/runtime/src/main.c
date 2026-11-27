#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include "zarc/runtime_wire.h"
#include "zarc/power.h"
#include "zarc_manifest.h"
#include "provider_fixture.h"
/* Simulator-only device readback and lab-local trusted peer. */
BUILD_ASSERT(CONFIG_MAIN_STACK_SIZE==ZARC_RUNTIME_MAIN_STACK,"manifest stack mismatch");
BUILD_ASSERT(CONFIG_MAIN_THREAD_PRIORITY==ZARC_RUNTIME_MAIN_PRIORITY,"manifest priority mismatch");
BUILD_ASSERT(ZARC_PROFILE_ENDPOINT_COUNT<=ZARC_CONTROLLER_DOMAINS,"endpoint allocation");
static zarc_runtime runtime;
static zarc_power power;
static uint64_t profile_boot_tick;
static bool ipc_opened;

struct encoded {size_t n;uint8_t data[ZARC_FRAME_HEADER+ZARC_PAYLOAD_MAX];};
K_MSGQ_DEFINE(lab_queue,sizeof(struct encoded),2,8);
#if defined(CONFIG_IPC_SERVICE)
int zarc_profile_ipc_open(void);
int zarc_profile_ipc_exchange(const zarc_frame *,zarc_frame *);
int zarc_profile_ipc_close(void);
#endif
static uint64_t ticks(void){return (uint64_t)k_uptime_get();}
static bool scope(void *ctx,uint64_t principal,uint8_t op,uint32_t id){
 ARG_UNUSED(ctx);ARG_UNUSED(op);ARG_UNUSED(id);
 return principal==1; /* fixed simulation identity only */
}
static bool wire(const zarc_frame *q,uint64_t now,zarc_frame *reply,zarc_controller_effect *effect){
 zarc_frame received;
#if defined(CONFIG_IPC_SERVICE)
 if(ZARC_PROFILE_KIND==3u){
  if(zarc_profile_ipc_exchange(q,&received)!=0)return false;
 }else
#endif
 {
  struct encoded bytes={0},copy;
  if(!zarc_frame_encode(q,bytes.data,sizeof bytes.data,&bytes.n)||
     k_msgq_put(&lab_queue,&bytes,K_NO_WAIT)!=0||k_msgq_get(&lab_queue,&copy,K_NO_WAIT)!=0||
     !zarc_frame_decode(copy.data,copy.n,&received))return false;
 }
 const zarc_wire_peer p={.principal=1,.age_known=true,.allow=scope};
 return zarc_runtime_dispatch(&runtime,&p,&received,now,true,reply,effect);
}
static bool simple_init(void *ctx){ARG_UNUSED(ctx);return true;}
static bool simple_start(void *ctx){ARG_UNUSED(ctx);return true;}
static bool simple_quiesce(void *ctx,uint64_t deadline){ARG_UNUSED(ctx);return ticks()<deadline;}
static bool simple_transition(void *ctx){ARG_UNUSED(ctx);return true;}
static void simple_release(void *ctx){ARG_UNUSED(ctx);}
static bool power_init(void *ctx){ARG_UNUSED(ctx);return zarc_power_init(&power,3,1,profile_boot_tick);}
static bool power_quiesce(void *ctx,uint64_t deadline){
 ARG_UNUSED(ctx);return ticks()<deadline&&power.phase!=ZARC_POWER_WAITING&&power.phase!=ZARC_POWER_READY;
}
static bool broker_quiesce(void *ctx,uint64_t deadline){ARG_UNUSED(ctx);return ticks()<deadline&&k_msgq_num_used_get(&lab_queue)==0;}
static void broker_release(void *ctx){ARG_UNUSED(ctx);k_msgq_purge(&lab_queue);}
static bool ipc_init(void *ctx){
 ARG_UNUSED(ctx);
#if defined(CONFIG_IPC_SERVICE)
 if(zarc_profile_ipc_open()!=0)return false;
 ipc_opened=true;return true;
#else
 return false;
#endif
}
static bool ipc_suspend(void *ctx){
 ARG_UNUSED(ctx);
#if defined(CONFIG_IPC_SERVICE)
 if(!ipc_opened||zarc_profile_ipc_close()!=0)return false;
 ipc_opened=false;return true;
#else
 return false;
#endif
}
static void ipc_release(void *ctx){
 ARG_UNUSED(ctx);
 if(ipc_opened)(void)ipc_suspend(NULL);
}
static size_t profile_services(zarc_service_descriptor *ext){
 if(ZARC_PROFILE_KIND==0u)return 0;
 ext[0]=(zarc_service_descriptor){.id=3,.required=true,.dependency_count=2,.dependencies={1,2},
  .init=simple_init,.start=simple_start,.quiesce=simple_quiesce,.suspend=simple_transition,
  .resume=simple_transition,.release=simple_release};
 if(ZARC_PROFILE_KIND==1u){
  ext[0].init=power_init;ext[0].quiesce=power_quiesce;
  ext[1]=ext[0];ext[1].id=4;ext[1].init=simple_init;ext[1].quiesce=broker_quiesce;ext[1].release=broker_release;
  return 2;
 }
 if(ZARC_PROFILE_KIND==3u){
  ext[0].init=ipc_init;ext[0].suspend=ipc_suspend;ext[0].resume=ipc_init;ext[0].release=ipc_release;
 }
 return 1;
}
static bool profile_setup(void){
 size_t expected=ZARC_PROFILE_KIND==1u?4u:ZARC_PROFILE_KIND==0u?2u:3u;
 if(ZARC_MANIFEST_SERVICE_COUNT!=expected)return false;
 for(size_t i=0;i<expected;++i)if(zarc_manifest_service_order[i]!=i+1u)return false;
 return true;
}
int main(void){
 zarc_service_descriptor extensions[2]={0};
 const size_t extension_count=profile_services(extensions);
 const zarc_runtime_config config={.extensions=extensions,.extension_count=extension_count,
  .remote_service=ZARC_PROFILE_KIND==1u?4u:ZARC_PROFILE_KIND==0u?0u:3u,
  .count=ZARC_PROFILE_ENDPOINT_COUNT,.endpoints=ZARC_PROFILE_ENDPOINTS,
  .period=ZARC_RUNTIME_PERIOD_MS,.deadline=ZARC_RUNTIME_DEADLINE_MS,.health_limit=ZARC_RUNTIME_HEALTH_LIMIT_MS,
  .max_validity=ZARC_RUNTIME_MAX_VALIDITY_MS,.config_digest=ZARC_RUNTIME_CONFIG_DIGEST,.remote_enabled=ZARC_PROFILE_KIND!=0u};
 uint64_t now=ticks();
 profile_boot_tick=now;
 if(!profile_setup()||!zarc_runtime_init(&runtime,&config,1,now))return 1;
 if(!zarc_composition_required_ready(&runtime.composition))return 1;
 if(!zarc_profile_provider_start(config.endpoints,config.count))return 1;
 printk("ZARC reference %s manifest %s\n",ZARC_PROFILE_NAME,ZARC_MANIFEST_SHA256);
 bool issued=false;
 for(;;){
  zarc_runtime_sample samples[ZARC_PROFILE_ENDPOINT_COUNT];
  if(!zarc_profile_provider_sample(samples,config.count))return 1;
  now=ticks();
  if(!zarc_runtime_step_batch(&runtime,now,samples,config.count))return 1;
  if(ZARC_PROFILE_KIND==1u&&!zarc_power_tick(&power,now))return 1;
  if(IS_ENABLED(CONFIG_ZARC_RUNTIME_SMOKE)&&!issued&&runtime.healthy){
   if(ZARC_PROFILE_KIND==1u){
    if(!zarc_power_request(&power,1,now,now+50,true)||
       !zarc_power_report(&power,1,1,0,true,now)||!zarc_power_report(&power,1,1,1,true,now)||
       power.phase!=ZARC_POWER_READY||!zarc_power_cancel(&power,1,now,true))return 1;
   }
   zarc_frame reply,q={.operation=ZARC_OP_HELLO,.request_id=1};zarc_controller_effect effect;
   if(config.remote_enabled){
    if(!wire(&q,now,&reply,&effect)||reply.payload[0]!=ZARC_CONN_COMPLETED)return 1;
    q.boot=reply.boot;q.session=reply.session;q.request_id=2;q.operation=ZARC_OP_CAPABILITIES;
    if(!wire(&q,now,&reply,&effect)||reply.payload[0]!=ZARC_CONN_COMPLETED||reply.payload[1]!=config.count)return 1;
   }
   for(size_t i=0;i<config.count;++i){
    uint32_t id=config.endpoints[i].id;
    if(config.remote_enabled){
     q.operation=ZARC_OP_DOMAIN;q.length=9;q.request_id=3u+i;
     for(size_t k=0;k<4;++k)q.payload[k]=(uint8_t)(id>>(8u*k));
     q.payload[4]=ZARC_ON;
     for(size_t k=0;k<4;++k)q.payload[5u+k]=(uint8_t)(50u>>(8u*k));
     if(!wire(&q,now,&reply,&effect)||reply.payload[0]!=ZARC_CONN_ACCEPTED||!effect.emitted)return 1;
    }else if(zarc_runtime_local_at(&runtime,id,ZARC_ON,true,now,50,true,&effect)!=ZARC_CONN_ACCEPTED)return 1;
    bool applied=false;
    if(!zarc_profile_provider_effect(&effect,&applied)||!zarc_runtime_release(&runtime,&effect,ticks())||!applied)return 1;
    now=ticks();
   }
   zarc_runtime_disconnect(&runtime);issued=true;
  }
  if(IS_ENABLED(CONFIG_ZARC_RUNTIME_SMOKE)&&issued){
   bool observed=true;
   for(size_t i=0;i<config.count;++i){
    zarc_observation o;
    if(!zarc_controller_snapshot(&runtime.controller,config.endpoints[i].id,&o)||o.value!=ZARC_ON||o.quality!=ZARC_QUALITY_VALID)observed=false;
   }
   if(observed){
    now=ticks();if(!zarc_runtime_suspend(&runtime,now,now+20)||!zarc_runtime_resume(&runtime,now))return 1;
    if(runtime.healthy||!zarc_runtime_stop(&runtime,now,now+20))return 1;
    if(ipc_opened||!zarc_profile_provider_stop())return 1;
    printk("ZARC runtime observed benign indicator and stopped\n");return 0;
   }
  }
  k_sleep(K_MSEC(1));
 }
}
