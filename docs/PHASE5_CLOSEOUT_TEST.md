# Phase 5 closeout test (manual, pass/fail)

Use this checklist to close the remaining Phase 5 items:
- cold-player clarity in first 10 minutes
- pacing across first 20-30 minutes
- accessibility + persistence checks

## Test setup

- Build latest executable from repo root.
- Delete `sunken_choir_settings.cfg` before the first run to confirm default behavior.
- Run with keyboard only (no debug teleports).

## 1) Cold-player first 10 minutes (clarity/fun)

- [ ] Start a fresh run and follow on-screen objective text only.
- [ ] Reach East arena route without external help.
- [ ] Confirm no confusion dead-end lasts longer than 3 minutes.
- [ ] Confirm at least one clear "next step" objective is always visible.

Pass if all checks are true.

## 2) 20-30 minute pacing pass

- [ ] Play Entrance -> Hub -> East -> Hub/Shaft flow with no resets.
- [ ] Combat pressure ramps without sudden impossible spike.
- [ ] Resource flow feels fair (shards + bench sinks remain understandable).
- [ ] Return paths/shortcuts are readable once unlocked.
- [ ] Session remains stable (no crash/soft-lock, transitions still smooth).

Pass if all checks are true.

## 3) Audio pass validation

- [ ] Hear jump, footstep, land, attack whiff/hit, pickup, and UI cues.
- [ ] Confirm combat never goes into long silent windows.
- [ ] Confirm zone/combat music behavior still updates while moving rooms.

Pass if all checks are true.

## 4) Accessibility + persistence (requested in Phase 5)

- [ ] Open options and remap Jump + Attack + Map.
- [ ] Change Master/Music/SFX volumes and toggle screenshake.
- [ ] Quit game fully and relaunch.
- [ ] Confirm remaps persist.
- [ ] Confirm volume values persist.
- [ ] Confirm screenshake toggle persists.

Pass if all checks are true.

## Exit rule

Phase 5 closeout is complete when sections 1-4 all pass in one build candidate.
