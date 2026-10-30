# Changelog

## v0.2.1
- Fixed `mod.json` Geode SDK requirement: `5.11.2` replaces the literal `$GEODE_VERSION` that caused the CI tag lookup failure.
- Updated the CMake project version to `0.2.1`.
- This fixes the specific SDK tag lookup error in the supplied log; a successful compile still needs confirmation from GitHub Actions.

# Changelog

## v0.2.0
- Added a CCDirector::drawScene CPU-side frame-time profiler.
- Reports average, 95th percentile, maximum, and reciprocal-average FPS every 120 frames.
- Preserves original drawScene call order and invokes the original exactly once.
- No gameplay, physics, collision, input, particle, or Wave Trail behavior changes.

## v0.1.0
- Initial Geode project foundation.
