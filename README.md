# GD FPS Core v0.7.1 — MAX ULTRA+

**Target:** Geometry Dash 2.2081 / Geode 5.11.2 / Windows.

This is an aggressive visual-load reduction profile. It intentionally sacrifices particle effects, glows, shine, secondary player effects, camera shake, shader-layer animation and decorative rendering while preserving the Wave Trail.

## v0.7.1 additions

- Hooks `GJBaseGameLayer::updateParticles(float)` and skips the central per-frame particle-update path when **Disable EVERY CCParticleSystem effect** is enabled.
- Adds `Disable shader-layer updates`, which skips `GJBaseGameLayer::updateShaderLayer(float)` by default. This removes cosmetic shader animation work; shader-driven background effects may stop animating.
- Retains the v0.7.0 particle creation guards, Cocos particle stopping/unscheduling, player-effect suppression, object glow/shine suppression and native Extra LDM request.
- Keeps the Wave Trail active, with a bounded point array, distance/sample decimation, corner preservation and reduced geometry refresh frequency.

## Aggressive visual reductions

- Suppresses the hookable `GJBaseGameLayer::spawnParticleTrigger(int, ...)` visual entry point.
- Skips `GJBaseGameLayer::updateParticles(float)` while global particle suppression is enabled.
- Prevents `GameObject` particle emitter creation using the native `m_hasNoParticles` guard.
- Prevents player particle attachment before emitters are created.
- Stops, hides and unschedules remaining `CCParticleSystem` nodes.
- Suppresses supported player visuals: glow refresh, swing fire, non-Wave dart streaks, bump/death/spawn/completion effects, Spider dash visuals, ghost visuals and transition circles.
- Suppresses object glows and shine animations, ring pulses, trigger flashes, gravity-transition visuals, scene glitter and camera shake.
- Skips shader-layer animation updates when the new setting is enabled.
- Requests Geometry Dash's native Low Detail + Extra LDM path for gameplay object parsing.
- Hides decoration-only objects during gameplay. Trigger objects and editor previews are excluded.

## Wave Trail preservation

The Wave Trail is a `HardStreak`, not a `CCParticleSystem`, and is deliberately not disabled. Its sample stream is decimated, nearby points are dropped, sharp turns are preferentially retained, the retained point array is capped (default: 96), and geometry refresh is performed every second scheduled update by default. These settings trade some smoothness/length for lower CPU and memory overhead. The trail is not sub-1-KB: Cocos point-array entries and draw geometry have object/allocation overhead beyond the raw coordinates.

## Gameplay behavior deliberately preserved

The mod does not bypass `GJBaseGameLayer::update`, player update/physics, collision checks, trigger dispatch, object activation, or input handling. `GameObject::update` is inline in the Windows 2.2081 bindings and is not hooked. Hiding decorative objects reduces rendering/overdraw, but those objects remain allocated in level arrays and may still contribute to object traversal or other engine overhead.

No mod can guarantee zero lag for a level containing one million objects. Even when visual work is removed, parsing, RAM use, object-list traversal, active gameplay-object processing, triggers, collisions, and engine internals remain. This release aims to reduce avoidable visual CPU/GPU work without changing gameplay timing or physics.

## Build and validation

Build against Geode SDK 5.11.2 and Windows GD 2.2081 bindings. The v0.7.1 hooks added here (`GJBaseGameLayer::updateParticles` and `updateShaderLayer`) have non-inline Windows 2.2081 entries in the official bindings. The previously reported inline-only methods remain excluded: `GJBaseGameLayer::spawnParticleTrigger(SpawnParticleGameObject*)`, `PlayerObject::flashPlayer`, `PlayerObject::playBurstEffect`, and `PlayerObject::spawnCircle2`. `GameObject::update` is also left unhooked because its Windows binding is inline.

The project metadata and archive structure have been validated, and hook signatures were checked against the published Windows 2.2081 bindings. **This environment does not contain the Geode SDK/toolchain or Geometry Dash executable, so this archive has not been compiled or runtime-benchmarked here.**
