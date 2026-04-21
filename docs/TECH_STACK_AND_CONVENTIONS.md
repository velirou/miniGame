# Tech stack and conventions

## Current stack (repository default)

| Layer | Choice | Notes |
|-------|--------|--------|
| Language | **C** | As in existing `platformer.c` |
| Graphics / window / input | **raylib** | Cross-platform; good for 2D |
| Build | **TBD** — document when added (Make, CMake, or IDE project) | Add build instructions here |
| Version control | **git** | |

**Engine migration:** Switching to C++, C#, Rust, Godot, etc. is allowed but is a **project decision**: update this file, note migration date, and adjust `AGENTS.md` / Cursor rules.

---

## Directory layout (target — create as needed)

```
miniGame/
  AGENTS.md
  docs/                 # vision, milestones, prompts (source of truth)
  src/                  # prefer moving .c files here when project grows
  assets/
    textures/
    audio/
    tilesets/           # if using Tiled or similar later
  build/                # local only; gitignored
```

Current layout keeps `platformer.c` at root as the gameplay entrypoint, with shared helpers in `src/` and headers in `include/`.

---

## Code conventions

- **Naming:** `snake_case` for functions and locals; `PascalCase` or `UPPER_SNAKE` for types/macros consistent with existing file.
- **Files:** one `main` entry; split modules when a file exceeds ~500–800 lines or when domains mix (e.g. `player.c`, `world.c`, `save.c`).
- **Headers:** include guards or `#pragma once` if you introduce headers.
- **Constants:** tune gravity, speed, i-frame duration at top of module or a small `tuning.h`.
- **Debug:** wrap debug draws in `#ifdef DEBUG` or a runtime flag; document in build section.

---

## Gameplay tuning log

When you change feel-critical numbers, add one line:

| Date | Constant | Old | New | Reason |
|------|----------|-----|-----|--------|
| 2026-04-19 | Phase 1 batch | — | `platformer.c` tunables block | Initial movement / combat tuning |
| 2026-04-21 | Tank/Dasher aggro speeds | see `platformer.c` | lower chase + strike | Common enemies closed too fast on aggro; Cantor unchanged |
| YYYY-MM-DD | e.g. `JUMP_VELOCITY` | | | e.g. follow-up playtest |

(Optional: small table at bottom of this file.)

---

## Save data (when implemented)

- **Version field** first in file or blob; bump on format change.
- **Migration:** if version mismatch, attempt migration or show “new game” with message.
- **Steam Cloud:** only after local saves are stable.

---

## Definition of Done (code task)

A gameplay task is **done** when:

1. It maps to a **milestone exit** or explicit sub-task in `docs/MILESTONES.md`.
2. **Manual test steps** are listed (in PR description or commit message).
3. No new **undocumented** tunables; critical numbers live in one discoverable place.
4. Debug-only code is gated.

---

## Builds and releases

**Phase 1 — Windows (MinGW-w64, raylib installed):** from repo root, with `raylib` and MinGW on `PATH`:

```text
gcc -o platformer.exe platformer.c src/assets_loader.c -lraylib -lopengl32 -lgdi32 -lwinmm -static-libgcc
```

Run `platformer.exe` with working directory set to the project folder (or wherever the exe lives). PNGs may live in **`assets/`** or the same folder as the executable — preferred and fallback names are centralized in `src/assets_loader.c` (`load_first_*` lists). Placeholders are used when files are missing.

**Coyote time / jump buffer:** tuned in `platformer.c` (search `COYOTE_`, `JUMP_BUFFER_`, or the `Player` timers `coyote_timer` / `jump_buffer_timer`).

**Other platforms:** link `raylib` per upstream docs; working directory must resolve `assets/` paths.

```text
# Debug (example — replace with real commands)
# cmake -B build && cmake --build build

# Release: [steps]
```

---

## Assets

- Prefer **power-of-two** textures where relevant; document sprite frame counts next to filenames.
- Asset filename aliases are centralized in `src/assets_loader.c` (`load_first_hero`, `load_first_attack`, `load_first_tile`, `load_first_enemy`, `load_first_map`) — update those lists when renaming.
- **Hero sheet:** a **64×32** image is treated as **one** pose (not split into two 32×32 frames).

---

## Accessibility checklist (Phase 5)

- [x] Runtime key remap for core keyboard actions (move, jump, sprint, attack, interact, map)
- [x] Master/music/SFX volume controls available in options
- [x] Optional screenshake toggle (default on)
- [x] Objective text gives directional guidance for current progression step

---

## Testing checklist (minimal)

Before merging a Phase 1–3 gameplay PR:

- [ ] 60 FPS on target machine (state GPU/CPU in PR if borderline)
- [ ] No crash on room transition / death / save
- [ ] Critical path still completable (dev smoke test)
