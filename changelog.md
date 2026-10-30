# Changelog

## v0.6.0 — ULTRA Load Shedding
- Blocks GameObject particle-emitter creation using the native `m_hasNoParticles` guard.
- Skips attaching player particle systems when particle suppression is enabled.
- Suppresses particle-trigger visuals and camera shake.
- Skips per-frame updates only for hidden decoration-only GameObjects that are not triggers.
- Adds player burst and flash/circle suppression hooks.
- Keeps native Extra LDM filtering of High Detail objects.
- Keeps the Wave Trail visible; aggressive point sampling remains, but renderer refresh defaults to 1 for a fluid trail.
- Does not bypass gameplay-object updates, triggers, collision checks, physics or input handling.

## Limitations
- Decoration objects remain allocated; this is not a memory-freeing object remover.
- High Detail filtering only removes objects marked High Detail by the level author.
- A level with extremely large numbers of active gameplay objects can still be CPU- or memory-bound. No mod can guarantee zero lag for one million active objects while preserving the game's full simulation.
- Wave Trail points are still stored as engine PointNode objects; reducing samples lowers their count, but cannot make the trail consume less than 1 KB in all cases.
- This package is source-only and has not been compiled in this environment. Build it with Geode SDK 5.11.2 / GitHub Actions before installing.
