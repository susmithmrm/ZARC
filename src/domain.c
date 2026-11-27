#include "zarc/domain.h"
#include <stddef.h>

void zarc_init(zarc_domain *d) {
    if (d != NULL) { *d = (zarc_domain){0}; d->result = ZARC_UNKNOWN_OUTCOME; }
}

zarc_status zarc_tick(zarc_domain *d, uint64_t tick) {
    if (d == NULL || tick < d->last_tick) return ZARC_INVALID;
    d->last_tick = tick;
    if (d->pending && tick >= d->deadline) {
        d->pending = false;
        d->result = ZARC_UNKNOWN_OUTCOME;
        if (d->observed != ZARC_UNKNOWN) { d->observed = ZARC_UNKNOWN; ++d->generation; }
    }
    return d->result;
}

zarc_status zarc_request(zarc_domain *d, bool allowed, uint64_t id, zarc_value desired,
                         uint64_t tick, uint64_t deadline, zarc_effect *effect) {
    if (effect == NULL) return ZARC_INVALID;
    *effect = (zarc_effect){0};
    if (d == NULL || tick < d->last_tick || (desired != ZARC_OFF && desired != ZARC_ON)) return ZARC_INVALID;
    if (!allowed) return ZARC_DENIED;
    (void)zarc_tick(d, tick);
    if (d->retained && id == d->request_id)
        return desired == d->desired ? d->result : ZARC_CONFLICT;
    if (d->pending) return ZARC_BUSY;
    if (deadline <= tick) return ZARC_INVALID;
    d->retained = true;
    d->pending = true;
    d->request_id = id;
    d->desired = desired;
    d->deadline = deadline;
    d->result = ZARC_ACCEPTED;
    effect->emitted = true;
    effect->desired = desired;
    return d->result;
}

zarc_status zarc_feedback(zarc_domain *d, zarc_value value, uint64_t tick) {
    if (d == NULL || tick < d->last_tick || (value != ZARC_OFF && value != ZARC_ON && value != ZARC_FAULT)) return ZARC_INVALID;
    (void)zarc_tick(d, tick);
    if (d->observed != value) { d->observed = value; ++d->generation; }
    if (d->pending && value == d->desired) {
        d->pending = false;
        d->result = ZARC_COMPLETED;
    } else if (d->pending && value == ZARC_FAULT) {
        d->pending = false;
        d->result = ZARC_UNKNOWN_OUTCOME;
    }
    return d->result;
}
