#ifndef ZARC_NATIVE_BOOT_UPDATE_H
#define ZARC_NATIVE_BOOT_UPDATE_H
#include "zarc/update.h"
/* Single-image MCUboot test/revert candidate. Persist callback must atomically
 * commit the explicit versioned update journal before returning success.
 * Image identity and verification must come from approved metadata/boot trust,
 * not a remote client's assertion. Sole maintenance-worker owner only. */
typedef int (*zarc_update_persist_fn)(void *,const zarc_update *);
int zarc_native_request_trial(zarc_update *,bool eligible,zarc_update_persist_fn,void *);
int zarc_native_confirm_trial(zarc_update *,const zarc_image *running,bool verified,bool healthy,bool compatible,bool eligible,
                             zarc_update_persist_fn,void *);
/* Caller restores the journal and supplies verified running-image identity.
 * Reconciliation persists both success and denial before admission. */
int zarc_native_recover_boot(zarc_update *,const zarc_image *running,bool verified,zarc_update_persist_fn,void *);
#endif
