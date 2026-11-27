# Common runtime wire dispatcher

A transport-independent owner-side dispatcher integrates the existing frame contract with continuous runtime sessions, discovery, snapshots, event barriers, event polling, typed indicator admission and retained status. It accepts verified transport metadata from a trusted owner; it does not authenticate the MCU link itself. The required scope callback is checked for discovery, individual observations, events, writes and retained results. Unauthorized events are not silently skipped. Old session/principal identities are rejected. The provider reserves effect capacity before dispatch and releases it independently.

Tests cover partial discovery, authorization, freshness, different principal, observed result and disconnect. Production transport trust, complete schemas, multi-client subscription policy and physical provider ownership remain open.
