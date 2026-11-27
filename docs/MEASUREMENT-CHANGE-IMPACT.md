# Timing measurement and version impact tools

The streaming measurement evaluator checks independently collected cycle timestamps, missing releases, elapsed allocated duration and exclusive deadlines. The G1-C-LAB plan cannot reduce its 24 h and 10 ms deadline acceptance thresholds. Transport evaluation checks independently measured exchange latency and wire-byte utilization in fixed nonoverlapping windows, charging whole exchanges to their start window. It is not a sliding-window or physical utilization proof; agree the window method and clock uncertainty before measurement.

Run `python tools/timing_evidence.py cycles --plan approved-plan.json --trace cycles.csv` or substitute transport. Common plan fields: configuration, board, clock, instrument, procedure. Cycle plan fields: profile, period_us, deadline_us, duration_us; CSV release_us,start_us,completion_us. Transport plan fields: latency_limit_us,window_us,bit_rate,bits_per_byte,utilization_permille,minimum_samples; CSV sent_us,received_us,tx_wire_bytes,rx_wire_bytes. The report hashes the raw trace and plan. A passing result cannot prove acquisition completeness or WCET.

Run `python tools/change_impact.py --base APPROVED_COMMIT --head HEAD` for a conservative direct requirement impact inventory. Unmapped changed files expand review to every requirement. Review transitive dependencies, approved ABI/API decisions and affected golden fixtures independently. No compatibility or release approval is inferred.

Synthetic unit tests establish evaluator behavior only; no physical 24 h or UART measurements have been acquired. Trace candidates RT-010/011, CON-013, VER-007 remain open for accepted evidence.
