# SFX backlog (Phase 5+)

Fill as you implement. Replace `TBD` with filename or FMOD event id when known.

| ID | Trigger | Status | Notes |
|----|---------|--------|--------|
| SFX-001 | Player footstep | Hooked (asset or synth fallback) | Uses `footstep.wav/.ogg` when present; otherwise generated low-step tone in `platformer.c` |
| SFX-002 | Player jump | Hooked (sample-ready) | Uses file override candidates (`jump.wav`, `jump.ogg`, etc.), otherwise falls back to a softer synthesized swoosh in `platformer.c` (`g_sfx_jump`) |
| SFX-003 | Player land | Hooked (synth) | Runtime-generated tone + land dust |
| SFX-004 | Player hurt | Hooked (synth) | Runtime-generated tone + hit burst |
| SFX-005 | Player death | Hooked (synth) | Runtime-generated tone on respawned death state |
| SFX-006 | Attack whiff | Hooked (sample-ready) | Uses file override candidates (`attack-whiff.wav`, `sword-swing.wav`, etc.), otherwise falls back to a noise/tone whoosh in `platformer.c` |
| SFX-007 | Attack hit enemy | Hooked (sample-ready) | Uses file override candidates (`attack-hit.wav`, `sword-hit.wav`, etc.), otherwise falls back to a layered hit transient in `platformer.c` |
| SFX-008 | Enemy attack telegraph | Hooked (synth) | Runtime-generated tone on caster/cantor windup |
| SFX-009 | Bench sit / rest | Hooked (synth) | Runtime-generated tone on bench interaction |
| SFX-010 | UI confirm | Hooked (synth) | Used in options remap/slider interactions |
| SFX-011 | UI cancel | Hooked (synth) | Used when closing options/canceling remap |
| SFX-012 | Pickup / shard | Hooked (synth) | Used for key/cling/map/lore shard pickups |
