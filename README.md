# GD FPS Core — v0.2.0

Target: Geometry Dash Windows 2.2081 + Geode.

## Implemented

This version adds a frame-time profiler by modifying `cocos2d::CCDirector::drawScene()` and timing the original call with `std::chrono::steady_clock`.

Every 120 frames, the Geode log receives:
- average CPU-side `drawScene` duration;
- 95th-percentile duration (p95);
- maximum duration in the sample window;
- reciprocal of average duration, shown as an indicative FPS figure.

### Important measurement limits

This is a CPU-side wall-clock measurement around `drawScene`, not a GPU timer. It measures time spent by the CPU in the function and the graphics-driver submission path; asynchronous GPU work may continue after the function returns. The reciprocal-average figure is therefore not a replacement for an in-game FPS counter or a GPU profiler. Compare results in the same scene and conditions.

## Safety constraints

- Preserve the Wave Trail.
- Do not change physics, collision, hitboxes, player input, or object timing.
- Do not globally disable particle systems: some are gameplay-relevant or owned by the player.
- Do not hook undocumented methods or assume a node is decorative based only on its class.
- No NOVA dependency.
- The profiler calls the original `drawScene()` exactly once and does not alter its return flow (the function is `void`).

## Build (Windows)

1. Install the Geode SDK and CLI for the target Geometry Dash version.
2. Open a terminal in this folder.
3. Run `geode build`.

Alternatively, configure with CMake after setting `GEODE_SDK` to the SDK directory.

## Next stages

1. Use profiler data from dense levels to identify whether CPU frame time is a bottleneck.
2. Add opt-in, narrowly scoped visual-effect controls only for effects whose ownership and gameplay role are verified.
3. Investigate Wave Trail optimization separately, preserving its visible path and timing.
4. Compare frame-time distributions before and after each change.

No performance gain is claimed until measured in-game. This source package has not been compiled against your local Geode SDK or tested in Geometry Dash in this environment.
