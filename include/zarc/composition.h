#ifndef ZARC_COMPOSITION_H
#define ZARC_COMPOSITION_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define ZARC_SERVICE_LIMIT 16u
#define ZARC_DEPENDENCY_LIMIT 8u
typedef enum { ZARC_SERVICE_REGISTERED, ZARC_SERVICE_READY,
 ZARC_SERVICE_QUIESCED, ZARC_SERVICE_SUSPENDED, ZARC_SERVICE_STOPPED,
 ZARC_SERVICE_FAILED, ZARC_SERVICE_BLOCKED } zarc_service_state;
typedef enum { ZARC_COMPOSITION_OK, ZARC_COMPOSITION_DEGRADED,
 ZARC_COMPOSITION_INVALID, ZARC_COMPOSITION_DUPLICATE,
 ZARC_COMPOSITION_MISSING_DEPENDENCY, ZARC_COMPOSITION_CYCLE,
 ZARC_COMPOSITION_ACTIVE } zarc_composition_status;
/* Sole-owner, non-reentrant, bounded callbacks. False init must release its
 * own partial resources. False start is followed by release. Quiesce must
 * resolve accepted work before returning true. False quiesce/resume/suspend
 * must preserve a releasable state; product policy owns physical reactions. */
typedef struct {
 uint32_t id;
 bool required;
 size_t dependency_count;
 uint32_t dependencies[ZARC_DEPENDENCY_LIMIT];
 void *context;
 bool (*init)(void *);
 bool (*start)(void *);
 bool (*quiesce)(void *, uint64_t deadline);
 bool (*suspend)(void *);
 bool (*resume)(void *);
 void (*release)(void *);
} zarc_service_descriptor;
typedef struct {
 zarc_service_descriptor services[ZARC_SERVICE_LIMIT];
 zarc_service_state states[ZARC_SERVICE_LIMIT];
 size_t order[ZARC_SERVICE_LIMIT], count;
 bool acquired[ZARC_SERVICE_LIMIT], configured, active;
} zarc_composition;
/* Zero-initialize before first configure. Descriptors copied, contexts borrowed. */
zarc_composition_status zarc_composition_configure(zarc_composition *, const zarc_service_descriptor *, size_t);
zarc_composition_status zarc_composition_start(zarc_composition *);
zarc_composition_status zarc_composition_quiesce(zarc_composition *, uint64_t now, uint64_t deadline);
zarc_composition_status zarc_composition_suspend(zarc_composition *);
zarc_composition_status zarc_composition_resume(zarc_composition *);
zarc_composition_status zarc_composition_stop(zarc_composition *);
bool zarc_composition_state(const zarc_composition *, uint32_t, zarc_service_state *);
bool zarc_composition_admits(const zarc_composition *, uint32_t);
#endif

