# Mobile Playability Checklist (Pre-Phase 3)

Goal: make the current Phase 2 vertical slice playable on phones without changing world scope.

## Branch and safety

- [x] Work on dedicated branch: `feature/mobile-foundation`
- [x] Keep desktop keyboard controls intact while adding touch support
- [ ] Re-test critical path after each mobile tweak

## Input and controls

- [x] Centralize player input into one input state (keyboard + touch)
- [x] Add on-screen touch controls for left/right movement
- [x] Add on-screen touch controls for run, jump, attack, bench/use, map toggle
- [x] Preserve jump press/release behavior (jump buffer + jump cut) for touch
- [x] Keep reset/debug/map keys for desktop testing

## UI and screen behavior

- [x] Use live screen size (`GetScreenWidth/GetScreenHeight`) each frame
- [x] Keep camera centered correctly on different aspect ratios
- [x] Draw touch controls only on mobile builds
- [x] Add map toggle state (M key / touch map button)

## Playtest passes (manual)

- [ ] 10-minute phone playtest: movement, combat, bench interactions
- [ ] Verify no control overlap blocks key gameplay moments
- [ ] Verify room transitions and fades still work on touch
- [ ] Verify minimap toggle works after map pickup
- [ ] Verify performance holds near 60 FPS in combat room

## Out of scope for this pass

- Full remapping UI
- Device-specific safe-area/notch handling
- Console-style options menu for touch layout editing
