# GD FPS Core v0.8.2 — Stability Hotfix

A Geode mod for Geometry Dash Windows 2.2081 targeting visual-workload reduction while preserving native gameplay processing.

## Stability changes
- `PlayLayer::init` is no longer hooked to invoke visibility toggles during construction.
- Native player particle children are attached by the original `PlayerObject::addAllParticles`.
- `GameObject::createAndAddParticle` keeps the engine's original return contract; optional emitters are stopped/hidden after creation.
- The Cocos scheduler is not modified from particle setup/update callbacks.
- `GJBaseGameLayer::updateParticles` always runs, preserving engine bookkeeping.

## Retained optimizations
- Supported particle-trigger and particle-system suppression.
- Player/object glow and secondary-effect suppression.
- Native Extra LDM use for High Detail objects.
- Optional hiding of gameplay decorations without altering trigger objects.
- Wave Trail point reduction and bounded point storage.
- Optional CPU profiling for scene, player, particle, visibility, Wave Trail, object setup and parser paths.

## Important
- This package has not been compiled with the Windows Geode SDK in this environment and has not been launched in-game here.
- If the game still crashes on level entry, send the Geode crash log and specify whether it crashes before the level appears, during loading, or immediately after gameplay starts.
- The mod does not guarantee a specific FPS gain; benchmark on the same level and settings.
