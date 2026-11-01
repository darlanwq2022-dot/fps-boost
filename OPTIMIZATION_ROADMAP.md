# Optimization Roadmap

## Current baseline (v0.9.2)
- Visual reduction: particle suppression, shader-layer update suppression, glow/shine/flash and secondary effects, native Extra LDM, optional decoration hiding.
- Profiling: drawScene cadence/wall duration, update/particle/visibility/Wave Trail/setup/parser timers, one-shot object composition inventory.
- Wave Trail is preserved with conservative point thinning; no default renderer-frame skipping.

## Bismuth ideas under research
Bismuth's changelog identifies shader-sprite optimization, separate batch nodes and index culling. Its object batching uses Z layer, spritesheet and blending as render-state keys. These are renderer-level optimizations and cannot safely be transplanted as isolated snippets.

1. **Object/index analysis:** correlate level object composition with `PlayLayer::updateVisibility` and update timings. Build a spatial index only if a supported, low-overhead hook and correct invalidation strategy are identified.
2. **Safe culling:** evaluate whether native visibility already does enough; any new culling must account for child visuals, move/rotate/follow triggers, camera transforms and objects entering the viewport.
3. **Batching:** investigate grouping only where atlas, Z order, blend mode, shader and animation semantics are identical. Preserve the original renderer as fallback.
4. **Shader optimization:** cache/reuse shader state only after confirming Geode/Cocos APIs and trigger-driven shader changes. Never globally skip shader work if it changes visible gameplay feedback.
5. **Wave Trail:** profile first; a custom ring buffer/renderer requires an exact compatibility layer for streak geometry and should not be approximated by deleting points aggressively.
6. **Memory:** instrument allocations before introducing pools; pooling objects with Cocos reference counting can cause use-after-free and stale state.

## Acceptance criteria
- Full Windows Geode 5.11.2 / GD 2.2081 CI build succeeds.
- Three repeatable A/B runs show a measurable improvement in average and tail frametime.
- No incorrect object positions, Z-order changes, disappearing triggers, collision changes, missed inputs or Wave Trail regressions.
- Feature is optional and can be disabled per user profile until broad level testing is complete.
