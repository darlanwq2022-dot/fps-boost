# Changelog

## v0.8.0 — MAX ULTRA X
- Added an optional background-effect suppression path, applied when PlayLayer initializes.
- Added optional ground and midground art-layer hiding, while preserving the collision/gameplay world.
- Added opt-in CPU profiling for HardStreak::updateStroke, GameObject::customSetup and GameObject::objectFromVector.
- Extended the frame profiler activation condition to include the new diagnostic modes.
- Kept the Wave Trail enabled and retained the previous sampling and point-cap controls.
- Updated docs to distinguish implemented hooks from deeper renderer/engine rewrites that are not part of this release.
- Checked added PlayLayer hook signatures against Windows Geometry Dash 2.2081 bindings.

## v0.7.1 — MAX ULTRA+
- Added a direct skip for GJBaseGameLayer::updateParticles(float) while global particle suppression is enabled.
- Added shader-layer animation suppression.
- Clarified that decorative objects are hidden, not removed from memory or skipped in GameObject::update.

## v0.7.0 — MAX ULTRA
- Reconnected supported PlayerObject visual hooks using Windows 2.2081 non-inline methods.
- Suppressed supported particle triggers and prevented player/object particle emitters from attaching.
- Suppressed supported bump, death, spawn, completion, Spider dash, ghost, transition-circle, swing-fire and non-Wave dart-streak visuals.
- Used native Extra LDM and hid decoration-only objects during gameplay without touching triggers or editor previews.
- Kept the Wave Trail active with point decimation, a hard point cap and reduced renderer refresh frequency.

## v0.6.1
- Removed confirmed inline-only hooks that failed compilation.
