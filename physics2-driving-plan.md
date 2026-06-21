# physics2 driving game — a live, end-to-end physics integration test (plan)

A third-person driving demo: you see the car from behind, a chase camera follows
it, and you drive around an **open playground** poking at one of every physics
subsystem — rigid stacks, swinging gates and wrecking balls (constraints), spring
bollards, and a cloth banner. The point is **validation under live interaction**:
the car *drives into* each subsystem instead of each being checked in an isolated
unit test. It becomes the project's flagship physics2 demo.

Decisions baked in (user, Session 77):
- **Open playground + stations** (not a circuit) — most flexible, grows
  station-by-station, ideal for an integration test. A circuit can come later.
- **Add a reusable `KeyboardHandler` to ICLQt** (not a demo-local Qt hack) —
  ICL has no app-level keyboard input today; a framework facility benefits every
  future app and fits ICL's self-containment.
- **Full arc** — M1–M5; rigid core first, then soft-body stations and polish, all
  in one world (multi-world dropped — see §1). Rigid-body focus first (user,
  Session 77): land the vehicle + course + rigid stations before wiring soft bodies.

This supersedes the old one-line "Phase 4b raycast vehicle" item: the
`VehicleDriver` IS Phase 4b, and the driving game is the showcase built on it.

---

## 1. World architecture: ONE unified world (the call, Session 77)

The Phase 4c claim that "constraints need a `SoftRigid` world" was a
**misdiagnosis** — that failing run was confounded by an anchor-overlap bug *and*
the Y-axis gimbal limit. Re-tested with correct geometry, **all five joint types
pass in the default `Deformable` world**, which is a `btDeformableMultiBodyDynamics
World` — a discrete world at heart that hosts **rigid bodies + 6DOF joints +
stable deformable cloth in one solver**. It already *is* the "DeformableRigid"
everything-world.

**Decision: build the whole driving game on the single default `Deformable`
world.** No `SoftRigid`, no multi-world split. This drops the entire "list of
worlds" machinery the earlier plan (and `physics-geom2-redesign-plan.md` §6)
called for — keep multi-world only as a far-future option if some scene ever needs
paper's *cluster* self-collision (SoftRigid-only) alongside deformable cloth, which
the driving game does not.

`SoftRigid` survives solely for the fold-aware paper (`PaperDriver`), which needs
cluster self-collision. Everything else — including this game — uses `Deformable`.

**One open risk** (the only thing that could reopen the question): the **raycast
vehicle** (`btActionInterface`) in the `Deformable` world is untested. Rigid
dynamics there is proven (`box_falls` runs in it), but the vehicle is the demo's
core, so **M1 must confirm the vehicle drives in the `Deformable` world.** If it
does, the unified-world call is locked; only if the vehicle genuinely misbehaves
there do we localize it to a rigid world and revisit.

---

## 2. New reusable building blocks (framework, not demo-local)

| Piece | Home | Role |
|---|---|---|
| **`VehicleDriver`** | `icl/physics2/` | `btRaycastVehicle` wrapper (= deferred Phase 4b) |
| **`KeyboardHandler`** | `icl/qt/` | app-level key input, mirrors `MouseHandler` |
| **`PhysicsWorld::addAction/removeAction`** | `icl/physics2/` | tick a `btActionInterface` (the vehicle) each step |
| **`ChaseCamera`** | demo → `icl/geom2/` | third-person follow camera (promote if reused) |

### 2a. `VehicleDriver` (`btRaycastVehicle`)

A `geom2::Driver` on the chassis node. Owns:
- the chassis `btRigidBody` (built like `RigidBodyDriver` — shape from the node),
- a `btDefaultVehicleRaycaster` bound to the world's `btDynamicsWorld`,
- a `btRaycastVehicle` (added to the world as a `btActionInterface`),
- 4 wheel `MeshNode`s as children of the chassis node.

API (ICL units / radians; no Bullet in the surface):
```cpp
void setEngineForce(float f);     // drive (rear or all wheels)
void setBrake(float b);
void setSteering(float radians);  // front wheels
float getSpeedKmh() const;        // for the HUD + chase-cam feel
```
**Thread split / sync (reuses the membrane pattern):** the vehicle's
`updateAction` runs on the sim thread inside `step()`. A **post-step capture hook**
(like soft bodies) snapshots the chassis transform + the 4
`getWheelTransformWS(i)` into a buffer; `sync()` on the UI thread writes the
chassis node + the 4 wheel child nodes, interpolated by `alpha`.

Construction via the world factory: `world.addVehicle(chassisNode, tuning)` /
`PhysicsScene::addVehicle(...)`, returning `VehicleDriver*`. Tuning struct
(suspension stiffness/damping/travel, wheel radius, friction, connection points)
with sensible defaults.

Gotchas to honor: fast wheels want **CCD** on the chassis; tune suspension to the
ICL→Bullet unit scale (mm); set `setCoordinateSystem` to match Z-up.

### 2b. `KeyboardHandler` (ICLQt)

`ICLWidget::keyPressEvent` currently only does F11; there's no install hook. Add,
mirroring `MouseHandler`:
- a `qt::KeyboardHandler` base (`virtual void process(const KeyEvent&)`),
- `ICLWidget::install(KeyboardHandler*)` + dispatch from `keyPressEvent` /
  `keyReleaseEvent` (set `setFocusPolicy(Qt::StrongFocus)` so the widget gets keys),
- a `KeyEvent` carrying the Qt key code + press/release + a convenience
  `held(int key)` state set (handle auto-repeat: ignore `event->isAutoRepeat()`).

The demo keeps the handler and reads held state each frame:
`if (keys.held(Qt::Key_W)) v->setEngineForce(+F);`

### 2c. `ChaseCamera`

Per-frame: target = chassis pose; desired camera pos = chassis +
`R_chassis * (0, -behind, +up)`; smooth with exponential lerp
(`pos += (desired-pos) * (1-exp(-k*dt))`), slerp the look direction toward the
car (slightly ahead). Updates `scene.getCamera(0)` in `run()`. Demo-local first;
promote to `geom2` if a second consumer appears. (Cameras aren't scene-graph nodes,
so it's a helper over `geom::Camera`, not a `Driver`.)

---

## 3. Milestones (multi-session)

### M1 — `VehicleDriver` + it drives. ✅ LANDED (Session 77).
- `PhysicsWorld::addAction/removeAction`; `VehicleDriver` (chassis + raycaster +
  4 raycast wheels + capture-hook sync of chassis + wheel transforms); `Config`
  (`VehicleConfig`); `PhysicsScene::addVehicle` (adds chassis + wheel nodes).
- **4 headless `stepOnce` tests** (all in the default Deformable world — settles
  the open risk): `vehicle_rests_on_wheels`, `vehicle_drives_forward` (+Y),
  `vehicle_steers`, `vehicle_brakes`. Full suite **934/934**. Minimal demo
  `physics2-driving` (sliders, fixed cam) builds + inits.
- **Two non-obvious findings (now fixed + documented):**
  1. **The soft/deformable worlds skip Bullet's `updateActions()`** (they override
     the step internals), so `world->addAction` never ticks the vehicle. Fix:
     `PhysicsWorld` ticks registered actions *manually* in `stepOnce` (after
     draining controls, before integration). So the vehicle works in ANY world —
     **the open risk is settled: it drives in the unified Deformable world.**
  2. **Suspension must be scale-aware.** physics2 scales lengths by `iclToBullet`
     but not time/mass, so Bullet-side gravity is ~10× SI; the standard stiffness
     20 / maxForce 6000 are ~10× too soft and the car bottoms out on its chassis
     box. `VehicleDriver` scales stiffness/damping by the gravity ratio and sets
     `maxSuspensionForce` to the chassis weight. (The plan's "vehicle unit scale"
     risk, realized + handled.)
- Open polish (M2): wheel-mesh orientation (`wheelAlign` is identity — visual only,
  needs a real display); CCD on the chassis for fast driving / jumps (M3).

### M2 — controls + chase camera. ✅ LANDED (Session 77).
- **`qt::KeyboardHandler`** — the app-level keyboard input ICL lacked (mirrors
  `MouseHandler`): `KeyEvent` + `KeyResult`, base tracks a held-key set, install
  via `widget->install(&handler)` / `gui["draw"].install(&handler)`. Wired into
  `ICLWidget` (`keyPressEvent`/`keyReleaseEvent`, auto-repeat filtered,
  `setFocusPolicy(StrongFocus)`) and the `DataStore::Slot` handle-dispatch
  (`HasInstallKeyboard` concept). Headless test `qt.keyboard.held_set` (935/935).
- Demo `physics2-driving` now drives with **W/S/A/D + Space** (held-key polling)
  and a **`ChaseCamera`** (demo-local: smoothed follow behind+above, Z-up, looks
  slightly ahead). Inits clean headless.
- **Deferred to a real display (sandbox has no GL):** feel tuning — accel curve,
  steer rate/return, camera distance/height/smoothing, wheel-mesh orientation
  (`wheelAlign`). The mechanics are pinned by M1's `stepOnce` tests; the *feel*
  needs eyes on it.

### M3 — the playground course (static)
- Ground arena + boundary, a few **ramps**, a **jump**, a **banked turn**, scattered
  static blocks. Verify suspension + CCD on jumps (no tunneling), recovery from
  landings.

### M4 — interactive stations (the integration test)
- **Rigid:** box pyramids to smash, barrels (cylinders) to scatter.
- **Constraints:** a **swing gate** (hinge), a **see-saw/teeter** (hinge + angular
  limits), a **wrecking ball** (ballsocket pendulum), **spring bollards** (spring
  constraint — bounce back when nudged).
- **Soft:** a **cloth banner/curtain** you burst through (SoftRigid, band-aided).
- Each station drive-tested; this is where the whole stack gets exercised at once.

### M5 — polish
- **HUD** (speedometer via `getSpeedKmh`), **reset** key (respawn the car), look
  polish (materials, maybe skid marks if cheap).
- (Multi-world dropped — the single `Deformable` world already carries rigid +
  joints + cloth. See §1.)

---

## 4. What this validates

Rigid dynamics, kinematic + collision response, **all joint types** (hinge /
slider / ballsocket / 6DOF / spring), soft bodies, CCD, contact events, and unified
picking — **under live interaction** rather than isolated assertions. The headless
`stepOnce` tests pin the vehicle mechanics deterministically; the playground is the
human-in-the-loop validation the unit tests can't give.

## 5. Open questions / risks (resolve as we go)

- **Keyboard focus:** the GL widget must grab focus for key events
  (`setFocusPolicy`); confirm it doesn't fight the GUI. (M2 spike.)
- **Vehicle unit scale:** `btRaycastVehicle` suspension constants are tuned for
  metres — must scale through `Units` (the mm→Bullet factor) or the car will be
  undriveable. (M1.)
- **Z-up:** `btRaycastVehicle` defaults to Y-up; set the coordinate system /
  up-axis to match physics2's Z-up. (M1.)
- **GUI verification limit:** the sandbox has no GL context, so M2+ feel-tuning
  needs a real display; mechanics stay headless-verifiable via `stepOnce`.

Branch `further-restructuring-and-cleanup` (or a dedicated branch if it runs long).
