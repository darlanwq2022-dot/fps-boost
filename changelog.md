# Changelog

## v0.7.1 — MAX ULTRA+
- Added a direct skip for `GJBaseGameLayer::updateParticles(float)` while global particle suppression is enabled, cutting the layer-level per-frame particle update path.
- Added an opt-in setting (enabled by default) to skip cosmetic `GJBaseGameLayer::updateShaderLayer(float)` updates.
- Updated startup diagnostics and mod metadata for the new visual paths.
- Audited the new hook signatures against the published Windows 2.2081 bindings.
- Clarified that decoration-only objects are hidden but remain allocated and are not claimed to have their `GameObject::update` calls skipped.

## v0.7.0 — MAX ULTRA
- Reconnected supported PlayerObject visual hooks using Windows 2.2081 non-inline methods.
- Suppressed the hookable GJBaseGameLayer particle-trigger visual path and prevented player/object particle emitters from attaching; stopped and unscheduled remaining particle systems.
- Suppressed supported bump, death, spawn, completion, Spider dash, ghost, transition-circle, swing-fire, and non-Wave dart-streak visuals.
- Avoided glow-color updates when object glows are disabled.
- Used native Extra LDM and hid decoration-only objects during gameplay without touching triggers or editor previews.
- Kept the Wave Trail active with point decimation, a hard point cap and reduced renderer refresh frequency.
- Kept gameplay update, trigger execution, collision, physics and input loops intact.

## v0.6.1
- Removed confirmed inline-only hooks that failed compilation.
- Retained native Extra LDM, decoration suppression, particle suppression and Wave Trail optimization.
