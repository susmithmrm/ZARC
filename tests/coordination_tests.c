#include "zarc/update.h"
#include "zarc/power.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_update u;zarc_update_policy p={1,2,3,1};
 zarc_image active={11,1,1,2,3},candidate={12,2,1,2,3},wrong={13,3,2,2,3};
 CHECK(!zarc_update_init(&u,&p,&active,false));
 CHECK(zarc_update_init(&u,&p,&active,true));
 CHECK(zarc_update_stage(&u,&wrong,true,true,true)==ZARC_UPDATE_DENIED);
 CHECK(zarc_update_stage(&u,&candidate,false,true,true)==ZARC_UPDATE_DENIED);
 CHECK(zarc_update_stage(&u,&candidate,true,true,false)==ZARC_UPDATE_DENIED);
 for(unsigned step=0;step<3;++step){
  CHECK(zarc_update_stage(&u,&candidate,true,true,true)==ZARC_UPDATE_OK);
  if(step>=1)CHECK(zarc_update_trial_request(&u,true)==ZARC_UPDATE_OK);
  if(step>=2)CHECK(zarc_update_boot_report(&u,&candidate,true)==ZARC_UPDATE_OK);
  zarc_update_interrupt(&u);CHECK(u.phase==ZARC_UPDATE_RECOVERY_REQUIRED);
  CHECK(zarc_update_boot_report(&u,&active,true)==ZARC_UPDATE_OK&&u.active.image_id==11);
 }
 CHECK(zarc_update_stage(&u,&candidate,true,true,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_trial_request(&u,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_boot_report(&u,&candidate,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_confirm(&u,false,true,true)==ZARC_UPDATE_DENIED&&u.phase==ZARC_UPDATE_RECOVERY_REQUIRED);
 CHECK(zarc_update_boot_report(&u,&active,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_stage(&u,&candidate,true,true,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_trial_request(&u,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_boot_report(&u,&candidate,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_confirm(&u,true,true,true)==ZARC_UPDATE_OK&&u.active.version==2);
 CHECK(zarc_update_stage(&u,&active,true,true,true)==ZARC_UPDATE_DENIED);
 zarc_power power;
 CHECK(zarc_power_init(&power,3,9,0));
 CHECK(!zarc_power_request(&power,1,0,10,false));
 CHECK(zarc_power_request(&power,1,0,10,true));
 CHECK(zarc_power_report(&power,9,1,0,true,1));
 CHECK(!zarc_power_report(&power,8,1,1,true,2));
 CHECK(zarc_power_tick(&power,10)&&power.phase==ZARC_POWER_FALLBACK);
 CHECK(!zarc_power_report(&power,9,1,1,true,10));
 CHECK(zarc_power_request(&power,2,11,20,true));
 CHECK(zarc_power_report(&power,9,2,0,true,12));
 CHECK(zarc_power_report(&power,9,2,1,true,13)&&power.phase==ZARC_POWER_READY);
 CHECK(zarc_power_report(&power,9,2,0,false,14)&&power.phase==ZARC_POWER_WAITING);
 CHECK(zarc_power_cancel(&power,2,15,true)&&power.phase==ZARC_POWER_CANCELLED);
 CHECK(!zarc_power_request(&power,2,16,20,true));
 puts("Coordination contracts passed");return 0;
}
