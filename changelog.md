# Changelog

## v0.9.4 — Presets + Smart Skip
- New `Performance preset` (Custom / Balanced / Extreme / Insane). Presets override the toggles at load; Wave Trail is never touched.
- New EXPERIMENTAL `Visibility update step`: runs the native PlayLayer visibility pass every N frames (forced on level reset). Gameplay, triggers, collision, physics and input still run every frame. May cause brief pop-in; benchmark per level.
- New `Progress bar/percent update step`: thins cosmetic HUD refresh.
- New optional Windows `Above-normal process priority`.
- Not yet built against Geode SDK 5.11.2; needs CI build + A/B benchmark.

## v0.9.3 — Extreme Scene Controls
- Added independent controls to hide the ground and midground layers.
- Added a control to disable the separate animated background-effect layer.
- Added an option to hide the attempts counter.
- Applies scene reductions after `PlayLayer::onEnterTransitionDidFinish`, avoiding partial-construction lifecycle changes.
- Filters later visibility requests only for the selected visual layers; gameplay objects, triggers, collisions, physics and inputs remain on the native path.
- These are real visual-work reductions, but ground/midground/HUD savings may be small and must be benchmarked per level.
- Retains the original renderer and the Wave Trail.
