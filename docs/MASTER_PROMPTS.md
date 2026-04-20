# Master prompts (Cursor / LLM)

**How to use:** Paste **Project context** once per new chat (or rely on `AGENTS.md` + rules). Then paste the **phase prompt** for the milestone you are in. Add files with `@` references as needed.

---

## 1. Project context (system-style — paste first in a fresh chat)

```
You are working on a Steam-bound SMALL Metroidvania RPG. Hollow Knight is a REFERENCE for mood, exploration, and combat feel — NOT a feature checklist. Scope is locked in docs/GAME_VISION_AND_SCOPE.md and docs/MILESTONES.md.

Rules:
- Do not expand scope (new biomes, big crafting, huge bestiary) unless the user explicitly overrides in this message.
- Prefer one complete vertical slice over many half systems.
- Match existing code style and stack; read docs/TECH_STACK_AND_CONVENTIONS.md before structural changes.
- Every gameplay change should state: which milestone (Phase 1–6) it serves and the exit criterion it advances.
- Minimize files touched; no drive-by refactors unrelated to the task.

Repository: read AGENTS.md, then the relevant milestone section in docs/MILESTONES.md.
```

---

## 2. Phase 0 — Lock the box

```
Milestone: Phase 0 (docs/MILESTONES.md).

Task: Fill open decisions in docs/GAME_VISION_AND_SCOPE.md: final title (or working title), player fantasy one-liner, exact 2–3 abilities and what each unlocks, room count in 12–25, primary RPG spine (one system). Add docs/world_graph.md with a bullet list of rooms as nodes and gates (no code).

Constraints: Do not write gameplay code. Output must be concise and paste-ready into the vision doc.
```

---

## 3. Phase 1 — Movement, combat skeleton, camera

```
Milestone: Phase 1 — exit: fun movement + combat in empty arena for 5+ minutes.

Task: [DESCRIBE: e.g. implement horizontal+vertical tile collision; add dodge with i-frames; fix camera bounds]

Read platformer.c (or current main module) and docs/TECH_STACK_AND_CONVENTIONS.md. Implement only what Phase 1 needs. After changes, list: (1) constants tuned (gravity, speeds), (2) known limitations, (3) how to manually test in 60 seconds.

Do not add map content, NPCs, or Steam integration.
```

### 3a. Combat feel tuning session

```
Milestone: Phase 1.

Task: Tune combat feel only: attack startup/active/recovery, hitstop duration, knockback, enemy hurt state. No new enemy types — use a static dummy hitbox.

Goal: hits feel “chunky” and readable at 60 FPS. Document final tunables at top of the combat module or in TECH doc.
```

### 3b. Physics debug

```
Milestone: Phase 1.

Task: Add a DEBUG toggle: draw player AABB, velocity vector, and collision normals. Fix any tunneling or corner snagging on the test map.

Remove or #ifdef debug draw for release builds per TECH doc.
```

---

## 4. Phase 2 — World structure (greybox Metroidvania)

```
Milestone: Phase 2 — greybox start→mid→end playable.

Task: [DESCRIBE: e.g. room data format; transitions; bench save/heal/respawn]

Follow docs/world_graph.md. Implement minimal greybox art (rectangles OK). Gates must match the 2–3 abilities from GAME_VISION_AND_SCOPE.md.

Deliver: how to traverse critical path; list of placeholders to replace later.
```

### 4a. Map reveal item

```
Milestone: Phase 2.

Task: Implement map reveal: fog of war or unexplored rooms until player acquires ITEM_NAME. Persist in save data structure (even if save is file stub).

Keep UI minimal: pause map or corner minimap — pick one for v1 and justify in 2 sentences.
```

---

## 5. Phase 3 — Enemies and bosses

```
Milestone: Phase 3 — three archetypes + mini-boss + final boss.

Task: Implement enemy archetype [A/B/C]: state machine (idle, windup, attack, recover), telegraph, damage to player, damage from player.

Constraints: Reuse one base struct if possible; no new progression systems. Boss: max 2 phases for final; mini-boss simpler pattern.

Include spawn placement notes for encounter designer (which room IDs).
```

### 5a. Boss design pass

```
Milestone: Phase 3.

Task: Boss [name]: document pattern list (ordered), safe windows, and how each phase unlocks. Implement with placeholder sprites. Playtest checklist: no off-screen damage; every lethal attack has telegraph ≥ X frames (state your X).

Adjust only boss-related code and shared combat hooks.
```

---

## 6. Phase 4 — Thin RPG layer

```
Milestone: Phase 4 — one progression spine only.

Task: [upgrades OR charms OR skill tree — pick what vision doc locked]

Constraints: cap power; every purchase must have a sink or opportunity cost. Add 3–5 lore interactables with short copy (I'll paste strings or you use TBD placeholders).

No second economy unless replacing the first.
```

---

## 7. Phase 5 — Polish and pacing

```
Milestone: Phase 5 — first 20–30 minutes shippable.

Task: Juice pass on [area]: screenshake rules, hit particles, land dust, audio hooks. Update `docs/sfx_backlog.md` with new rows or status changes.

Do not add new mechanics. Fix pacing bugs: signposting, difficulty spikes, unclear objectives — list issues fixed.
```

### 7a. Narrative / environmental pass

```
Milestone: Phase 5.

Task: Rewrite NPC barks and sign text to ≤ 2 lines each. Align tone with player fantasy in GAME_VISION_AND_SCOPE.md. No lore dumps in dialogue boxes — put detail in environment inspectables.

Output: table OLD → NEW for each string changed.
```

---

## 8. Phase 6 — Steam production

```
Milestone: Phase 6.

Task: [DESCRIBE: e.g. pause menu, options, Steam init stub, achievement hooks]

Constraints: fail gracefully if Steam API not present in dev. Document build steps for Windows release in TECH_STACK_AND_CONVENTIONS.md.

No gameplay feature creep.
```

### 8a. Save file safety

```
Milestone: Phase 6.

Task: Versioned save format; migration from v0 to v1; corruption detection — on failure, start new game with clear message. Minimal implementation.

Test cases: list manual steps.
```

---

## 9. Code review prompts (any phase)

### 9a. Scope guard

```
Review the diff against docs/GAME_VISION_AND_SCOPE.md and docs/MILESTONES.md. List any scope creep, unnecessary abstractions, or files that should not have changed. Suggest the smallest rollback or split to stay on milestone.
```

### 9b. Performance pass

```
Profile mindset only: identify per-frame allocations, O(n²) collision, texture binding churn. Propose fixes ordered by impact. Do not rewrite engine — incremental fixes only.
```

---

## 10. Marketing / store (human + LLM)

### 10a. Steam page draft

```
Using GAME_VISION_AND_SCOPE.md only: write Steam short description (≤ 300 chars), long description with bullet features (no false claims), and 5 tag suggestions. Tone: [mysterious / bleak / hopeful]. No comparisons to other games by name.
```

### 10b. Trailer beat sheet

```
Create a 60-second trailer beat sheet: seconds 0–10 hook, 10–40 gameplay pillars, 40–55 boss tease, 55–60 title. List only features that exist or will exist in v1 per MILESTONES.md.
```

---

## 11. Weekly planning (short)

```
Given docs/MILESTONES.md, list what we should finish this week to advance [Phase X]. Break into 3–5 tasks with hour estimates. Flag risks (dependencies, unknown engine issues). Do not add new features.
```

---

## Prompt index

| Use when | Section |
|----------|---------|
| New chat | §1 Project context |
| Decisions only | §2 Phase 0 |
| Movement / combat / camera | §3 |
| Rooms / bench / map | §4 |
| Enemies / bosses | §5 |
| Currency / upgrades | §6 |
| Juice / story polish | §7 |
| Steam / saves / menus | §8 |
| Review | §9 |
| Store / trailer | §10 |
