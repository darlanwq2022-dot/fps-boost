# GD FPS Core v0.9.2 — Bismuth Research Build

Windows / Geometry Dash 2.2081 / Geode 5.11.2 / C++23.

## Purpose

This build keeps the established visual reductions and accurate profiler from v0.9.1, and adds an optional one-shot, read-only inventory of objects in the active level. It is a compatibility/research step toward deciding which renderer optimizations can be integrated safely. It does not claim a new FPS gain without an A/B benchmark.

## Bismuth research boundary

Bismuth describes itself as an experimental attempt to rewrite Geometry Dash's rendering pipeline. Its changelog lists shader-sprite optimization, a separate batch-node optimization, and index culling. Its object-rendering architecture groups sprite records by render attributes such as Z layer, spritesheet and blending state. Those are substantial renderer-level changes. Bismuth's own bug notes document at least one LDM-related moving-orb visual-position regression.

GD FPS Core does not copy Bismuth source code and does not replace the original renderer. It borrows the direction of measuring object populations and investigating culling/batching, while keeping original Z order, texture atlases, blending, shaders, trigger visuals and animation semantics under the game's renderer.

References: [Bismuth repository](https://github.com/iliasHDZ/Bismuth), [changelog](https://github.com/iliasHDZ/Bismuth/blob/master/changelog.md), [bug notes](https://github.com/iliasHDZ/Bismuth/blob/master/BUGS.md), [Geode modify tutorial](https://docs.geode-sdk.org/tutorials/modify/).

## New: one-shot level object inventory

Setting: `profile-object-composition` (default OFF).

On the first `PlayLayer::updateVisibility` call with a non-empty object array, the mod performs one read-only scan and logs: total objects, currently visible objects, decoration-marked objects, triggers, High Detail objects, and scan duration. It does not change object state, skip rendering, remove objects, or alter update/physics/collision/trigger/input processing. The scan runs once per PlayLayer to avoid recurring profiler overhead.

This is diagnostic, not an FPS optimization by itself. It helps establish whether decoration population is large enough to justify a future supported culling strategy.

## Existing measurements

- `CCDirector::drawScene` call interval: approximate frame-call cadence, not guaranteed display-present FPS.
- `CCDirector::drawScene` wall duration: elapsed wall time inside the call; it can include waiting/blocking and is not a pure CPU or GPU timer.
- Optional inclusive elapsed-time scopes: `GJBaseGameLayer::update`, `PlayerObject::update`, particle update, visibility update, Wave Trail update, object setup and object parsing.
- Frame budget counts above 16.67 ms, 8.33 ms and 4.17 ms.
- One interval immediately after each report is discarded so synchronous log output is not counted as a game hitch.
- Correct slowest-1%-average estimate for each reporting window.

Nested timing scopes overlap. Do not add `GJBaseGameLayer::update` to `PlayerObject::update` or other nested timings.

## Profiler overhead

Frame cadence is captured on every `drawScene` call while the monitor is enabled. Detailed subsystem timers run on 1 of every N frames (default 4). Disable the profiler for normal gameplay after collecting a baseline. The object inventory performs one scan per PlayLayer only when explicitly enabled.

## Existing optimizations retained

- Suppression of supported particle triggers and stopped optional emitters.
- Native Extra LDM for gameplay parsing, which can omit High Detail objects before normal runtime use.
- Hiding decoration-only nodes and secondary glow/shine/flash visuals. Hidden decoration nodes may remain allocated and may still be touched by game-side object loops; this is not CPU object removal.
- Wave Trail remains enabled. Default settings avoid skipping renderer refreshes and use conservative point thinning.
- No changes to player physics, collision, trigger execution or input loops.

## Not implemented

Not shipped in this build: custom Cocos renderer, static sprite batching, Bismuth-style batch nodes, replacement spatial index, hierarchical culling, custom collision broadphase, GPU timestamp queries, or memory pools. Geode's documented `$modify` example for adding an inherited virtual method via `registerVirtual` explicitly labels that mechanism as not yet supported. Therefore this build deliberately does not ship a `GameObject::visit` override based on it.

A real renderer rewrite must prove equivalence for Z ordering, blending, spritesheet/atlas state, shaders, triggers, rotations, color/alpha/move/follow triggers, and object animation before it can replace any native rendering path.

## Build

Requires Geode SDK 5.11.2 and a C++23 toolchain. Set `GEODE_SDK` and build with CMake/Ninja or `geode build`. A source-package validation is not a substitute for a build with the target SDK. This environment does not include Geode SDK, so a real `.geode` binary has not been produced here.
