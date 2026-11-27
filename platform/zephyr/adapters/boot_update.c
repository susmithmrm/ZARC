#include "boot_update.h"
#include <zephyr/dfu/mcuboot.h>
#include <errno.h>
static int save(zarc_update *u,const zarc_update *next,zarc_update_persist_fn persist,void *ctx){
 int ret=persist(ctx,next);
 if(ret!=0)return ret<0?ret:-EIO;
 *u=*next;return 0;
}
int zarc_native_request_trial(zarc_update *u,bool eligible,zarc_update_persist_fn persist,void *ctx){
 if(!u||!persist)return -EINVAL;
 zarc_update next=*u;
 if(zarc_update_trial_request(&next,eligible)!=ZARC_UPDATE_OK)return -EPERM;
 int ret=save(u,&next,persist,ctx);if(ret<0)return ret;
 /* Never request permanent upgrade; recovery path must remain available. */
 ret=boot_request_upgrade(BOOT_UPGRADE_TEST);
 if(ret<0){
  zarc_update_interrupt(&next);
  (void)save(u,&next,persist,ctx);
 }
 return ret;
}
int zarc_native_confirm_trial(zarc_update *u,bool healthy,bool compatible,bool eligible,
                             zarc_update_persist_fn persist,void *ctx){
 if(!u||!persist||!u->configured||u->phase!=ZARC_UPDATE_TRIAL)return -EINVAL;
 zarc_update next=*u;
 if(zarc_update_confirm(&next,healthy,compatible,eligible)!=ZARC_UPDATE_OK){
  int ret=save(u,&next,persist,ctx);return ret<0?ret:-EPERM;
 }
 /* The owner must have reconciled the actual running verified image with the
  * candidate in TRIAL. Never confirm solely because the boot timer expired. */
 if(!boot_is_img_confirmed()){
  int ret=boot_write_img_confirmed();if(ret<0)return ret;
 }
 /* If journaling fails after confirmation, next boot must reconcile the
  * verified running image and confirmation state before offering new work.
  * No rollback or successful journal write is asserted on that failure. */
 return save(u,&next,persist,ctx);
}
