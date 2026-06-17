# physics2 — a clean physics module on geom2 (architecture + plan)

Supersedes the seed notes in `physics-geom2-integration-notes.md` and the earlier
"redesign in place" sketch. Decision (Session 71): **build a new `physics2`
module from scratch on top of geom2, migrate the demos, retire the legacy
`physics` module** — the same pattern geom2 used to replace geom. The valuable
domain logic (paper folding, soft-body config, constraint setup) is Bullet-level,
not `geom::SceneObject`-level, so it *transplants* into the new architecture; we
rebuild the skeleton, not the math.

Paper manipulation (`PhysicsPaper3`) is the crown jewel and must keep working
throughout (bridge-first, modernize later).

---

## 1. Why a new module (not a retrofit)

The rot is in the inheritance, not the algorithms: `PhysicsObject :
geom::SceneObject` spreads an object's pose across four homes (SceneObject /
Bullet body / MotionState / mirror node) and forces the
duplicated-geometry + per-frame-mirror bridge (`PhysicsScene2`). Drivers,
threading, and the render/sim decoupling all fight that base class if retrofitted.

geom2 needs **no changes** to host physics (per-node local transform with
on-demand world composition, per-node geometry versioning, `Scene2::findObject`
picking, BVH, thread-safe locking are all already there) **except** one new,
general mechanism: attachable **Drivers** + a per-frame `sync` pass (§3). That
mechanism is not physics-specific and pays for itself outside physics.

Build order: `geom2` → `physics2` (drivers drive geom2 nodes; geom2 stays
Bullet-free). Slots where `physics` sits in the current build reorder.

---

## 2. Threading model — physical accuracy decoupled from render speed

**Decoupling principle:** fixed-timestep simulation + render-side interpolation.
Physics advances in fixed `dt` ticks (e.g. 1/120 s) — deterministic, stable,
framerate-independent. The render thread runs at its own rate and **interpolates**
between the two most recent physics states by `alpha = accumulator / dt`. A 30 fps
and a 144 fps render see the *same* simulation, just sampled differently.

**Golden rule:** the physics thread *never* touches a geom2 node or GL. Nodes and
the renderer's GL cache belong exclusively to the UI thread. The two sides
communicate only through two channels:

```
  PHYSICS THREAD (per world)            MEMBRANE                UI / RENDER THREAD
  ─────────────────────────       ──────────────────       ────────────────────────
  fixed-dt loop:                  StateBuffer               render loop (own rate):
    drain command queue           (double/triple-buffered     read latest 2 snapshots
    world.step(dt)                 snapshots:                  alpha = acc/dt
    MotionState → write pose        [body → pose]              lerp pos / slerp rot
      into BACK snapshot            [soft → vertices]          → write node transform
    swap front/back (atomic)        seq/version)               markGeometryDirty()
    sleep to hold rate                                         Renderer::render()
         ▲                                                            │
         └─────── command queue (add/remove/grab/kinematic) ◀─────────┘
```

1. **`StateBuffer` (physics → render), lock-light.** Each completed step publishes
   per-body poses (and soft-body vertex arrays for paper) into a back buffer, then
   atomically swaps. Render always reads a *complete* snapshot, never half-written,
   never blocking the sim. Triple-buffering means physics never waits on render.
2. **Command queue (UI → physics).** Structural/control mutations — `addBody`,
   `removeBody`, mouse-grab target, **kinematic transform** — are enqueued on the
   UI thread and drained by the sim thread between steps.

**Pose authority, correctly stated:** the *physics state* is the source of truth;
the geom2 node is a **derived, interpolated view** of it. This is the definition
that survives the thread split (the earlier "MotionState writes the node directly"
does not — the node must only be written on the UI thread).

**Kinematic / user-controlled bodies (the maze fix):** the UI does not write the
body. It posts "maze pose = X" to the command queue; the sim thread feeds it to
the `MotionState` (what Bullet actually reads for kinematic bodies); render
interpolates. Collision *and* render tilt consistently; ball rolls.

**Worlds and threads — three independent axes:**
1. **one world = one sim thread** (baseline; `btDiscreteDynamicsWorld` is not
   thread-safe, so a world is owned by exactly one thread). This alone gives the
   render decoupling. Start here.
2. **N independent worlds, N threads** — separate non-interacting sims (paper +
   rigid rig + robot) each on its own thread, each publishing its own `StateBuffer`
   into the *same* Scene2. Free, because Bullet worlds never share state. (This is
   the "a / a,b,c" question — answered by composition, see §6.)
3. **one world, parallel inside `step()`** — Bullet `btDiscreteDynamicsWorldMt` +
   `btTaskScheduler`, an orthogonal per-world `setParallel(true)` opt-in, *not* a
   separate sim thread.

**The world owns its loop:** `start()` / `pause()` / `stop()` / `setRate()` for
the threaded case, plus **`stepOnce(dt)`** for a deterministic, single-threaded
mode (tests, headless — no render thread to decouple from).

---

## 3. The Driver mechanism (geom2, general)

A node holds zero or more **Drivers** — attachable internals that each get a
lifecycle + a handle back to their node. Lives in geom2, knows nothing about
Bullet.

```cpp
namespace geom2 {
  class Driver {                         // attachable behaviour bound to one Node
    Node *m_node = nullptr;
  public:
    virtual ~Driver() = default;
    Node *node() const { return m_node; }
    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void sync(double dt, double alpha) {}   // UI thread, each render frame
    friend class Node;
  };
}

// Node gains:
template<class T,class...A> std::shared_ptr<T> Node::addDriver(A&&...);
template<class T> T* Node::getDriver() const;        // first of type
const std::vector<std::shared_ptr<Driver>>& Node::drivers() const;

// Scene2 gains the one new concept — a per-frame UI-side pull pass:
void Scene2::sync(double dt);            // pre-order walk → driver->sync(dt, alpha)
```

The driver's "interface to its parent" is just `node()->setTransformation(...)`,
mesh access + `markGeometryDirty()`, visibility, children — the node never needs
to know what a driver *is*.

**Naming chosen** (`Driver`, not `Component` — ICL is the *Image Component
Library*; "Component" is poisoned. Not `Behavior` — too general, invites
game-engine scope creep, and mismatches the `XxxDriver` concretes):

| Concept | Name |
|---|---|
| Abstract attachable (geom2) | **`Driver`** — `node->addDriver<T>()`, `getDriver<T>()` |
| Physics rigid / soft / paper | **`RigidBodyDriver`**, **`SoftBodyDriver`**, **`PaperDriver`** |
| Membrane | **`StateBuffer`** |
| Sim advance (sim thread) | **`PhysicsWorld::step(dt)`** |
| Render-side pull (UI thread) | **`Scene2::sync(dt)`** / **`Driver::sync(dt, alpha)`** |
| The world | **`PhysicsWorld`** |
| Render+sim facade | **`PhysicsScene`** (physics2) |

Distinct verbs on purpose: the world **steps**, the scene **syncs**. Never two
different "update"s.

**Two kinds of driver:**
- **World-free** (animator, billboard, attractor): no Bullet → `node->addDriver<SpinDriver>(rate)`. Does its work entirely in `sync()`.
- **World-bound** (physics): needs a `PhysicsWorld` → born from the world factory (§5), which internally calls `node->addDriver(theDriver)` for the render-side wiring and adds the Bullet registration.

A physics driver **splits across two threads** (documented contract, single class
with thread-partitioned members):
- *sim-thread-only*: owns `btRigidBody` + `MotionState`, writes its `StateBuffer` slot.
- *UI-thread-only*: `sync(dt, alpha)` reads the slot, interpolates, writes `node()`.

---

## 4. Units / scale policy

Promote the mm→Bullet factor off the global `#define` onto `PhysicsWorld`:
`setUnitScale(float)` / `getUnitScale()` (default `0.01`, = current behaviour).
Add a **sub-margin guard**: on shape creation warn if any extent falls below
Bullet's collision margin in world units (the 3 mm floor → 0.03-unit → NaN trap).
Optional `autoFitScale(boundsMm)`. Air-density cubic scaling stays *derived* from
`unitScale` (do not reintroduce the 1000× soft-body drag bug). Port the
`applyTorque`/`applyTorqueImpulse` **double-scale fix** (legacy used `icl2bullet()`
twice).

---

## 5. World factory (no world-passing, no enum)

The world owns the Bullet world *and* the command queue, so body creation is a
structural mutation that goes through the queue anyway — the world is the honest
factory.

```cpp
// generic, extensible — injects *this as the world, attaches to node, typed return
template<class D,class...A> D* PhysicsWorld::addDriver(std::shared_ptr<Node> node, A&&...);

// convenience wrappers (forward to the template) for the common cases
RigidBodyDriver* PhysicsWorld::addRigidBody(node, float mass);
SoftBodyDriver*  PhysicsWorld::addSoftBody(node, ...);
PaperDriver*     PhysicsWorld::addPaper(...);
```

`world.addRigidBody(ball, 1.0f)` for the 90% case; `world.addDriver<MyDriver>(node,
…)` for a third-party driver another module can't add a typed method for. Typed,
not `createDriver(enum)` — driver ctors are heterogeneous (mass vs. vertices vs.
paper corners) and an enum would hand back a `Driver*` you'd have to cast.

**Collision shape — derive by default, explicit overload for proxies:**
- `world.addRigidBody(sphereNode, mass)` → reads `SphereNode` → sphere shape;
  `MeshNode` → convex hull. One representation, least typing, single source of truth.
- `world.addRigidBody(node, mass, shape)` → explicit cheap proxy that differs from
  the detailed visual.

**Ownership / RAII:** the **node owns the driver** (shared_ptr in its driver
list); the driver owns its Bullet body and enqueues register/unregister in
`onAttach`/`onDetach`. Destroy the node → the body leaves the world.

---

## 6. The `PhysicsScene` facade (composition, not inheritance)

`PhysicsWorld` does **not** inherit `Scene2`. That would re-fuse exactly the
boundary the threading model draws: thread ownership collides (Scene2 is
UI-owned, world is sim-owned), it forces a 1:1 world↔scene (killing the N worlds :
1 scene flexibility of axis 2), and it makes a god-object of two unrelated
responsibility sets — the `PhysicsObject : SceneObject` mistake one level up.

The one-call convenience comes from **HAS-A**, a thin coordinator that owns one
Scene2 + one PhysicsWorld and sequences them:

```cpp
namespace physics2 {
  class PhysicsScene {                       // owns a Scene2 + a PhysicsWorld
    RigidBodyDriver* add(std::shared_ptr<Node> node, float mass) {
      m_scene.addNode(node);                       // render side
      return m_world.addRigidBody(node, mass);     // sim side + driver wired
    }
    void  sync(double dt);   // forwards to Scene2::sync (UI thread)
    void  start();           // forwards to the world's sim loop
    Scene2&       scene();   // escape hatches: advanced / multi-world case
    PhysicsWorld& world();
  };
}
```

Single-world apps use the facade (`scene.add(ball, 1.0f)` = node in scene + body in
world + driver wired, one call). The advanced a/b/c case drops to a raw `Scene2` +
several `PhysicsWorld`s and wires drivers itself.

---

## 7. New constituents (prioritized)

| # | Capability | Approach | Serves |
|---|---|---|---|
| 1 | **Collision-shape debug draw** | `btIDebugDraw` → geom2 line overlay node, world toggle | would have caught the maze collision/render split immediately; diagnostic backbone |
| 2 | **Triggers / sensors** | `SensorDriver` over `btGhostObject` (+ ghost pair callback); overlap begin/stay/end, no response | maze holes, robotics interaction zones |
| 3 | **Collision filtering** | groups/masks on driver add | sensors vs. solids, ragdoll self-collision |
| 4 | **Raycast vehicle** | `btRaycastVehicle` `VehicleDriver` replacing the broken hinge-wheel car | fixes `physics-car` |
| 5 | **Richer contact events** | begin/stay/end stream off the dispatcher manifold | games/robotics sims |
| 6 | **Force fields** | world-level `ForceField` (wind, buoyancy, point attractor) applied each step | generalizes water-rocket drag + paper `VertexAttractor` |
| 7 | **Soft–rigid anchors** | wrap `btSoftBody::appendAnchor` | pin paper corners to rigid bodies / a gripper |
| 8 | **CCD toggle** | per-body `setCcdMotionThreshold`/`SweptSphereRadius` | maze ball tunneling, fast small objects |

Deferred: heightfield terrain, character controller.

---

## 8. Paper manipulation — preservation

`PhysicsPaper3` (fold-aware link splitting, FoldMap, self-collision, LinkState
tagging) does **not** depend on the pose-sync mess: soft bodies have identity
pose; their mesh is driven from Bullet node positions each frame. In physics2 it
becomes a **`PaperDriver`** on a `MeshNode`. De-risk by **bridging first**: the
driver initially wraps the existing soft-body/fold construction (transplanted from
`PhysicsPaper3.cpp`) to get it rendering + interacting under the new system, then
modernize the internals later. Close the `// TODO IMPLEMENT LOCKER`
thread-safety gaps in the paper mouse handling during Phase 5 (picking).

---

## 9. Phases

- **Phase 0 — geom2 Driver mechanism. ✅ LANDED (Session 71).** Abstract
  `geom2::Driver` (`Driver.{h,cpp}`, vtable anchor); `Node::addDriver<T>()`/
  `getDriver<T>()`/`getDrivers()`/`removeDriver()` (drivers not copied on
  deepCopy); `Scene2::sync(dt, alpha=1)` pre-order traversal (parent before
  children). Demo `geom2-driver-spin` (world-free `SpinDriver`, parent+counter-
  spinning child) builds/links/inits. Verified headless by `tests/
  test-geom2-driver.cpp` — 7 GL-free tests (attach/onAttach, typed lookup,
  removeDriver/onDetach, sync dt+alpha forwarding, transform-driving,
  group-child recursion, not-copied); full suite 884/884 green.
- **Phase 1 — physics2 skeleton. ✅ LANDED (Session 71).** New `icl/physics2/`
  module (built after geom2, gated bullet+qt). `Units` (per-world scale, default
  0.01); `StateSlot` membrane (double-buffered, position-lerp/rotation-slerp);
  `CollisionShapeFactory` (CuboidNode→box, SphereNode→sphere, else convex hull);
  `RigidBodyDriver` (geom2::Driver owning btRigidBody + a MotionState that
  publishes the slot each step; UI-side `sync` samples it into the node);
  threaded `PhysicsWorld` (fixed-timestep loop, `start`/`stop`/`stepOnce`,
  gravity/unit-scale, lock-serialized `addBody`/`removeBody`, typed-template
  factory `addDriver<D>` + `addRigidBody`); `PhysicsScene` facade (owns Scene2 +
  PhysicsWorld, one-call `add()`, composition not inheritance, member order =
  world-first). Demo `physics2-scene` (physics on its own thread; run loop only
  `sync`+render). Verified by `tests/test-physics2.cpp` — 4 deterministic
  `stepOnce` tests (units round-trip; box falls 500→rests ~60; sync writes node;
  custom unit scale); full suite 888/888; threaded start→run→stop smoke clean.
  **Scope adjustments:** (1) the UI→sim *command queue* is deferred to Phase 2
  (where kinematic targets / mouse grab genuinely need it) — Phase 1 uses
  lock-serialized add/remove, which is correct; the load-bearing sim→render
  `StateBuffer` membrane IS present. (2) world is `btDiscreteDynamicsWorld`
  (rigid only); Phase 3 upgrades it to `btSoftRigidDynamicsWorld` for paper.
- **Phase 2 — kinematic + debug draw. ✅ LANDED (Session 71).** UI→sim
  **command queue** on `PhysicsWorld` (`enqueue`, drained at the start of each
  `stepOnce` before stepping). `RigidBodyDriver::setKinematic(bool)` (CF_KINEMATIC
  + DISABLE_DEACTIVATION) + `setKinematicTransform(Mat)` (routes through the queue
  so the sim thread updates the MotionState target — what Bullet reads — and
  publishes the render slot together). **Collision-shape debug-draw overlay**
  (`btIDebugDraw` → `PhysicsWorld::getDebugLines()` in ICL units →
  `PhysicsScene::setDebugDrawEnabled` rebuilds a line `MeshNode` each sync).
  Interactive demo `physics2-tilt` (slider-tilted kinematic platform, balls roll,
  debug toggle). Verified by 3 new tests: **`kinematic_tilt_rolls_ball`** (tilted
  kinematic ramp → ball rolls, |Δx|>80 — the maze fix), `kinematic_flat_holds_ball`
  (flat control → ball still), `debug_lines` (non-empty wireframe). Full suite
  891/891; tilt-demo threaded smoke clean. **Deferred:** the full `physics-maze`
  geometry port (compound tray + holes) is GUI-heavy and unverifiable in the
  sandbox — the kinematic test + tilt demo cover the emblematic fix; port the
  maze proper on a real display (needs compound bodies, see Phase 4).
- **Phase 3a — soft-body foundation. ✅ LANDED (Session 71).** `PhysicsWorld`
  upgraded to `btSoftRigidDynamicsWorld` (soft-body collision config +
  `btSoftBodyWorldInfo` with scale-correct air density + scaled gravity);
  `addSoftBody`/`removeSoftBody`/`getSoftBodyWorldInfo`. **Post-step capture
  hooks** (`addCapture`/`removeCapture`, invoked after `stepSimulation` on the
  sim thread) + `SoftStateBuffer` (vertex-array membrane). `SoftBodyDriver`
  (geom2::Driver owning a `btSoftBody` cloth patch; builds the MeshNode topology
  from soft-body nodes/faces once, captures node positions each step, copies the
  snapshot into the MeshNode + recomputes normals each `sync`); corners re-pinned
  by index after `setTotalMass` (avoids the un-pin gotcha). `PhysicsScene::addCloth`
  factory. Demos `physics2-cloth` (cloth drapes over a box, pinned banner, debug
  toggle). Verified by `physics2.cloth_sags` (pinned cloth sags, pinned corners
  hold); 8 physics2 tests / full suite 892/892; threaded soft-body smoke clean.
- **Phase 3b — fold-aware paper (`PhysicsPaper3`) + water-rocket. ⏳ TODO.**
  `PaperDriver` bridging the existing `PhysicsPaper3` soft-body/FoldMap/
  link-splitting/self-collision construction (transplant the Bullet-level logic,
  not a rewrite); the paper-space mouse interaction; `physics-water-rocket`
  (soft parachute + rigid rocket). Large + GUI-interactive (fold visuals,
  drag-to-fold) → best done on a real display. Also fix the legacy `applyTorque`
  double-scale and close the `ManipulatablePaper` locker TODOs when porting.
- **Phase 4a — constituents (batch 1). ✅ LANDED (Session 71).**
  **CCD** (`RigidBodyDriver::setCcd`, ICL units → Ccd motion threshold + swept
  sphere); **collision filtering** (`RigidBodyDriver::setCollisionFilter(group,
  mask)` → `PhysicsWorld::setBodyFilter` remove+re-add); **contact events**
  (`PhysicsWorld::setContactCallback`, post-step manifold scan, driver pointers
  via body userPointer = `geom2::Driver*`); **force fields**
  (`PhysicsWorld::addForceField`/`removeForceField`, applied to each dynamic body
  pre-step at its position). Verified by 4 tests: `ccd_no_tunnel` (30 m/s ball
  doesn't tunnel a thin floor), `collision_filter` (filtered pass-through +
  colliding control), `contact_events` (callback fires on landing, sees the box
  driver), `force_field` (constant +x field drifts a body). 14 physics2 tests /
  full suite 898/898.
- **Phase 4b — constituents (batch 2). ✅ MOSTLY LANDED (Session 71).**
  **Sensors/triggers** (`SensorDriver` over `btPairCachingGhostObject` +
  `btGhostPairCallback` in the world; `PhysicsWorld::addCollisionObject`;
  `PhysicsScene::addSensor`; `getOverlappingDrivers()`); **soft-rigid anchors**
  (`SoftBodyDriver::anchorNode`/`cornerNodeIndex` → `btSoftBody::appendAnchor`,
  queued — pin a cloth corner to a rigid/kinematic body). Verified:
  `sensor_detects_passthrough` (zone detects a body, doesn't deflect it),
  `anchor_holds_corner` (anchored corner stays up, rest sags). 15 physics2 tests /
  full suite 899/899. **Still TODO:** raycast vehicle (#4, `btRaycastVehicle` →
  fix `physics-car`) — larger + GUI-bound, best on a real display.
  **Bug found + fixed (was misattributed to Bullet):** the intermittent
  soft-vs-rigid "crash" was a **use-after-free in `~PhysicsWorld`** — `ghostCb`
  (the ghost-pair callback added in 4b) was deleted *before* the world/broadphase,
  whose teardown calls `btHashedOverlappingPairCache::removeOverlappingPair`,
  which dereferences it. Pinpointed via an AddressSanitizer reproducer (lldb
  can't attach in the sandbox); isolated Bullet soft-rigid is clean. Fix:
  delete `ghostCb` LAST. `cloth_rests_on_box` test + the demo's drape-over-box
  restored; 16 physics2 tests / full suite 900/900. (Cloth *visual* quality —
  the pleating — is still the separate defaults-policy tuning matter.)
- **Phase 5 — unified picking. ✅ LANDED (Session 71).** `physics2::PhysicsMouseHandler`
  (extends `geom2::Scene2MouseHandler` for camera nav): Shift+Left press raycasts
  via `Scene2::findObject(ray)`, resolves the hit node's driver via
  `node->getDriver<RigidBodyDriver>()`, and grabs the dynamic body with a
  `btPoint2PointConstraint` spring (impulseClamp/tau); drag moves the grab target
  along a camera-facing plane; release lets go. All Bullet mutations
  (add/move/remove constraint, activation) routed through the world command
  queue — sim-thread safe. Installed in `physics2-scene` + `physics2-tilt`
  (grab the rolling balls). Verified by `physics2.pick_resolves_driver` (centre-
  pixel `findObject` → correct `RigidBodyDriver`); 9 physics2 tests / full suite
  893/893; grab-demo smoke clean. (Soft-body / paper-space picking arrives with
  Phase 3b.)
- **Phase 6 — retire legacy.** Delete `physics` (incl. `PhysicsScene`/
  `PhysicsScene2`) and the dead `geom::Scene` physics path once all demos are on
  physics2.

## Cross-cutting TODO — a physics defaults policy (parameter standards)

Right now soft-body (and some rigid) parameters are **hand-picked magic numbers**
in `SoftBodyDriver` (margin 4, kLST 0.4, piterations 5, …). The S71 cloth-on-box
still looks wrong (radial pleats / puffing) because these aren't principled.
Establish *standards* instead of tuning constants:
1. **Derive from physics, scale-independent.** Margin = a fixed cloth thickness
   (~1–2 mm); mass from areal density (g/m²) × area, not an arbitrary total;
   solver iterations from a stability target (timestep × stiffness). A default
   should mean something and survive a unit-scale change.
2. **Expose, don't bury.** A named `SoftBodyConfig` struct (defaults + ranges),
   overridable per cloth — mirror legacy `SoftObject` exposing `kDP/kDF/kLST` as
   `Configurable` properties. Same for rigid (restitution/friction/CCD/iterations).
3. **Settle empirically once, then bake.** A small interactive tuning harness
   (sliders over the config) across canonical scenes (drape, hang, flag, drop);
   freeze the values that look right as documented defaults with rationale.
   Also revisit self-collision (CL_SELF/VF_SS) — its absence is part of the
   pleating. Do this deliberately, not by accretion.
4. **Soft-vs-rigid collision works now** (the "crash" was a `~PhysicsWorld`
   use-after-free, fixed — see Phase 4b). What remains is *visual quality*: a
   flat cloth draping on a box pleats into sharp folds. Tuning = soft margin,
   solver iterations, self-collision (CL_SELF/VF_SS), cluster vs SDF. Same
   parameter-standards work; no longer a stability blocker.

## Cross-cutting TODO — `DefaultScene` (SceneType presets) + default physics scene

*(Sketch — Session 71. Design agreed; not yet implemented. Decisions baked in:
preset-driven Configurable (not a flat knob soup); sky-as-background is in scope;
**it's a `Scene2` subclass that replaces `DemoScene2`**, not a decorator.)*

### Problem

Every demo hand-rolls the same furniture (ground + lights + camera), each slightly
differently and usually worse than `DemoScene2`. The physics2 demos in particular
ship a single light, **no ground/wall, black clear color**
(`physics2-scene.cpp:39`) — objects float in a void. We want one type that *is* a
**nice-looking, coherent world** out of the box, picked from a small set of
high-level *scene types*, with only a few global knobs exposed.

### Shape — a `Scene2` subclass that replaces `DemoScene2`

`geom2::DemoScene2` already builds the good furniture (checkerboard ground + back
wall, 4-point soft-shadow rig, framed camera) and is itself a `Scene2` subclass —
but it's welded to the model-*viewer* job (file-load + auto-scale-to-400mm, Y-up).
`DefaultScene` generalises it and **takes its place**:

```cpp
namespace icl::geom2 {
  /// A Scene2 that furnishes itself: ground, walls, props, lights, camera, sky,
  /// chosen by a high-level SceneType preset. Configurable (extends Scene2's own
  /// Configurable) → preset + a few global knobs surface in the Prop UI and
  /// rebuild the furniture live. Up-axis aware (Y for viewers, Z for physics).
  /// Replaces DemoScene2 (whose viewer behaviour becomes SceneType::Studio +
  /// an optional loadAndFit(files) helper).
  class DefaultScene : public Scene2 {
   public:
    enum class SceneType {
      Void,       ///< nothing but a camera (content brings its own world)
      Studio,     ///< neutral checkerboard ground + back wall, lamp rig (the DemoScene2 look)
      Landscape,  ///< green ground, scattered rocks / trees / flowers, sun + sky
      Room,       ///< floor + 4 walls + ceiling, ceiling lamp, a tabletop to place things on
    };
    explicit DefaultScene(SceneType = SceneType::Studio);
    void setSceneType(SceneType);              // also a "scene type" Configurable property
    /// optional viewer convenience: load model files, auto-fit, keep the preset furniture
    void loadAndFit(const std::vector<std::string> &files, const std::string &rotation = "");
   private:
    void rebuildEnvironment();  // clears m_furniture, repopulates per current preset + knobs
    std::vector<std::shared_ptr<Node>> m_furniture;  // only the furniture it owns (not user content)
  };
}
```

Why subclass (the earlier "decorator" worry was unfounded): `physics2::PhysicsScene`
composes its scene **by value** (`Scene2 m_scene`, `PhysicsScene.h:90`). Upgrade
that member to `DefaultScene` and physics inherits the entire preset/furniture API
through `scene()` for free — no decorator indirection, and the type *is* a scene so
the name reads right. `m_furniture` is tracked separately from user-added nodes so
`rebuildEnvironment()` can swap presets without touching content.

**Exposed Configurable properties (deliberately small — "live, but not everything"):**
- `scene type` (menu: Void / Studio / Landscape / Room) — the main knob; rebuilds.
- `up axis` (Y / Z) — Z is the physics convention; flips ground plane + sky + camera.
- `extent` (characteristic world size, mm) — drives ground size, light radius, camera dist.
- `ground` (on/off), `shadows` (on/off), `sky background` (on/off), `coordinate frame` (on/off).

Per-preset internals (tree count, wall color, checker palette, tabletop height…)
are **chosen by the preset**, *not* surfaced as flat properties. A preset is just a
private builder method (`buildStudio()`, `buildLandscape()`, `buildRoom()`,
`buildVoid()`) that appends nodes to `m_owned` and adds lights/camera.

### SceneType presets (initial set)

| Preset | Ground / container | Props | Lights | Camera | Use |
|---|---|---|---|---|---|
| `Void` | none | none | 1 key + soft fill | framed to bounds | content owns the world; cleanest |
| `Studio` | checkerboard ground (faded edges) + back wall | none | warm key + cool fill + rim + top, soft shadows | 3/4 view | the current `DemoScene2` look; default for viewers |
| `Landscape` | green ground (large, fading) | rocks, trees (trunk+cone), flowers | sun (warm, shadowed) + sky fill | eye-level 3/4 | nature / point-cloud / octree demos |
| `Room` | floor + 4 walls + ceiling (open front); **floor is the play surface** | a table as a prop (not the spawn surface) | ceiling lamp (visible fixture) + soft fill | looking in through open wall | indoor physics play area |

Reuse what exists: `Studio` ≈ today's `DemoScene2::setup` furniture;
`Landscape` ≈ `DemoScene2::setupNatureScene` (already builds green ground + gray
rocks + cylinder/cone trees — lift it in, add flowers). `Room`/`Void` are new.

### Edge polish (the "problems" you're seeing)

- **Ground edges:** the current hard square reads badly. Add a radial **alpha
  fade-out** toward the rim (or a beveled border) so it dissolves instead of
  ending in a sharp line. Applies to Studio + Landscape grounds.
- **Sky background:** the GL renderer *already has* a procedural sky —
  `sampleSky(dir)` (`Renderer.cpp:139`, zenith→horizon→ground gradient) — but it's
  only sampled for **reflections + diffuse ambient**; the visible background is a
  flat `glClearColor` (`Scene2.cpp:196`). Wire the **same** model as a fullscreen
  background pass (a Renderer change: draw a screen-filling gradient from per-pixel
  view rays *before* the scene). One model → backdrop and reflections match. Must
  be **up-axis aware** (it's currently `dir.y` only). Toggle falls back to flat clear.
- **Lamp rig:** Studio/Room get warm key + cool fill (+ rim/top) with soft shadows
  — `DemoScene2` already does this; just parameterize off `extent` and up-axis.

### Default physics scene

`PhysicsScene` composes a `DefaultScene` (upgrade `m_scene`'s type from `Scene2`).
`physics2::PhysicsScene::setupDefault(SceneType, opts)`:
- selects the preset on the internal `DefaultScene` (`m_scene.setSceneType(...)`), **and**
- adds **matching static colliders** for whatever the preset's "solid" furniture is
  — Studio/Landscape → a ground-plane collider coincident with the visual ground;
  Room → floor + walls colliders (bodies drop onto the **floor**; the table is a
  static collider prop, not where `add()`ed bodies spawn);
  Void → just a ground plane (or nothing).
- sets gravity along the chosen up-axis, a sensible `setBounds(extent)`, and leaves
  the scene **ready to `scene.add(node, mass)`**.

This is the key payoff: the visual furniture and the physics colliders come from
**one source**, so what you see is what bodies actually rest on (the maze-tilt class
of "render vs collision disagree" bug can't recur here).

### Refactor / sequencing

1. **DONE (Session 71)** — Landed `DefaultScene : Scene2` with `Studio` + `Void`,
   the sky-as-background pass, and faded ground edges:
   - `Renderer::setSkyEnabled/setSkyUp` + a fullscreen sky pass reusing the
     existing `sampleSky` model (up-axis aware), drawn into the active color
     target so SSR reflects the backdrop (`Renderer.cpp`).
   - `geom2::DefaultScene` — `SceneType{Void,Studio}`, up-axis aware (Y/Z),
     Configurable knobs `scene type / up axis / ground / sky background /
     shadows / SSR` (all the nice features **on by default**), `setExtent`.
     Furniture tracked in `m_furniture`, rebuilt live without touching user content.
   - Faded-edge checkerboard ground (`makeFadedChecker`) + back wall, 4-light
     lamp rig (Studio) / key+fill (Void), framed camera (created once, updated
     in place on rebuild).
   - `Scene2::removeNode` now also purges the lights vector (a removed light
     must stop shining) — prerequisite for clean live rebuilds.
   - Demo `geom2-default-scene` (live Prop panel) + 6 headless tests
     (`test-geom2-default-scene.cpp`, structure + no-leak round-trip). Suite green.
   - **Unverified (needs real display):** the sky pass, faded edges, lamp rig —
     sandbox has no GL context. Construction/structure/threading are verified.
2. **TODO** — Fold the existing `DemoScene2` furniture in as `Studio`
   (+ `loadAndFit` for the viewer job) and `setupNatureScene` as `Landscape`;
   then **delete `DemoScene2`** and repoint its users (`scene-to-pointcloud`,
   `raycast-octree`, any `-i` viewer) at `DefaultScene`. Add `Room` + flowers.
3. **TODO** — Upgrade `PhysicsScene::m_scene` to `DefaultScene`, add
   `setupDefault`; migrate `physics2-scene/-tilt/-cloth` to it.

NOTE: GUI render correctness is unverifiable in this sandbox (GL context creation
fails) — the sky pass + faded edges + lamp rig all need a real display to confirm.
Logic/units/threading/sync are verifiable via `stepOnce` (deterministic, no render
thread) + tests; the collider/visual coincidence is checkable headless.

Branch `further-restructuring-and-cleanup`.
