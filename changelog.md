# Changelog

## v0.3.0
- Replaced the misleading reciprocal of `drawScene` CPU duration with frame-call interval metrics.
- Added avg/p95/p99/max intervals, average frame-call rate, estimated 1% low, and separate `drawScene` CPU avg/p95.
- Added optional visual reductions for GameObject glow sprites and particle emitters.
- Added player visual particle and player glow reductions.
- Preserved `PlayerObject::m_waveTrail`, `m_trailingParticles`, `m_regularTrail`, and `m_shipStreak`.
- Added Geode settings for each option; the settings require restart.
- No changes to physics, collisions, hitboxes, object timing, or inputs.

## v0.2.1
- Fixed Geode SDK pin to 5.11.2.

## v0.2.0
- Added CPU-side drawScene timing monitor.

## v0.1.0
- Initial Geode project foundation.
