# Agent instructions (miniGame)

You are helping build a **small Steam Metroidvania RPG**. *Hollow Knight* is a **reference for feel** (exploration, mood, melee movement loop) — **not** a feature or content target.

## Mandatory reads (in order)

1. `docs/GAME_VISION_AND_SCOPE.md` — scope, pillars, v1 boundaries  
2. `docs/MILESTONES.md` — current phase and exit criteria  
3. `docs/TECH_STACK_AND_CONVENTIONS.md` — stack, layout, done definition  

For copy-paste LLM prompts, see `docs/MASTER_PROMPTS.md`.

## Hard rules

- **Do not expand scope** (extra biomes, large crafting, huge enemy lists, parallel RPG systems) unless the user explicitly says so in the same request.
- **Complete the active milestone exit** before adding “more map” or “more features.” Phase 1 (feel + collision + camera) blocks content-heavy work.
- **Minimize diffs:** touch only files needed for the task; no drive-by refactors.
- **Every task** should state which **phase (0–6)** it advances and how to **manually test** it.

## Where truth lives

| Topic | File |
|-------|------|
| What we’re building | `docs/GAME_VISION_AND_SCOPE.md` |
| Order of work | `docs/MILESTONES.md` |
| Prompts for agents | `docs/MASTER_PROMPTS.md` |
| Tech / build / conventions | `docs/TECH_STACK_AND_CONVENTIONS.md` |
| PNG filenames & load order | `src/assets_loader.c` (`load_first_*` lists), optional folder `assets/` |

## Phase 0 (locked)

- **Title (working):** The Sunken Choir  
- **Fantasy:** pilgrim-knight descends to end a hymn in the Choir Below  
- **Unlocks:** Veil Drift (mini-boss), Chord Cling (pickup), Cathedral Key (gated path)  
- **World:** 18 rooms — `docs/world_graph.md`  
- **RPG spine:** nail upgrades (3 tiers), one optional health shard  

## Current codebase note

The repo may contain an early **raylib + C** prototype (e.g. `platformer.c`). Treat it as **Phase 1** foundation until the world graph and bench loop land.
