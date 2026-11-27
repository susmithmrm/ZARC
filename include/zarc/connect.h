#ifndef ZARC_CONNECT_H
#define ZARC_CONNECT_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define ZARC_CONNECT_MAJOR 1u
#define ZARC_CONNECT_MINOR 0u
#define ZARC_REQUEST_LIMIT 16u
#define ZARC_FRAME_HEADER 32u
#define ZARC_PAYLOAD_MAX 256u
typedef enum { ZARC_CONN_ACCEPTED, ZARC_CONN_COMPLETED, ZARC_CONN_DENIED,
 ZARC_CONN_BUSY, ZARC_CONN_CONFLICT, ZARC_CONN_INVALID, ZARC_CONN_EXPIRED,
 ZARC_CONN_INCOMPATIBLE, ZARC_CONN_UNAVAILABLE, ZARC_CONN_UNKNOWN_OUTCOME } zarc_conn_status;
typedef enum { ZARC_OP_HELLO=1, ZARC_OP_CAPABILITIES, ZARC_OP_SNAPSHOT,
 ZARC_OP_DOMAIN, ZARC_OP_MODE, ZARC_OP_REQUEST_STATUS, ZARC_OP_SUBSCRIBE,
 ZARC_OP_HEALTH, ZARC_OP_UPDATE_STATUS, ZARC_OP_RESULT, ZARC_OP_EVENT } zarc_operation;
typedef struct { uint64_t session, request_id; uint32_t domain, desired;
 uint8_t operation; uint64_t validity; } zarc_intent;
typedef struct { zarc_intent intent; uint64_t principal, deadline; zarc_conn_status status; } zarc_request_entry;
typedef struct {
 uint64_t boot, session, principal, last_tick, max_validity; size_t count;
 bool connected; zarc_request_entry entries[ZARC_REQUEST_LIMIT];
} zarc_link;
/* Caller is the trusted transport/policy adapter, not an untrusted wire peer.
 * A new boot ID must be supplied externally on restart. No cryptography here.
 * Retain all admitted entries until a new session, refusing overflow. A new
 * session cannot erase outstanding accepted work. */
bool zarc_link_init(zarc_link *, uint64_t boot, uint64_t max_validity, uint64_t now);
zarc_conn_status zarc_link_open(zarc_link *, uint64_t session, uint64_t verified_principal, uint8_t major, uint8_t minor, uint64_t now);
void zarc_link_disconnect(zarc_link *);
zarc_conn_status zarc_link_admit(zarc_link *, const zarc_intent *, bool policy_allowed, bool age_known, uint64_t transit_age, uint64_t now, bool *fresh);
zarc_conn_status zarc_link_result(zarc_link *, uint64_t session, uint8_t operation, uint64_t id, uint64_t now);
zarc_conn_status zarc_link_finish(zarc_link *, uint64_t session, uint8_t operation, uint64_t id, uint64_t now, bool feedback_confirmed);
bool zarc_link_tick(zarc_link *, uint64_t now);
typedef struct { uint8_t operation; uint16_t length; uint64_t boot, session, request_id; uint8_t payload[ZARC_PAYLOAD_MAX]; } zarc_frame;
bool zarc_frame_encode(const zarc_frame *, uint8_t *, size_t capacity, size_t *length);
bool zarc_frame_decode(const uint8_t *, size_t length, zarc_frame *);
typedef struct { uint8_t bytes[ZARC_FRAME_HEADER+ZARC_PAYLOAD_MAX]; size_t used; bool failed; } zarc_stream;
typedef enum { ZARC_STREAM_MORE, ZARC_STREAM_FRAME, ZARC_STREAM_INVALID } zarc_stream_status;
/* Consume at most one frame. Caller retains remainder and closes/reset stream
 * on malformed header. Zero-initialize stream before use. */
zarc_stream_status zarc_stream_feed(zarc_stream *, const uint8_t *, size_t, size_t *consumed, zarc_frame *);
typedef struct { uint64_t boot, session, generation, sequence; uint32_t value; bool valid; } zarc_snapshot_cache;
bool zarc_cache_snapshot(zarc_snapshot_cache *, uint64_t boot, uint64_t session, uint64_t generation, uint64_t sequence, uint32_t value);
bool zarc_cache_event(zarc_snapshot_cache *, uint64_t boot, uint64_t session, uint64_t generation, uint64_t sequence, uint32_t value);
void zarc_cache_invalidate(zarc_snapshot_cache *);
#endif
