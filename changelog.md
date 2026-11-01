# Changelog

## v0.9.2 — Bismuth Research Build
- Added an opt-in, read-only, one-shot level object inventory (total, visible, decoration-marked, trigger, High Detail, and scan time).
- Removed the experimental inherited `GameObject::visit` override after checking Geode documentation: `registerVirtual` for adding inherited virtual functions is explicitly documented as not yet supported.
- Kept the native Geometry Dash renderer and gameplay paths intact.
- Retained v0.9.1 profiler corrections and conservative Wave Trail defaults.
- No FPS gain is claimed from the object inventory; use it to guide future A/B-tested renderer work.
