# World graph — The Sunken Choir (v1)

## Summary

| Field | Value |
|-------|--------|
| **Working title** | The Sunken Choir |
| **Room count** | **18** (R01–R18) |
| **Critical path (unique rooms visited)** | ~14–16 depending on routing |
| **Optional branch** | Yes — **hidden wing inside R14**; reward: **health shard** (one) + shortcut to R11 |
| **Benches** | R04, R11, R17 antechamber (see notes) |

---

## Abilities and gates (cross-reference)

| Gate | Unlocks |
|------|---------|
| **Veil Drift** | R07 spike run; R10→R12 narrow; shortcut R08→R05 (one-way) |
| **Chord Cling** | R05 vertical; R12→R13 drop climb back; reach R15 |
| **Cathedral Key** | R16→R17 gate |
| **Cantor defeated** | Awards **Veil Drift** (required to proceed along main line) |
| **High Cantor defeated** | Ending / credits |

---

## Room list (purpose of each room)

| ID | Name | Role |
|----|------|------|
| R01 | Floodstairs | Start; tone-setting; basic movement |
| R02 | Reliquary Gap | First hazard read (no damage floor / water) |
| R03 | Drowned Pew | Combat teach — archetype A |
| R04 | Mercy Bench | **Bench 1**; hub branch |
| R05 | Canticle Shaft | **Chord Cling** gate (vertical); shortcut down after cling |
| R06 | Broken Rose Window | Routing; archetype B intro |
| R07 | Gargoyle Walk | **Veil Drift** gate (spikes) — backtrack after R09 |
| R08 | Antechapel | Connects to mini-boss; environmental story |
| R09 | Cantor’s Loft | **Mini-boss Cantor** → **Veil Drift** |
| R10 | Echo Nave | Arena spacing; archetype C |
| R11 | Crossing | **Bench 2**; map-feel hub |
| R12 | Organ Pit | Timing challenge pre-cling; post-drift revisit |
| R13 | Bellows Crypt | **Chord Cling** pickup |
| R14 | Sunken Sacristy | Lore; **optional secret wing** (shard + shortcut to R11) inside same screen |
| R15 | Confessional Maze | **Cathedral Key** (requires Chord Cling to reach) |
| R16 | High Altar Gate | Key check → boss approach |
| R17 | Bellcrown | **Final boss High Cantor** (2 phases max) |
| R18 | Stillwater | Ending space — short walkout / stinger |

*Note:* Optional content lives in a **secret sub-area** of R14 (same map node for the 18-room count).

---

## Graph (connections)

Legend: `↔` both ways, `→` one-way, `[gate]` requirement.

```
                    [R18 Stillwater]
                           ↑
                    [R17 Bellcrown]  (final boss)
                           ↑
              [gate: Cathedral Key]
                           ↑
                    [R16 High Altar Gate]
                           ↑
                    [R15 Confessional Maze]  [needs Chord Cling to enter from R14]
                           ↑
         ┌─────────────────┴─────────────────┐
         ↑                                   ↑
   [R14 Sunken Sacristy]              (optional secret in R14 → shard + shortcut → R11)
         ↑
   [R13 Bellows Crypt]  ← Chord Cling pickup
         ↑
   [R12 Organ Pit]  [partial gate: Veil Drift for clean traversal]
         ↑
   [R11 Crossing]  **Bench 2**
         ↑
   [R10 Echo Nave]
         ↑
   [R07 Gargoyle Walk]  [gate: Veil Drift]
         ↑
   [R06 Broken Rose Window]
         ↑
   ┌────┴────┐
   ↑         ↑
[R08]     [R05 Canticle Shaft]  [gate: Chord Cling for full vertical]
   ↑         ↑ (shortcut down after cling)
[R09]       │
Cantor      │
(mini)      │
   ↑         │
   └────┬────┘
        ↑
   [R04 Mercy Bench]  **Bench 1**
        ↑
   [R03 Drowned Pew]
        ↑
   [R02 Reliquary Gap]
        ↑
   [R01 Floodstairs]  START
```

**Early critical path (first bench to mini-boss):** R01→R02→R03→R04→R06→R08→R09 (get Veil Drift).

**Then:** R07 open → R10→R11→R12→R13 (Chord Cling) → R14→R15 (key) → R16→R17→R18.

---

## Bench placements

| Bench ID | Room | Notes |
|----------|------|--------|
| B1 | R04 Mercy Bench | After first combat set piece |
| B2 | R11 Crossing | Mid-map; optional branch loops back here |
| B3 | R16 (antechamber) *or* small room R16a | If scope tight: heal statue before R17 only — prefer **real bench** in R16 lobby |

**Decision:** Use **two full benches + bench before boss** — either R16 lobby as B3 or merge B3 into R11 pacing (harder). **Locked for v1:** **three** bench sites — R04, R11, **R16 antechamber** (treat as part of R16 graph node).

---

## Secret / optional

| Location | Secret | Reward |
|----------|--------|--------|
| R14 (secret wing) | Break wall or underwater passage | **Health shard** + shortcut to R11 |
| R08 | Inspectable lore | Flavor only |

---

## Enemy placement (design intent only)

| Archetype | Example rooms |
|-----------|-----------------|
| A — slow / heavy | R03, R06 |
| B — fast / flanking | R06, R08, R10 |
| C — ranged / area denial | R10, R12, R15 |

Bosses: R09 Cantor (mini), R17 High Cantor (final).

---

## Revision history

| Date | Change |
|------|--------|
| Phase 0 | Initial 18-room graph, abilities, keys, benches |
