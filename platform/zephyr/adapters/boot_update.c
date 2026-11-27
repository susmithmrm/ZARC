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
int zarc_native_confirm_trial(zarc_update *u,const zarc_image *running,bool verified,bool healthy,bool compatible,bool eligible,
                              zarc_update_persist_fn persist,void *ctx){
 if(!u||!running||!persist||!u->configured||u->phase!=ZARC_UPDATE_TRIAL)return -EINVAL;
 const zarc_image *i=&u->candidate;
 if(!verified||running->image_id!=i->image_id||running->version!=i->version||
    running->target!=i->target||running->contract!=i->contract||running->config_schema!=i->config_schema)return -EPERM;
 zarc_update next=*u;
 if(zarc_update_prepare_confirmation(&next,healthy,compatible,eligible)!=ZARC_UPDATE_OK){
  int ret=save(u,&next,persist,ctx);return ret<0?ret:-EPERM;
 }
 int ret=save(u,&next,persist,ctx);if(ret<0)return ret;
 if(!boot_is_img_confirmed()){
  ret=boot_write_img_confirmed();if(ret<0)return ret;
 }
 if(zarc_update_reconcile_confirmation(&next,running,verified,boot_is_img_confirmed())!=ZARC_UPDATE_OK)return -EIO;
 return save(u,&next,persist,ctx);
}
