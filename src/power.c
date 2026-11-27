#include "zarc/power.h"
bool zarc_power_init(zarc_power *p,uint32_t required,uint64_t epoch,uint64_t now){
 if(!p||!required||!epoch)return false;
 *p=(zarc_power){.required_mask=required,.epoch=epoch,.last_tick=now,.configured=true};return true;
}
bool zarc_power_tick(zarc_power *p,uint64_t now){
 if(!p||!p->configured||now<p->last_tick)return false;
 p->last_tick=now;
 if(p->phase==ZARC_POWER_WAITING&&now>=p->deadline)p->phase=ZARC_POWER_FALLBACK;
 return true;
}
bool zarc_power_request(zarc_power *p,uint64_t id,uint64_t now,uint64_t deadline,bool authorized){
 if(!p||!p->configured||!authorized||!id||id<=p->request_id||deadline<=now||!zarc_power_tick(p,now))return false;
 if(p->phase==ZARC_POWER_WAITING||p->phase==ZARC_POWER_READY)return false;
 p->request_id=id;p->deadline=deadline;p->ready_mask=0;p->phase=ZARC_POWER_WAITING;return true;
}
bool zarc_power_report(zarc_power *p,uint64_t epoch,uint64_t id,unsigned guest,bool ready,uint64_t now){
 if(!p||!p->configured||epoch!=p->epoch||id!=p->request_id||guest>=32u||!zarc_power_tick(p,now))return false;
 uint32_t bit=UINT32_C(1)<<guest;
 if(!(p->required_mask&bit)||(p->phase!=ZARC_POWER_WAITING&&p->phase!=ZARC_POWER_READY))return false;
 if(ready)p->ready_mask|=bit;else p->ready_mask&=~bit;
 if(p->ready_mask==p->required_mask)p->phase=ZARC_POWER_READY;
 else p->phase=now>=p->deadline?ZARC_POWER_FALLBACK:ZARC_POWER_WAITING;
 return true;
}
bool zarc_power_cancel(zarc_power *p,uint64_t id,uint64_t now,bool authorized){
 if(!p||!authorized||id!=p->request_id||!zarc_power_tick(p,now))return false;
 if(p->phase!=ZARC_POWER_WAITING&&p->phase!=ZARC_POWER_READY)return false;
 p->phase=ZARC_POWER_CANCELLED;p->ready_mask=0;return true;
}
