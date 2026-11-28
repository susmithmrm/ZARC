# Verification trace and hardware evidence

The [authoritative Z12 register](architecture/12-Requirements-Verification.csv) defines the 178 requirement IDs, design references, verification methods and acceptance conditions. The [requirement trace](REQUIREMENT-TRACE.csv) maps those IDs to candidate implementation/test paths and records the open evidence gate. A candidate path does not claim that a requirement is verified or accepted.

Run the structural check from the repository root:

```sh
python tools/verify_evidence.py --trace-only
```

A release gate additionally requires an externally reviewed evidence JSON, a selected configuration and exact commit. It checks one entry per requirement, approved applicability, verdict, verification method, review fields and hash of a raw evidence artifact. It cannot independently authenticate reviewers or judge the adequacy of evidence; that remains an engineering responsibility. Mandatory ALL requirements cannot be excluded. A release with missing evidence fails.

```sh
python tools/verify_evidence.py --evidence /path/to/evidence.json --configuration approved-bench --commit FULL_COMMIT_SHA
```

## Benign hardware-in-the-loop procedure

Select a lab board, approved independent indicator feedback instrumentation, pinned firmware/build/configuration, and measured clock/uncertainty. Obtain board-owner approval and isolate from real vehicle actuators. Perform nominal admission, link disconnection and independent measured feedback, then repeat for timeout, stale data, resets and saturation. Record electrical and timing evidence separately; no universal safe reaction is prescribed.

The observation evaluator accepts a JSON plan containing board, firmware_sha256, configuration, instrument, clock, procedure, max_feedback_ms and expected_value. The trace is a JSON array of monotonic time_ms/event records: admitted, link_disconnected and feedback with value, quality VALID and source independent_instrument. The evaluator checks a single disconnected-feedback scenario and hashes raw data. It does not operate hardware, prove instrument independence or replace other required HIL tests.

```sh
python tools/hil_observations.py --plan /path/to/approved-plan.json --trace /path/to/raw-trace.json
```

No hardware runs have been performed. Actual target selection, electrical evidence, complete scenario suite, HIL traces, watchdog reset, flash interruption, resource and timing measurements and responsible approval are required.
