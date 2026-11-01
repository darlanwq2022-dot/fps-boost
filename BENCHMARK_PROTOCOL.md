# GD FPS Core — Benchmark Protocol

## Baseline setup
1. Use the same Geometry Dash version, Geode version, resolution, fullscreen/windowed mode, FPS limit, and graphics driver.
2. Use one repeatable level and the same start position or section.
3. Warm up the level once, then collect at least three runs with the profiler disabled for normal performance numbers.
4. Repeat three runs with only the specific optimization under test enabled.
5. Compare median average cadence, p95, p99, 1% low estimate, and `drawScene` wall duration. Compare each run separately; do not combine data from different scenes.

## Diagnostic runs
- Enable `frame-monitor` for frame-call cadence and wall duration.
- Enable one or two subsystem timers at a time. They are inclusive and can overlap; do not add them together.
- Set subsystem sampling interval to 4 or 8 for lower profiler overhead.
- Enable `profile-object-composition` only for one diagnostic run. It scans the object array once per PlayLayer and logs total/visible/decoration/trigger/High Detail counts and scan duration. Turn it back off for FPS benchmarks.

## Interpretation
- `drawScene` call cadence is not a guaranteed display-present FPS measurement and is not a GPU timestamp.
- A high object count does not prove that object updates are the bottleneck. Use visibility/update/parse timings.
- A high decoration count makes culling a candidate, not a proven win.
- Accept an optimization only if it improves repeatable frame metrics across several levels and does not cause visual or gameplay regressions.
- Test at least one object-heavy level, one trigger-heavy level, and one level with heavy shader/blending effects before enabling renderer-related changes globally.
