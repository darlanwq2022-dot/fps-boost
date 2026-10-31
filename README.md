# GD FPS Core v0.9.0 — Profiling Engine Foundation

Windows / Geometry Dash 2.2081 / Geode 5.11.2.

## Purpose

This release establishes a more trustworthy measurement baseline before adding deeper engine changes. It improves frame-cadence reporting, computes a slowest-1%-average estimate correctly, adds configurable subsystem sampling to reduce profiler overhead, and uses honest timing labels.

## What is measured

- `CCDirector::drawScene` call interval: approximate frame-call cadence, not guaranteed display-present FPS.
- `CCDirector::drawScene` wall duration: elapsed wall time inside the call, which can include waiting/blocking and is not a pure CPU or GPU timer.
- Optional inclusive elapsed-time scopes: `GJBaseGameLayer::update`, `PlayerObject::update`, particle update, visibility update, Wave Trail update, object setup and object parsing.
- Frame budget counts above 16.67 ms, 8.33 ms and 4.17 ms.
- One interval immediately after each report is discarded so synchronous log output is not counted as a game hitch.
- Correct slowest-1%-average estimate for each reporting window.

Nested timing scopes overlap. Do not add `GJBaseGameLayer::update` to `PlayerObject::update` or other nested timings.

## Profiler overhead

Frame cadence is captured on every `drawScene` call while the monitor is enabled. Detailed subsystem timers run on 1 of every N frames (`Subsystem sampling interval`, default 4). Set it to 1 for denser samples or 8/16 for lower diagnostic overhead. Disable the profiler for normal gameplay after collecting a baseline.

## Actual optimizations retained

- Suppression of supported particle triggers and stopped optional emitters.
- Native Extra LDM for gameplay parsing, which can omit High Detail objects before normal runtime use.
- Hiding decoration-only nodes and secondary glow/shine/flash visuals. Hidden decoration nodes may remain allocated and may still be visited by game-side object loops; this is not CPU object removal.
- Wave Trail remains enabled. Default settings now avoid skipping renderer refreshes and use less aggressive point thinning. More aggressive options remain configurable.
- No changes to player physics, collision, triggers or input loops.

## Not implemented in this release

Custom Cocos renderer, static sprite batching, a replacement spatial index, hierarchical culling, custom collision broadphase, GPU timestamp queries, or a memory pool. These require deeper engine-level work and separate validation; the mod does not claim these features.

## Build

Requires Geode SDK 5.11.2 and a C++23 toolchain. Set `GEODE_SDK` and build with CMake/Ninja or `geode build`.

A successful source-package validation is not a substitute for a build with the target SDK. This environment does not include the Geode SDK, so a real `.geode` binary has not been produced here.


### v0.9.1 build hotfix

The percentile-index lambda now captures a local snapshot of the report window size (`sampleCount`) instead of attempting to capture the `FrameWindow::count` member directly. This addresses the Clang error reported by the Windows Geode CI build.
