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
- **Full arc through multi-world** — M1–M5; the multi-world upgrade for stable
  cloth lands the deferred multi-world experiment as a bonus.

This supersedes the old one-line "Phase 4b raycast vehicle" item: the
`VehicleDriver` IS Phase 4b, and the driving game is the showcase built on it.

---

## 1. The crux: world architecture (single-world now → multi-world later)

We learned in Phase 4c that **constraints (and the raycast vehicle) need a
`SoftRigid` (discrete) world** — the `Deformable` multibody solver injects energy
into 6DOF joints. But **stable cloth wants the `Deformable` world** (the SoftRigid
cloth path is band-aided and can explode at rest). A course with both swinging
gates *and* a cloth banner can't have both ideal in one world.

Resolution — phase it:
- **M1–M4: single `SoftRigid` world.** Vehicle + rigid + constraints + springs all
  correct; cloth runs band-aided (works, occasionally janky). Gets the whole game
  playable.
- **M5: multi-world.** Vehicle/rigid/constraints stay on a `SoftRigid` world; the
  cloth banner moves to a `Deformable` world; both publish into one `Scene2`. This
  is exactly the deferred "paper SoftRigid + cloth Deformable in one scene"
  experiment from `physics-geom2-redesign-plan.md` §6 — now with a concrete
  consumer driving it. Needs `PhysicsScene` to own a *list* of worlds (the one
  missing piece called out there).

So the driving game is the forcing function that finally motivates multi-world.

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

### M1 — `VehicleDriver` + it drives ✅ unit-testable headless
- `PhysicsWorld::addAction/removeAction`; `VehicleDriver` (chassis + raycaster +
  4 wheels + capture-hook sync); `addVehicle` factory + `PhysicsScene::addVehicle`.
- **Headless `stepOnce` tests:** engine force → chassis moves +forward (local);
  steering → heading changes (yaw); brake → decelerates to rest; drives up a static
  ramp without flipping; wheels stay grounded on flat (suspension rest length).
- Minimal demo (temporary slider control, fixed camera, flat ground) to confirm it
  rolls. Build + suite green.

### M2 — controls + chase camera
- `KeyboardHandler` in ICLQt (+ a tiny headless-ish test of the held-set logic).
- Wire WASD/arrows → engine/brake/steer; `ChaseCamera` follow.
- **Feel tuning** (real display): accel curve, max steer + steer-return, suspension,
  camera distance/height/smoothing. Drive freely on flat ground.

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

### M5 — multi-world + polish
- `PhysicsScene` owns a *list* of worlds; cloth banner → a `Deformable` world while
  vehicle/rigid/constraints stay `SoftRigid`; both sync into the one `Scene2`.
  (Lands the deferred multi-world experiment.) Picking resolves a hit node → its
  owning driver regardless of world.
- **HUD** (speedometer via `getSpeedKmh`), **reset** key (respawn the car), look
  polish (materials, maybe skid marks if cheap).

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
