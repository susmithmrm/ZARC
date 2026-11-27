#include "zarc/schedule.h"
#include <stddef.h>
static uint64_t add_sat(uint64_t a,uint64_t b){return b>UINT64_MAX-a?UINT64_MAX:a+b;}
bool zarc_periodic_init(zarc_periodic *s,uint64_t first,uint64_t period,uint64_t deadline){
 if(!s||!period||!deadline||deadline>period||first>UINT64_MAX-deadline)return false;
 *s=(zarc_periodic){.period=period,.deadline=deadline,.next_release=first,.configured=true};return true;
}
bool zarc_periodic_poll(zarc_periodic *s,uint64_t now,zarc_release *out){
 if(!s||!out||!s->configured||now<s->last_tick)return false;
 *out=(zarc_release){0};s->last_tick=now;
 if(s->pending&&now>=s->current_deadline){s->missed=add_sat(s->missed,1);s->pending=false;}
 if(s->exhausted||now<s->next_release)return true;
 uint64_t skipped=(now-s->next_release)/s->period;
 uint64_t release=s->next_release+skipped*s->period;
 s->skipped=add_sat(s->skipped,skipped);out->skipped=skipped;
 if(release>UINT64_MAX-s->period)s->exhausted=true;
 else s->next_release=release+s->period;
 if(release>UINT64_MAX-s->deadline){s->exhausted=true;s->missed=add_sat(s->missed,1);return true;}
 uint64_t deadline=release+s->deadline;
 if(now>=deadline){s->missed=add_sat(s->missed,1);return true;}
 out->due=true;out->release=release;out->deadline=deadline;
 s->pending=true;s->current_deadline=deadline;return true;
}
bool zarc_periodic_complete(zarc_periodic *s,uint64_t now,bool *on_time){
 if(!s||!on_time||!s->configured||!s->pending||now<s->last_tick)return false;
 s->last_tick=now;s->pending=false;*on_time=now<s->current_deadline;
 if(*on_time)s->completed=add_sat(s->completed,1);else s->missed=add_sat(s->missed,1);
 return true;
}
