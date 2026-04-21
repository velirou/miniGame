# World graph — The Sunken Choir (current implementation)

## Summary

This file now documents what is currently playable on `main` (desktop build), not the full 18-room end target.

| Field | Value |
|-------|--------|
| **Working title** | The Sunken Choir |
| **Implemented room count** | **7** |
| **Critical path** | Entrance -> Hub -> East -> Hub -> Shaft -> Antechamber -> Bellcrown -> Stillwater |
| **Current bosses** | Cantor (mini), High Cantor (final) |
| **Benches** | Hub, Antechamber |
| **Map reveal** | Pickup in Shaft (opens minimap toggle) |

---

## Gates and progression (implemented)

| Gate | Requirement | In-game result |
|------|-------------|----------------|
| Hub -> East | Cathedral Key (`GATE_EAST_ARENA`) | Access to Cantor + Chord Cling pickup room |
| Hub -> Shaft | Chord Cling (`GATE_CLING`) | Unlocks vertical route south |
| Shaft -> Antechamber (right-bottom exit) | Veil Drift (`GATE_DRIFT`) | Opens late-game branch |
| Antechamber -> Bellcrown | Cathedral Key (`GATE_KEY`) | Final boss approach |
| Antechamber (high return-lift) -> Entrance | None (`GATE_NONE`) | Late-game shortcut back to start |
| Bellcrown (high-left lift) -> Entrance | High Cantor defeated (`GATE_FINAL_CLEAR`) | Post-boss shortcut back to start |
| Bellcrown -> Stillwater | High Cantor defeated (`GATE_FINAL_CLEAR`) | Ending walkout room |
| Stillwater -> Entrance | None (`GATE_NONE`) | Wraps map back to start |

---

## Room list (implemented)

| ID | Runtime enum | Name | Purpose |
|----|--------------|------|---------|
| R01 | `ROOM_ENTRANCE` | Entrance | Start room, Cathedral Key pickup, early enemy |
| R02 | `ROOM_HUB` | Hub | Main connector, first bench, links to Entrance/East/Shaft |
| R03 | `ROOM_EAST` | East arena | Cantor mini-boss + Chord Cling pickup |
| R04 | `ROOM_SHAFT` | Shaft | Vertical room, map pickup, drift-gated right exit |
| R05 | `ROOM_ANTECHAMBER` | Antechamber | Late-game lobby, second bench, gate to Bellcrown |
| R06 | `ROOM_BELLCROWN` | Bellcrown | Final boss room (High Cantor) |
| R07 | `ROOM_STILLWATER` | Stillwater | Post-boss ending/walkout space |

---

## Graph (implemented)

Legend: `↔` both ways, `[gate]` requirement.

```
[Entrance] ↔ [Hub] ↔ [East arena]
                |        ^ (Cantor mini-boss)
                |        |
                └─[Chord Cling required]─┘

[Hub] ↔ [Shaft]
          |  \
          |   └─[Veil Drift required]→ [Antechamber] ↔ [Bellcrown] → [Stillwater]
          |                                |  \          |
          |                                |   └────→ [Entrance]
          |                                |      (post-boss lift from Bellcrown)
          |                                |             └─[High Cantor defeated]
          |                                └─[Cathedral Key required]
          |                                └─(high return-lift) → [Entrance]
          |
          └────────────────────────────────────────────────────────→ [Entrance]
                             (Stillwater wraparound door)
          └─(map pickup in Shaft)
```

---

## Enemy placement (implemented)

| Room | Enemy focus |
|------|-------------|
| Entrance | Dasher |
| Hub | Tank |
| East arena | Cantor (mini-boss) |
| Shaft | Caster |
| Antechamber | Caster |
| Bellcrown | High Cantor (final boss) |

---

## Notes

- The long-term vision remains an 18-room world (`docs/GAME_VISION_AND_SCOPE.md`), but this graph tracks the current playable slice.
- As rooms are added, keep enum names and graph IDs aligned with `platformer.c`.

---

## Revision history

| Date | Change |
|------|--------|
| Phase 0 | Initial 18-room concept graph locked |
| Phase 2+3 slice | Replaced doc with current 7-room implemented graph on `main` |
