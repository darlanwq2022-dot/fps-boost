# GD FPS Core v0.9.4 — Presets + Smart Skip

Windows / Geometry Dash 2.2081 / Geode 5.11.2 / C++23.

## What changed in v0.9.3

This iteration adds four independent scene/UI reduction settings using documented `PlayLayer` virtual methods: hide the ground layer, hide the midground layer, disable the separate animated background-effect layer, and hide the attempts counter. The settings are applied after `PlayLayer::onEnterTransitionDidFinish`, not during construction. Later engine requests to show a disabled visual layer are filtered to keep the selected reduction active.

These options remove actual visual nodes/layers from the gameplay presentation and can reduce draw/overdraw or animation work. The expected gain is level- and GPU-dependent; the attempts counter and ground/midground savings may be small. They are not advertised as a guaranteed large FPS multiplier.

## Preserved behavior

- Native Geometry Dash renderer, object ordering, atlas selection and blending remain in use.
- Player physics, collision, trigger dispatch, input handling and gameplay object updates are not bypassed.
- Wave Trail remains enabled and visible. Existing conservative Wave Trail sampling controls remain configurable.
- Existing particle, glow, shader-layer, flash, LDM and decoration reductions remain available.

## Existing profiler

The optional profiler reports `drawScene` call cadence/wall duration and sampled subsystem timings. It is not a GPU timestamp or guaranteed monitor-present FPS counter. Disable profiling for normal gameplay after collecting a baseline. The one-shot object inventory is diagnostic only and does not itself improve FPS.

## Bismuth boundary

Bismuth's batching, shader-sprite and index-culling concepts remain research targets. This build does not replace the Cocos renderer or copy Bismuth's experimental renderer path. A renderer rewrite needs separate validation for Z order, blending, texture atlases, shader state, animated objects, move/follow triggers and visibility invalidation.

## Build and validation

Requires Geode SDK 5.11.2 and a C++23 toolchain. Set `GEODE_SDK` and build with CMake/Ninja or `geode build`. This packaging environment does not contain the target Geode SDK, so the source can be statically validated here but a full Windows `.geode` build and in-game benchmark still need to run in the project's CI.

## v0.9.4 additions
Presets, experimental visibility/HUD update skipping and optional process priority. The custom batch renderer and spatial-index culling from Bismuth are still NOT implemented: they require building and testing against the real Geode SDK.
