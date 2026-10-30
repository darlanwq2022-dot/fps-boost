# GD FPS Core v0.8.1 — MAX ULTRA X Hotfix

**Target:** Geometry Dash 2.2081 / Geode 5.11.2 / Windows.

This release extends the aggressive visual-reduction profile and adds diagnostic timing for the systems that matter most on huge levels. It does not claim to make one million active gameplay objects free: parsing, object-list traversal, physics, triggers, collisions and RAM consumption still have unavoidable costs.

## v0.8.1 hotfix

- Fixed the background-effect visibility call: `toggleBGEffectVisibility` is a `PlayLayer` instance method in the Windows GD 2.2081 bindings, not a `GJBaseGameLayer` static/member call.
- No gameplay logic or optimization behavior was otherwise changed.

## v0.8.0 additions

- Optional suppression of the background-effect visibility path.
- Optional hiding of ground and midground art layers. Both are disabled by default for readability; hiding the art does not disable the ground collision map.
- Per-system CPU timers for `HardStreak::updateStroke`, `GameObject::customSetup`, and `GameObject::objectFromVector`.
- Existing frame cadence, game update, player update, particle update and visibility profiling retained.
- The Wave Trail remains active. Existing sample spacing, corner retention, maximum point count and render-update controls remain configurable.

## Existing MAX ULTRA reductions retained

- Suppress supported particle triggers and stop/unschedule remaining particle systems.
- Prevent supported GameObject and PlayerObject particle emitters from being attached.
- Remove player/object glows, shine, flash, death/spawn/completion, transition-circle, ghost, Spider dash and secondary trail effects where supported.
- Skip cosmetic shader-layer updates and camera shake.
- Request native Extra LDM for gameplay object parsing.
- Hide decoration-only objects in gameplay, while keeping triggers and editor previews untouched.

## Important limits

- `GameObject::update` is inline in the Windows 2.2081 bindings and is not hooked. Decoration sprites can be hidden, but their objects remain allocated and may still be visited by engine code.
- The mod does not replace Cocos2d's renderer, create a custom texture atlas, rewrite draw-call batching, change the engine's collision broadphase, or move unsafe gameplay/rendering work to background threads. Those require substantially deeper engine work and cannot be truthfully advertised as implemented by ordinary hooks.
- The Wave Trail is a `HardStreak`, not a particle system. It is never disabled. The existing point/refresh controls still trade some smoothness/length for lower CPU/memory cost; use conservative settings if visual fidelity is more important.
- Ground/midground hiding is optional and disabled by default. Background effects are disabled by default in this extreme profile.

## Profiling instructions

Enable the profiler options only while diagnosing a level, then disable them for normal play. Check the Geode log for frame cadence, drawScene CPU time, game/player/particle/visibility update time, Wave Trail time, object setup time and object parser time. These are CPU-side measurements; they are **not** GPU timers and do not guarantee presented FPS. Compare the same level, section, resolution and settings before/after.

## Build and validation

Build against Geode SDK 5.11.2 and Windows GD 2.2081 bindings. The v0.8.1 hotfix calls the PlayLayer background-effect method on the correct class. Added PlayLayer hooks (`init`, `toggleGroundVisibility`, `toggleMGVisibility`) correspond to non-inline Windows 2.2081 PlayLayer entries; the background-effect toggle is called as an inline engine helper and is not hooked. Profiling additions use existing hook points already present in this project.

This package contains source and metadata, not a compiled `.geode`. The current environment does not have the Geode SDK/toolchain or Geometry Dash executable, so it cannot be compiled or runtime-benchmarked here. Signatures were checked against the published Windows 2.2081 bindings, and JSON/archive structure is validated locally.
