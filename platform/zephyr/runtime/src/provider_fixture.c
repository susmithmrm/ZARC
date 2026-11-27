#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <string.h>
#include "provider_fixture.h"
#include "device_pm.h"
#include <zephyr/pm/device.h>
#include <errno.h>
#include "zarc_manifest.h"
enum command_kind { SAMPLE, EFFECT, SUSPEND, RESUME, STOP };
/* This device exists only on native_sim. No physical power rail is touched. */
static int fixture_pm_action(const struct device *dev,enum pm_device_action action){
 ARG_UNUSED(dev);
 return action==PM_DEVICE_ACTION_RESUME||action==PM_DEVICE_ACTION_SUSPEND?0:-ENOTSUP;
}
PM_DEVICE_DEFINE(zarc_runtime_virtual_pm,fixture_pm_action);
DEVICE_DEFINE(zarc_runtime_virtual_pm,"zarc_runtime_virtual_pm",NULL,
 PM_DEVICE_GET(zarc_runtime_virtual_pm),NULL,NULL,POST_KERNEL,CONFIG_KERNEL_INIT_PRIORITY_DEVICE,NULL);
struct command {enum command_kind kind;uint64_t id;zarc_controller_effect effect;};
struct reply {
 uint64_t id;bool ok,applied;
 size_t count;zarc_runtime_sample samples[ZARC_CONTROLLER_DOMAINS];
};
K_MSGQ_DEFINE(commands,sizeof(struct command),2,8);
K_MSGQ_DEFINE(replies,sizeof(struct reply),2,8);
K_THREAD_STACK_DEFINE(worker_stack,ZARC_RUNTIME_WORKER_STACK);
static struct k_thread worker;
static k_tid_t tid;
/* Setup before starting the worker; never mutate while it is running. */
static zarc_endpoint_config config[ZARC_CONTROLLER_DOMAINS];
static size_t count;
static uint64_t transaction;
static bool fault;
static uint64_t ticks(void){return (uint64_t)k_uptime_get();}
static void run(void *a,void *b,void *c){
 ARG_UNUSED(a);ARG_UNUSED(b);ARG_UNUSED(c);
 zarc_value values[ZARC_CONTROLLER_DOMAINS]={0};
 uint64_t seq[ZARC_CONTROLLER_DOMAINS]={0},tokens[ZARC_CONTROLLER_DOMAINS]={0};
 for(size_t i=0;i<count;++i)values[i]=ZARC_OFF;
 struct zarc_native_pm pm={0};
 if(zarc_native_pm_open(&pm,DEVICE_GET(zarc_runtime_virtual_pm))!=0||zarc_native_pm_acquire(&pm)!=0)return;
 for(;;){
  struct command q;
  if(k_msgq_get(&commands,&q,K_FOREVER)!=0)return;
  struct reply r={.id=q.id,.ok=true};
  if(q.kind==SUSPEND){r.ok=zarc_native_pm_release(&pm)==0;}
  else if(q.kind==RESUME){r.ok=zarc_native_pm_acquire(&pm)==0;}
  else if(q.kind==STOP){r.ok=!pm.held||zarc_native_pm_release(&pm)==0;}
  else if(!pm.held){r.ok=false;}
  else if(q.kind==SAMPLE){
   r.count=count;
   uint64_t now=ticks();
   for(size_t i=0;i<count;++i){
    if(seq[i]==UINT64_MAX){r.ok=false;break;}
    r.samples[i]=(zarc_runtime_sample){config[i].id,++seq[i],now,values[i]};
   }
  }else if(q.kind==EFFECT){
   const zarc_controller_effect *e=&q.effect;size_t i=0;
   while(i<count&&config[i].id!=e->id)++i;
   if(i==count||!e->emitted||e->boot!=1||!e->token||
      e->token<=tokens[i]||(e->desired!=ZARC_ON&&e->desired!=ZARC_OFF))r.ok=false;
   else {
    tokens[i]=e->token;
    /* Expired work is discarded. Readback is a later separate SAMPLE. */
    if(ticks()<e->deadline){values[i]=e->desired;r.applied=true;}
   }
  }else r.ok=false;
  if(k_msgq_put(&replies,&r,K_NO_WAIT)!=0)return;
  if(q.kind==STOP)return;
 }
}
static bool exchange(struct command *q,struct reply *r){
 if(!tid||fault||transaction==UINT64_MAX)return false;
 q->id=++transaction;
 if(k_msgq_put(&commands,q,K_NO_WAIT)!=0||k_msgq_get(&replies,r,K_MSEC(4))!=0||
    r->id!=q->id||!r->ok){fault=true;return false;}
 return true;
}
bool zarc_profile_provider_start(const zarc_endpoint_config *p,size_t n){
 if(tid||!p||!n||n>ZARC_CONTROLLER_DOMAINS)return false;
 memcpy(config,p,n*sizeof *p);count=n;transaction=0;fault=false;
 k_msgq_purge(&commands);k_msgq_purge(&replies);
 tid=k_thread_create(&worker,worker_stack,K_THREAD_STACK_SIZEOF(worker_stack),run,NULL,NULL,NULL,
   K_PRIO_PREEMPT(ZARC_RUNTIME_WORKER_PRIORITY),0,K_NO_WAIT);
 return tid!=NULL;
}
bool zarc_profile_provider_sample(zarc_runtime_sample *out,size_t n){
 struct command q={.kind=SAMPLE};struct reply r;
 if(!out||n!=count||!exchange(&q,&r)||r.count!=n)return false;
 memcpy(out,r.samples,n*sizeof *out);return true;
}
bool zarc_profile_provider_effect(const zarc_controller_effect *e,bool *applied){
 if(!e||!applied)return false;
 struct command q={.kind=EFFECT,.effect=*e};struct reply r;
 if(!exchange(&q,&r))return false;
 *applied=r.applied;return true;
}
bool zarc_profile_provider_suspend(void){
 struct command q={.kind=SUSPEND};struct reply r;return exchange(&q,&r);
}
bool zarc_profile_provider_resume(void){
 struct command q={.kind=RESUME};struct reply r;return exchange(&q,&r);
}
bool zarc_profile_provider_stop(void){
 if(!tid)return false;
 struct command q={.kind=STOP};struct reply r;
 bool ok=exchange(&q,&r);
 if(!ok||k_thread_join(tid,K_MSEC(100))!=0){k_thread_abort(tid);ok=false;}
 tid=NULL;return ok;
}
