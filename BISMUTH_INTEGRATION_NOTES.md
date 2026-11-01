# Bismuth Integration Notes

## Reviewed upstream ideas
The public Bismuth repository describes itself as an experimental Geometry Dash renderer rewrite. Its v0.1.0-alpha changelog lists shader-sprite optimization, a separate batch-node optimization, and index culling. Its object utility types organize sprites using Z layer, spritesheet, and blending state. The project also records at least one LDM-related moving-orb visual-position bug.

## What this build takes
- The general principle that render-state grouping and culling can reduce renderer work.
- A read-only object-composition inventory to measure the potential value of future index/culling work.
- Benchmark requirements to compare object-heavy and trigger-heavy levels.

## What this build does not take
- No Bismuth source code was copied.
- No replacement renderer, custom batch node, index culling, shader replacement, or Z-order rewrite is included.
- No undocumented inherited virtual hook is used. The Geode modify tutorial presents `registerVirtual` for adding inherited virtual methods as an unsupported/not-yet-implemented pattern.

## Why be conservative
A renderer can be faster in one level and visually wrong in another if Z ordering, texture atlas identity, blending, shaders, alpha/color/move/follow/rotate triggers or child transforms are not reproduced exactly. This project will only add such a path after we can A/B test and fall back to native rendering safely.
