# GD FPS Core optimization roadmap

## Implemented and retained

- Native Extra LDM is forced for gameplay parsing. The engine can omit High Detail objects while constructing its normal object set.
- Decoration-only nodes are hidden during gameplay. This reduces rendering/overdraw, but those nodes can remain allocated and may still be touched by game-side loops.
- Supported optional particle emitters/triggers and secondary visual effects are suppressed.
- Wave Trail samples are simplified within configurable limits; its renderer is no longer skipped by default.
- Profiler timings and frame-cadence summaries are opt-in.

## Candidate work that needs measurement and/or deeper engine integration

1. Spatial partition for visibility checks. Must be proven to avoid missing camera movement, zoom, portals and dual-camera behavior.
2. Custom static sprite batching. Requires preserving texture/blend/z-order and invalidation semantics; a naive batch can change visuals or increase GPU cost.
3. More efficient Wave Trail storage. Needs the exact HardStreak ownership/lifetime model before replacing the engine's point container.
4. Object-update culling. The current bindings identify some update methods as inline or part of central engine loops; hiding a node alone does not skip game logic.
5. Texture/asset deduplication and atlas changes. Needs resource lifetime and renderer validation.
6. GPU timer queries. Requires a graphics-backend-specific implementation and must report unavailable on unsupported backends.
7. Memory pools and allocator changes. Must not replace the game's allocator globally without profiling evidence and lifetime audits.

## Acceptance rule

Every optimization must include: (a) a specific hot path measured before, (b) an isolated implementation, (c) A/B timings on the same level and hardware, (d) visual/gameplay regression checks, and (e) a documented fallback. If it cannot demonstrate a repeatable improvement, it stays experimental or is removed.
