# Milestones — execution order

Complete milestones **in order**. Each phase has **exit criteria**: do not skip to “more content” until the current exit is met.

---

## Phase 0 — Lock the box (scope contract)

**Goal:** decisions on paper so implementation does not thrash.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 0.1 | One-line pitch + player fantasy | Written in `GAME_VISION_AND_SCOPE.md` |
| 0.2 | Room count target (12–25) + rough graph sketch | Image or structured list in repo (`docs/world_graph.md` or `docs/art/`) |
| 0.3 | List of **2–3** abilities and what each **opens** | Table in vision doc |
| 0.4 | **Non-goals** list (already drafted; customize) | You agree nothing new enters without cutting something |
| 0.5 | Pillars unchanged (4 max) | Stated in vision doc |

**Exit:** you can explain the whole game in **under 2 minutes** without hand-waving systems.

**Status:** **Complete.** Locked details: `GAME_VISION_AND_SCOPE.md` (title *The Sunken Choir*, player fantasy, abilities/key, nail-upgrade spine, input target) and `world_graph.md` (18 rooms, graph, benches).

---

## Phase 1 — Character, combat skeleton, camera

**Goal:** fun in an empty test arena for **5+ minutes**.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 1.1 | Run, jump, fall control; consistent physics | No “sticky” or random launches |
| 1.2 | **Horizontal + vertical** collision resolution | Walls/ceilings/floors behave; no tunneling at 60 FPS |
| 1.3 | One **primary attack** (or short combo) + hit detection | Targets register hits reliably |
| 1.4 | **One defensive option**: **sprint** (Shift) with clarity; dodge/block may come later | Player trusts the button |
| 1.5 | Hit feedback: brief freeze, flash, or knockback on enemies (placeholder OK) | Combat reads clearly |
| 1.6 | Camera: follows player; room bounds or clamp; no motion sickness | Test on your worst screen |
| 1.7 | Coyote time / jump buffer (optional but recommended) | Values referenced in `docs/TECH_STACK_AND_CONVENTIONS.md` → `platformer.c` |

**Exit:** blindfolded teammate metaphor — a new player can **move and fight** without reading code comments.

**Implementation (code):** `platformer.c` targets 1.1–1.6 (coyote + buffer, hitstop on hit). **Playtest required** to confirm exit criteria.

**Manual test (5 minutes):** Build and run `platformer.exe`. Walk the arena; jump gaps; bump walls/ceiling; short-hop by releasing Space early; buffer jump before landing; Shift sprint; attack dummy (J) on mid platform until “Dummy cleared”; F1 toggles colliders; confirm camera stops at world edges.

---

## Phase 2 — World structure (Metroidvania spine)

**Goal:** greybox **start → mid → end** is playable.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 2.1 | Room loading / transitions (fade, slide, or seamless) | No crash when crossing edges |
| 2.2 | **Bench** (save/heal/respawn) + death loop | Player respawns with world rules consistent |
| 2.3 | **2–3 gates** tied to abilities or keys | Cannot sequence-break by accident (or if allowed, intentional) |
| 2.4 | **Map reveal** item or ability | Player can orient in 12–25 rooms |
| 2.5 | One **shortcut** that feels good to unlock | “I’m clever” moment |

**Exit:** friends play greybox and **finish** without dev teleport cheats.

---

## Phase 3 — Enemies and bosses

**Goal:** fair, learnable combat — quality over quantity.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 3.1 | Enemy archetype A (e.g. slow tank) | Telegraphed attacks |
| 3.2 | Enemy archetype B (e.g. fast) | Different rhythm from A |
| 3.3 | Enemy archetype C (e.g. ranged or area denial) | Forces new positioning |
| 3.4 | Mini-boss: simple pattern, readable phases | Beatable without grind |
| 3.5 | Final boss: **2 phases max** for v1 | Checkpoint or phase transition is fair |
| 3.6 | Encounter design pass: arenas, spawn fairness | No off-screen hits in main path |

**Exit:** deaths feel like **player mistakes**; no RNG-heavy losses on critical path.

---

## Phase 4 — Thin RPG layer

**Goal:** progression **changes how fights play**, not spreadsheets.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 4.1 | One system: upgrades **or** charms **or** skill nodes (choose one spine) | Documented caps |
| 4.2 | Currency or shards with **sinks** (bench upgrades, map, keys) | No infinite soft grind required |
| 4.3 | 3–5 NPCs / lore objects | Short text; supports pillar “atmosphere” |
| 4.4 | Optional branch reward feels **worth** backtracking | Playtester validates |

**Exit:** upgrading something makes you **want** to retry an old fight — optional.

---

## Phase 5 — Content, pacing, juice

**Goal:** shippable **experience**, not just mechanics.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 5.1 | First **20–30 minutes** at full polish bar | Juice, SFX, tuning |
| 5.2 | Full map pacing: signposting, difficulty curve | No dead confusion > 3 minutes |
| 5.3 | Audio pass: music layers or zones + combat stingers | No long silence in combat |
| 5.4 | Accessibility: remaps, volume, optional screenshake toggle | Checklist in tech doc |

**Exit:** cold player session: **fun** + **clear goal** in first 10 minutes.

---

## Phase 6 — Steam production

**Goal:** releasable Windows build and discoverability.

| ID | Deliverable | Done when |
|----|-------------|-----------|
| 6.1 | Title screen, pause, options, quit | |
| 6.2 | Save/load robustness (corruption handling minimum) | |
| 6.3 | Steam build pipeline (depot, branches) | Documented |
| 6.4 | Achievements (small set) | |
| 6.5 | Store page + capsule art + **trailer** | |
| 6.6 | External playtest → bug bash | No P0/P1 on critical path |

**Exit:** you would **pay $5–15** for this if you weren’t you (sanity check).

---

## Post-v1 (only after Phase 6 green)

- Boss rush / New Game+
- Extra biome
- Localization

---

## Dependency graph (short)

```
Phase 0 → Phase 1 → Phase 2 → Phase 3 → Phase 4 → Phase 5 → Phase 6
```

**Never** skip Phase 1 exit to “make more levels.” Bad feel kills the project early.
