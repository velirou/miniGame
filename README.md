# miniGame

Small **Steam**-oriented **Metroidvania RPG** (Hollow Knight–inspired feel; tight scope). All planning lives under **`docs/`**.

| | |
|---|---|
| **Vision & boundaries** | [docs/GAME_VISION_AND_SCOPE.md](docs/GAME_VISION_AND_SCOPE.md) |
| **Roadmap & exit criteria** | [docs/MILESTONES.md](docs/MILESTONES.md) |
| **LLM / Cursor prompts** | [docs/MASTER_PROMPTS.md](docs/MASTER_PROMPTS.md) |
| **Tech conventions** | [docs/TECH_STACK_AND_CONVENTIONS.md](docs/TECH_STACK_AND_CONVENTIONS.md) |
| **Agent briefing** | [AGENTS.md](AGENTS.md) |

Prototype code lives at the repo root: `platformer.c` (**Phase 1**: movement, tile collision, coyote/jump buffer, sprint, attack, dummy enemy, camera bounds, F1 hitboxes). Build needs a **raylib** development install (see `docs/TECH_STACK_AND_CONVENTIONS.md`).

**Art (PNG):** place files in **`assets/`** or next to `platformer.exe`. Load order is centralized in `src/assets_loader.c` (`load_first_*` lists). Summary:

| Role | Try first | Fallbacks |
|------|-----------|-----------|
| Hero | `assets/hero-idle-strip.png` or `hero-idle-strip.png` | `sprite-hero.png`, `medieval-rpg-main-character-d002.png`, `player_spritesheet.png` |
| Background | `sprite-background.png` | `background.png`, `sprite-castle.png` |
| Ground tile | `tile-stone-ground.png` | `tile-ground.png`, `tile.png` |
| Dummy enemy | `enemy-echo-basic.png` | `sprite-simple-enemy.png` |

Future art (not loaded in Phase 1 yet): `sprite-miniboss.png`, `sprite-final-boss.png`, `sprite-strong-enemy.png`, `sprite-gate.png`, `sprite-quest-map.png`, `sprite-quest-map-key.png`, `sprite-slash.png`.

Missing files use colored placeholder shapes. **Hero:** `64×32` = **one** full-frame wide pose (not two 32×32 cells in one row).
