# Changelog

## v0.8.2 — Stability Hotfix
- Removed direct visibility calls from `PlayLayer::init` to avoid touching layer state during fragile construction.
- Restored native `PlayerObject::addAllParticles` behavior.
- Preserved the original `GameObject::createAndAddParticle` return value; optional emitters are suppressed after creation.
- Stopped changing Cocos scheduler registrations from particle setup/update callbacks.
- Restored `GJBaseGameLayer::updateParticles` so the game maintains its own particle bookkeeping.
- Kept supported visual suppression, object LDM/decorative options, Wave Trail handling, and profiler hooks.

This is a stability-first hotfix. It may give up a small amount of the previous particle-creation/update optimization in exchange for safer level initialization.
