#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include "zarc/runtime.h"
#include "zarc_manifest.h"
/* Simulator only: memory-backed device state, no physical GPIO/rail access.
 * A product provider must independently establish actual feedback and ownership. */
static zarc_runtime runtime;
static zarc_value device_value=ZARC_OFF;
static uint64_t ticks(void){return (uint64_t)k_uptime_get();}
int main(void){
 if(ZARC_MANIFEST_SERVICE_COUNT!=2u||zarc_manifest_service_order[0]!=1u||
    zarc_manifest_service_order[1]!=2u)return 1;
 const zarc_runtime_config config={.endpoint={1,50},.period=10,.deadline=8,
  .health_limit=40,.max_validity=100};
 uint64_t now=ticks(),sequence=0;
 if(!zarc_runtime_init(&runtime,&config,1,now))return 1;
 printk("ZARC reference manifest %s\n",ZARC_MANIFEST_SHA256);
 bool issued=false;
 for(;;){
  now=ticks();
  if(!zarc_runtime_step(&runtime,now,true,++sequence,now,device_value))return 1;
  if(IS_ENABLED(CONFIG_ZARC_RUNTIME_SMOKE)&&!issued&&runtime.healthy){
   zarc_controller_effect effect;
   if(zarc_runtime_local(&runtime,ZARC_ON,true,now,50,true,&effect)!=ZARC_CONN_ACCEPTED)return 1;
   /* Synchronous simulated provider: no queued work after returning. */
   if(ticks()>=effect.deadline)return 1;
   device_value=effect.desired;
   if(!zarc_runtime_release(&runtime,&effect,ticks()))return 1;
   issued=true;
  }
  if(IS_ENABLED(CONFIG_ZARC_RUNTIME_SMOKE)&&issued){
   zarc_observation observation;
   if(zarc_controller_snapshot(&runtime.controller,1,&observation)&&
      observation.value==ZARC_ON&&observation.quality==ZARC_QUALITY_VALID){
    now=ticks();
    if(!zarc_runtime_stop(&runtime,now,now+20))return 1;
    printk("ZARC runtime observed benign indicator and stopped\n");
    return 0;
   }
  }
  k_sleep(K_MSEC(1));
 }
}
