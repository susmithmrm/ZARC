#include "zarc/runtime.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_runtime r={0};
 const zarc_runtime_config cfg={.endpoint={1,30},.period=10,.deadline=8,.health_limit=25,
  .max_validity=50,.config_digest=1,.build_digest=2};
 zarc_controller_effect effect;
 zarc_observation observation;
 CHECK(zarc_runtime_init(&r,&cfg,77,0));
 CHECK(zarc_runtime_local(&r,ZARC_ON,true,0,20,true,&effect)==ZARC_CONN_UNAVAILABLE);
 CHECK(zarc_runtime_step(&r,0,true,1,0,ZARC_OFF)&&r.healthy);
 CHECK(zarc_runtime_local(&r,ZARC_ON,true,1,20,true,&effect)==ZARC_CONN_ACCEPTED);
 CHECK(effect.boot==77&&effect.token==1&&effect.deadline==21);
 CHECK(!zarc_runtime_stop(&r,2,10));
 CHECK(zarc_runtime_step(&r,10,true,2,10,ZARC_ON)&&r.healthy);
 CHECK(zarc_controller_snapshot(&r.controller,1,&observation)&&observation.value==ZARC_ON);
 CHECK(zarc_runtime_release(&r,&effect,11));
 CHECK(zarc_runtime_step(&r,20,false,0,0,ZARC_UNKNOWN)&&!r.healthy);
 CHECK(r.diagnostics.count>=1);
 CHECK(zarc_runtime_local(&r,ZARC_OFF,true,21,20,true,&effect)==ZARC_CONN_UNAVAILABLE);
 CHECK(zarc_runtime_step(&r,30,true,3,30,ZARC_ON)&&r.healthy);
 zarc_runtime_metrics metrics;zarc_runtime_health_view health;
 CHECK(zarc_runtime_metrics_read(&r,&metrics)&&metrics.completed==4&&metrics.missed==0&&metrics.skipped==0);
 CHECK(metrics.effect_highwater==1&&metrics.diagnostic_highwater>=1);
 CHECK(zarc_runtime_health_read(&r,1,&health)&&health.ready&&!health.failures&&!health.effect_owned);
 CHECK(zarc_runtime_step(&r,59,true,4,59,ZARC_ON));
 CHECK(zarc_runtime_metrics_read(&r,&metrics)&&metrics.skipped==1&&metrics.missed==1);
 CHECK(zarc_runtime_stop(&r,60,70)&&!r.running);
 CHECK(!zarc_runtime_step(&r,70,true,5,70,ZARC_OFF));
 puts("Continuous reference runtime contracts passed");return 0;
}
