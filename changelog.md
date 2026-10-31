# v0.9.0 — Profiling Engine Foundation

- Discards the frame-call interval immediately after a synchronous report, so profiler logging is not mistaken for a game hitch.
- Improved frame report: average/min/p95/p99/max `drawScene` call cadence, threshold counts and a slowest-1%-average estimate.
- Clarified that `drawScene` interval is a frame-call cadence estimate, not GPU-present FPS.
- Clarified wall-time/inclusive-time labels; nested timing scopes overlap.
- Added configurable detailed subsystem sampling (default 1/4 frames) to reduce diagnostic overhead.
- Made detailed profiler metrics divide by sampled frames, not all frames.
- Fixed the Wave Trail point cap so it remains a hard cap even if count overshoots by more than one point.
- Changed Wave Trail defaults to preserve each renderer refresh and retain more geometry by default.
- Avoids repeating writes to native LDM flags for every parsed object once the flags are already enabled.
- No changes to physics, collisions, triggers or input processing.

# v0.8.2 — Stability Hotfix

- Preserved native PlayLayer construction and Cocos scheduler bookkeeping.
- Preserved particle factory return values and player particle-node attachment.
- Retained supported visual reductions and optional subsystem profiling.
