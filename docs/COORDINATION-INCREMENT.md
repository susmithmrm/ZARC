# Portable coordination models

Update state tracks the active verified image, candidate metadata, trial request, reported boot, confirmation and recovery. Compatibility is an explicit exact target/contract/schema policy with minimum version and strictly newer candidate. The fixture supplies verification facts and recovery eligibility; the model never verifies signatures or writes flash. Interruption requests recovery and cannot declare the old image active until the approved boot provider reports it. Persisting the transition, actual boot selection and anti-rollback hardware policy are not implemented here.

Power coordination aggregates required guest readiness within a deadline. Reports carry epoch and request identity; loss of readiness revokes the aggregate; timeout produces a fallback indication rather than an electrical action. Product policy must authorize actions, verify readiness freshness and recheck the local state before any physical transition. This is not AAOS VHAL or actual VM/rail control.

Trace candidates: ASR-006/007, HOST-008, SVC-005/011. Tests are host decision fixtures only. Real boot trust, signed update stack, durable coordination, power eligibility, hardware flash/recovery and vehicle allocation remain open.
