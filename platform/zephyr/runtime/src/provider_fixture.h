#ifndef ZARC_PROFILE_PROVIDER_FIXTURE_H
#define ZARC_PROFILE_PROVIDER_FIXTURE_H
#include "zarc/runtime.h"
/* Independent simulated provider owner. Native queues copy immutable commands.
 * No physical hardware is operated. These are bounded laboratory calls, not
 * a qualified execution-time or scheduling allocation. */
bool zarc_profile_provider_start(const zarc_endpoint_config *,size_t count);
bool zarc_profile_provider_sample(zarc_runtime_sample *,size_t count);
bool zarc_profile_provider_effect(const zarc_controller_effect *,bool *applied);
bool zarc_profile_provider_stop(void);
#endif
