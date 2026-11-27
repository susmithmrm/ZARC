#ifndef ZARC_MAINTENANCE_FIXTURE_H
#define ZARC_MAINTENANCE_FIXTURE_H
#include "zarc/diagnostics.h"
/* Native simulator partition only. Call before starting and after stopping
 * the runtime/device worker. Never write flash in the active cycle path. */
bool zarc_profile_maintenance_open(uint64_t *boot,zarc_diagnostics *restored);
bool zarc_profile_maintenance_close(const zarc_diagnostics *snapshot);
#endif
