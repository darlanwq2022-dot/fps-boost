# Optimization Roadmap — v0.9.3

## Shipped
- Aggressive optional suppression of supported particles and secondary effects.
- Native Extra LDM and optional hiding of decoration-only objects.
- Optional scene reductions for ground, midground, animated background effects and attempts HUD.
- Optional subsystem profiler and one-shot object inventory.
- Conservative Wave Trail sampling; Wave Trail remains visible.

## Next high-value research targets
1. Measure `PlayLayer::updateVisibility` and object-update cost on object-heavy levels.
2. Determine whether a supported spatial index can accelerate visibility without stale results from move/rotate/follow triggers.
3. Investigate renderer batching only for sprites with identical Z order, texture atlas, blend mode and shader state.
4. Profile allocations before considering pools; Cocos reference counting makes unsafe pooling a crash risk.
5. Investigate shader-state caching only where trigger-driven shader changes remain correct.

## Do not ship without proof
- Do not remove active gameplay objects from engine lists.
- Do not skip trigger, collision, physics, player or input updates.
- Do not override inherited `visit` using unsupported virtual-registration paths.
- Do not claim GPU FPS from `drawScene` cadence.

## Acceptance criteria
- Full Windows build with the exact target Geode SDK succeeds.
- Three repeatable A/B runs per setting show better average and tail frametime.
- No incorrect Z order, missing trigger visuals that convey gameplay, disappearing objects, input loss, collision changes or Wave Trail regressions.
