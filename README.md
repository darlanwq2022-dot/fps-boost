# GD FPS Core v0.6.0 — ULTRA Load Shedding

Target: Geometry Dash 2.2081 / Geode 5.11.2 / Windows.

## What this build actually does

- Prevents GameObject particle emitters from being created when particle suppression is enabled.
- Skips attaching player particle systems to the active scene.
- Suppresses particle-trigger visuals, camera shake, object glows, shine, trigger flashes, ring pulses, player burst/flash/circle/death/spawn/dash/ghost visuals and non-Wave trails.
- Forces native Low Detail + Extra LDM in gameplay and passes `lowDetail=true` while parsing level objects. This can filter objects marked High Detail before they enter the live object list.
- Hides editor-flagged decoration objects and optionally skips their `GameObject::update`, provided they are not triggers.
- Keeps the Wave Trail (`PlayerObject::m_waveTrail`, a `HardStreak`) enabled. It keeps fewer points by spacing/sampling and preserves sharper direction changes. Renderer refresh is unthrottled by default for fluid appearance.

## What it deliberately does not do

It does not remove arbitrary active gameplay objects, skip `GJBaseGameLayer::update`, bypass trigger execution, collision checking, player physics or input handling. Doing so can change level behavior or make levels impossible to complete.

A level containing one million objects is not guaranteed to run without lag. High Detail and decoration-only content can be cut down substantially, but live gameplay objects still need memory, updates and collision/trigger processing. The mod cannot safely promise zero lag at any object count.

The Wave Trail is not a sub-1-KB object: the game stores its geometry as `PointNode` entries in a `CCArray`. Sampling reduces the number of points, but the retained history still consumes memory proportional to its length. The engine already prunes old points as the trail moves.

## Build

Extract the archive directly into the root of the GitHub repository, replacing older source files. Keep `mod.json` and `CMakeLists.txt` at the repository root. Build using Geode SDK 5.11.2. This archive contains source only and has not been compiled here.

## First test

Use an object-heavy level with the ULTRA defaults, then compare with the same level with `Skip hidden decoration updates` and `Force native Ultra LDM` turned off one at a time. Keep the frame profiler off during normal gameplay; enable it only for tests. Compare identical sections and settings to avoid misleading results.
