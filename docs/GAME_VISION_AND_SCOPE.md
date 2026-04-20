# Game vision and scope (v1)

## Working title

**The Sunken Choir** (working title — change before Steam page if needed)

## Elevator pitch

**A compact 2D exploration action-RPG:** one ruined region beneath a drowned cathedral, tight melee combat, ability-gated paths, and a clear story beat — playable in **one sitting or two**, built for Steam as a **small, complete** game, not a forever project.

**Reference (tone and design, not feature parity):** *Hollow Knight* — mood, exploration rhythm, dodge-and-strike combat, map surprises, bench/save fantasy.

---

## One-line pitch (store-ready draft)

*Descend through a flooded cathedral to silence a curse that still sings in the deep — a short Metroidvania about blade, drift, and the last hymn.*

---

## Format targets

| Dimension | v1 target | Notes |
|-----------|-----------|--------|
| Session length | **2–4 hours** first playthrough | Optional completionism +30–60 min |
| World size | **18 rooms** (see `docs/world_graph.md`) | One critical path + **one** optional branch |
| Abilities / key items | **2 abilities + 1 key** | See table below |
| Enemy archetypes | **3** distinct families | Re-skins OK inside a family |
| Bosses | **1 mini-boss + 1 final boss** | Final: max 2 phases |
| NPCs with dialogue | **3–5** | Short lines; story also in environment |
| RPG systems | **One spine: nail upgrades** | 3 tiers at benches; no parallel charm meta in v1 |

---

## Design pillars (locked — 4 max)

1. **Exploration** — map is readable; backtracking opens shortcuts; secrets feel earned.
2. **Combat feel** — responsive movement; fair attacks; dodge or block that players trust.
3. **Progression clarity** — player always knows *roughly* what blocks progress (ability, key, boss).
4. **Atmosphere** — audio + lighting + color support one cohesive mood.

Anything that does not serve these four is **out of scope for v1** unless it replaces something weaker.

---

## Hollow Knight “reference strip” (what we actually ship)

**Ship in spirit (adapted to small scope):**

- Single continuous map with **locks** and **keys** (abilities count as keys).
- **Bench-like** checkpoint: heal, respawn on death, world state updates.
- **Melee-forward** combat with a small moveset that stays readable.
- **Map reveal** (item or system) so exploration feels rewarding.
- **Environmental storytelling** over long cutscenes.

**Do not ship in v1 (explicit non-goals):**

- Multi-biome world at Team Cherry scale.
- Large charm inventory meta-game, fragile builds, or deep crafting.
- Speedrun tech as a requirement for main path.
- Procedural map.
- Full quest log with ten parallel threads.

**Project-specific non-goals (v1):**

- Second playable character or NG+ content.
- More than **three nail upgrade tiers** or a second currency sink beyond shards + map.
- Fishing, gardening, or other life-sim loops.
- Voice-acted dialogue or long cinematic sequences.
- **Scope rule:** a new feature only ships if something else is cut or deferred to post-v1; default answer is **no**.

---

## Player fantasy (locked)

**You are a pilgrim-knight** who **descends to end a hymn that should have stopped** **in the sunken cathedral called the Choir Below.**

Keep this sentence stable; it steers art, UI, and writing.

---

## Abilities and gates (locked)

Two **abilities** + one **key item**. Details and room IDs live in `docs/world_graph.md`.

| Unlock | Type | What it opens |
|--------|------|----------------|
| **Veil Drift** | Ability (short invulnerable dash) | Spike gauntlets, narrow timing gaps, some shortcuts (e.g. approach to mid-game) |
| **Chord Cling** | Ability (wall slide + wall jump) | Vertical shafts, organ-loft routes, access to key and optional branch |
| **Cathedral Key** | Key item | Sealed gate before the final boss arena |

**Acquisition order (intended critical path):** Veil Drift from the mini-boss → Chord Cling from exploration → Cathedral Key from a gated side path → final boss.

---

## Primary progression spine (locked)

- **Nail upgrades only:** three tiers purchased at benches with a single currency (**choir shards** — name TBD in strings).
- Optional **health shard** (one) on the optional branch — does not add a second “system,” just a pickup.

---

## Input target (locked)

- **Development:** controller-first for feel and camera tests.
- **Ship:** **keyboard + controller** with remappable bindings (Steam expectation).

---

## World graph (paper-first)

Authoritative room list and connections: **`docs/world_graph.md`**.

Rules:

1. **Nodes** = rooms, **edges** = connections (include one-way and shortcuts).
2. **Gates** marked: Veil Drift, Chord Cling, Cathedral Key, boss defeats.
3. **Benches** control difficulty spacing — three benches for v1 (see graph doc).

**Rule:** no room is “filler” — each teaches something (enemy, hazard, secret, or shortcut).

---

## Success definition (v1 “done”)

- Steam build: **Windows** at minimum; controls remappable; stable saves.
- **No soft-locks**; critical path verified by playtesters who did not write the game.
- **First 20 minutes** polished (juice, difficulty, clarity) — this is your store demo / refund window.
- Store page + **one** trailer that shows real gameplay.

---

## Phase 0 checklist (complete)

- [x] Working title
- [x] Player fantasy sentence
- [x] Ability list (2) + key (1) and what each unlocks
- [x] Room count **18** + rough graph (`docs/world_graph.md`)
- [x] Primary progression: **nail upgrades (3 tiers)**
- [x] Input: **controller-first dev, ship both**

---

## Relationship to current codebase

The repository may start as a **Raylib + C** prototype. Larger scope may justify **moving to a higher-level engine or language** later — that is a **milestone decision**, not a daily whim. Document engine changes in `TECH_STACK_AND_CONVENTIONS.md`.

---

## The two-minute explanation (Phase 0 exit)

You play a pilgrim-knight exploring **the Choir Below**, a flooded cathedral ruin. You fight with a **nail** upgraded at **benches** using one currency. **Veil Drift** (from a mini-boss) lets you pass spikes and tight timings; **Chord Cling** (found deeper) opens vertical paths and the **Cathedral Key**; the key opens the path to a **final boss** that ends the curse’s song. **Eighteen rooms**, one optional branch with a reward, **three benches**, **three enemy families**, no extra biomes or crafting — mood and exploration like *Hollow Knight*, scope like a focused indie episode.
