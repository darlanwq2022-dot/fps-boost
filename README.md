# GD FPS Core v0.3.0 — Extreme Render Profile

Target: Geometry Dash Windows 2.2081 + Geode 5.11.2.

## What's implemented

### Real visual-work reductions

- **Level object glow reduction:** hides each `GameObject`'s separate `m_glowSprite` after object setup and after glow-color updates. This reduces glow overdraw and draw work where levels use many glowing objects.
- **Level object particle rendering reduction:** hides `GameObject::m_particle` after particle activation. This removes its draw work; the particle system may still be scheduled and simulated internally.
- **Player visual particle reduction:** hides the player's ground, landing, click, vehicle, UFO, robot, dash, and swing particle emitters after `PlayerObject::update`.
- **Player glow reduction:** hides the icon and vehicle glow sprites.
- The actual Wave Trail (`m_waveTrail`), trailing particles (`m_trailingParticles`), regular trail, and ship streak are deliberately untouched.

The options are enabled by default for the Extreme profile and require a restart. They can be changed from the mod's Geode settings page. These are visual-only reductions; physics, hitboxes, collision checks, object timing, and inputs are not changed.

### Corrected frame monitor

The previous profiler reported `1000 / drawScene CPU duration` as an FPS-like value. That was misleading. v0.3.0 separates:

- frame-call interval: time between successive `CCDirector::drawScene()` entries;
- p95/p99/max frame-call interval;
- average call rate and a p99-derived 1% low estimate;
- CPU time spent inside `drawScene()` (average and p95).

This measures frame-call pacing and CPU-side draw submission time. It does **not** measure asynchronous GPU execution or prove the exact number of frames physically displayed by the monitor.

## Build

1. Extract this archive directly into the repository root.
2. Confirm `mod.json` is beside `CMakeLists.txt`.
3. Build with the Geode CLI / `geode-sdk/build-geode-mod` GitHub Action.
4. Install the resulting `.geode` file through Geode.

## Important limits

This source package has not been compiled or tested against your local game in this environment. The hooks use the published 2.2081 bindings for `GameObject` and `PlayerObject`, but a successful GitHub Actions build is still required. No numeric FPS gain is claimed until you compare the same level and scene before and after.

The particle options hide rendering; they do not yet stop all particle simulation. More aggressive culling or particle-simulation changes need separate in-game testing because broad Cocos node culling can hide gameplay cues or freeze effects incorrectly.
