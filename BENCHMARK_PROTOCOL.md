# GD FPS Core benchmark protocol

This protocol is used to distinguish real performance changes from settings, timing noise, and visual-only changes.

## Keep the test controlled

For every A/B run, keep these identical:

- Exact same level and the same section/start position.
- Same game version, resolution, fullscreen/window mode, FPS target and VSync setting.
- Same graphics/Low Detail settings and same set of other Geode mods.
- Same route and approximately the same duration. Prefer a repeatable StartPos or replayable section.
- Same background applications and recording/overlay setup.

Warm up the level for 15 seconds, then capture at least three runs of 30 seconds. Compare the median of the three run summaries, not one best result.

## Pass A: baseline

1. Enable `Frame cadence monitor`.
2. Set `Report interval` to 600 frames.
3. Leave all subsystem profile toggles disabled.
4. Run the chosen section and save the Geode log.

## Pass B: subsystem diagnosis

Enable only one detailed timing option at a time. Use `Subsystem sampling interval = 4` initially. Use `1` only when a short-lived hot spot is being missed; use `8` or `16` when the diagnostic itself affects results.

- Main game update and PlayerObject update are inclusive/nested measurements; do not add them together.
- `drawScene` wall duration can include blocking/waiting and is not a GPU timestamp.
- Particle simulation cannot be diagnosed while global particle suppression is active, because the mod intentionally bypasses the original particle update in that mode. To measure the unmodified particle path, temporarily disable particle suppression and restart.
- Object parsing and custom setup are most useful during level load; capture the transition/loading period as well as gameplay.

## Pass C: one optimization at a time

Start from the baseline and enable exactly one optimization setting per A/B comparison. This identifies which setting changes frame cadence or wall time. Only after individual measurements should settings be combined.

## Read the output correctly

- Average call interval is converted to estimated `drawScene` calls per second. It is not guaranteed display-present FPS.
- p95/p99/max call intervals show stutter and worst-frame behavior.
- 1% low is the inverse of the average duration of the slowest ceil(1%) of samples in that report window.
- Counts above 16.67 ms, 8.33 ms and 4.17 ms show how frequently the sample misses 60, 120 and 240 Hz frame budgets respectively.
- `drawScene` wall duration is a wall-clock measurement. It is not pure CPU time and is not GPU time.
- Detailed subsystem times use sampled frames and can overlap. They are diagnostic scopes, not additive totals.

## Limits

A Geode-level timer cannot guarantee hardware-level GPU timings without a supported graphics API timer-query path. This release does not claim to measure GPU execution, exact monitor presentation, or prove a 10,000-FPS result. For a true GPU bottleneck diagnosis, use an external graphics profiler supported by the GPU/driver.
