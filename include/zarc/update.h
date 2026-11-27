#ifndef ZARC_UPDATE_H
#define ZARC_UPDATE_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {uint64_t image_id,version;uint32_t target,contract,config_schema;} zarc_image;
typedef struct {uint32_t target,contract,config_schema;uint64_t minimum_version;} zarc_update_policy;
typedef enum {ZARC_UPDATE_ACTIVE,ZARC_UPDATE_STAGED,ZARC_UPDATE_TRIAL_REQUESTED,
 ZARC_UPDATE_TRIAL,ZARC_UPDATE_RECOVERY_REQUIRED,ZARC_UPDATE_UNAVAILABLE,ZARC_UPDATE_CONFIRMING} zarc_update_phase;
typedef struct {zarc_update_policy policy;zarc_image active,candidate;zarc_update_phase phase;bool configured;} zarc_update;
typedef enum {ZARC_UPDATE_OK,ZARC_UPDATE_DENIED,ZARC_UPDATE_INVALID,ZARC_UPDATE_CONFLICT} zarc_update_status;
/* Portable decision model. Verified flags are facts supplied by the approved
 * boot/metadata and health providers. Not a signature verifier, flash writer
 * or durable boot state. The integrator must persist transitions atomically. */
bool zarc_update_init(zarc_update *,const zarc_update_policy *,const zarc_image *,bool boot_verified);
zarc_update_status zarc_update_stage(zarc_update *,const zarc_image *,bool metadata_verified,bool eligible,bool recovery_verified);
zarc_update_status zarc_update_trial_request(zarc_update *,bool eligible);
zarc_update_status zarc_update_boot_report(zarc_update *,const zarc_image *,bool verified);
zarc_update_status zarc_update_confirm(zarc_update *,bool healthy,bool compatible,bool eligible);
/* Native sequence: persist CONFIRMING before bootloader confirmation, then
 * reconcile verified running identity and observed confirmation. */
zarc_update_status zarc_update_prepare_confirmation(zarc_update *,bool healthy,bool compatible,bool eligible);
zarc_update_status zarc_update_reconcile_confirmation(zarc_update *,const zarc_image *,bool verified,bool confirmed);
void zarc_update_interrupt(zarc_update *);
#endif
