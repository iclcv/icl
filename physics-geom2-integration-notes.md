# geom2 ↔ Bullet physics integration — pain points & improvement-plan seed

Written at the end of Session 70 (geom→geom2 migration). We paused here because
the physics/geom2 integration doesn't feel *right or intuitive* — the maze made
this concrete: after a long debugging chain the maze now **renders** its tilt but
the **ball doesn't roll**, because the maze's collision and its rendered node end
up driven by two different, out-of-sync transforms. That symptom is the tip of a
structural issue worth redesigning before pushing further.

## The trigger (maze tilt) — what it exposed
To make an interactive tilt-maze we needed a mass-0 maze that the user moves each
frame. Getting there required, in sequence:
1. floor falling through → made the floor a compound child (was a separate
   constrained body whose `SixDOFConstraint` diverged to NaN);
2. ball NaN on contact → the moved mass-0 compound had to be flagged KINEMATIC;
3. maze rendered flat → kinematic bodies are positioned by Bullet from their
   **MotionState**, but `rotate()`/`setTransformation()` write the **body** world
   transform; `updateSceneObject()` reads the body back (now identity) → render
   flat. Worked around by driving the render node from the known tilt.
4. …but now the **collision doesn't actually tilt** (MotionState never updated),
   so the ball doesn't roll, and the visual (faked tilt) and physics (flat)
   disagree. Plus the tilt resets on mouse-release (minor UX).

Every step was a workaround for the same root cause: **no single, consistent
source of truth for an object's pose across SceneObject / Bullet body /
MotionState / geom2 node.**

## What works well today (keep)
- `PhysicsScene2` auto-mirroring bridge for **dynamic rigid** objects:
  `physics-scene`, `physics-constraints` simulate + render correctly.
- `geom2::fromSceneObject` universal converter (handles arbitrary SceneObject
  geometry: primitives, hulls, soft meshes, compounds → MeshNode/GroupNode).
- Per-node geometry dirty flag (`GeometryNode::markGeometryDirty`) for dynamic
  meshes (soft bodies) — re-uploads only changed nodes.

## Core friction (what bothers us)
1. **Multiple sources of truth for pose.** An object's transform lives in up to
   four places: `geom::SceneObject::m_transformation`, the Bullet body world
   transform, the `MotionState`, and the geom2 mirror node. They're synced by
   `updateSceneObject()` (reads body → SceneObject) + `syncSceneFromPhysics()`
   (reads SceneObject → node). Kinematic / user-driven bodies break the chain
   (body vs MotionState), so physics and render diverge.
2. **Duplicated geometry.** Physics still uses **legacy `geom::SceneObject`**;
   geom2 gets a *copy* (mirror node) rebuilt by the converter and re-synced each
   frame. Two representations that can (and did) drift: colours
   (`[0,1]` vs `[0,255]` double-scale bug), transforms, the maze.
3. **The bridge assumes `getTransformation()` is an authoritative read-back.**
   True for plain dynamic bodies, false for kinematic/controlled ones (we had to
   add `getMirrorNode()` to override the render transform manually).
4. **Legacy conventions leak through the converter.** Colour ranges; and the
   ICL-mm → Bullet ×0.01 scale makes small scenes (maze: 3 mm floor = 0.03 Bullet
   units) fall *below Bullet's default collision margin* → NaN. Scale is a global
   and hard to reason about per scene.
5. **No first-class "physics+render object."** A thing that has geometry, a pose,
   a collision shape and a render node is stitched together from
   `geom::SceneObject` + Bullet body + `MotionState` + geom2 node, with sync glue.
6. **Kinematic / moved objects, picking and manipulation are ad-hoc.** Tilt-maze,
   moving platforms, mouse-grab each need bespoke handling; there's no clean
   "controlled body you move each frame, render + collision stay consistent."

## Design directions to weigh next session
- **One source of truth for pose.** Pick the authority (most likely the geom2
  node, or the Bullet body) and *derive* the rest. For controlled/kinematic
  objects, write to the **MotionState** (what Bullet actually reads) via a proper
  `setKinematicTransform()` that updates MotionState + body + node together.
- **Physics object owns a geom2 node directly** (no per-frame mirror copy):
  geometry exists once; physics writes the node's transform; collision shape is
  derived from (or paired with) that geometry. Removes the `geom::SceneObject`
  dependency from physics and the drift.
- **A clean controlled-body abstraction** (kinematic platforms, the tilt-maze):
  moved via MotionState, collisions correct, render follows, no manual override.
- **Unit/scale policy:** decide a per-world Bullet scale (or auto-scale) and
  guard against sub-margin geometry; surface it instead of a hidden global.
- **Unified interaction on Scene2:** picking + grab + manipulate built on the
  existing `Scene2::findObject` raycast, instead of per-demo mouse handlers.

## Open questions to settle before coding
- Should a physics object **be / own** a geom2 node (single representation), or
  keep the converter+mirror? (Leaning: own a node.)
- Where does the **collision shape** come from — derived from geom2 geometry, or
  specified explicitly alongside it?
- How to express **kinematic / user-controlled** bodies cleanly (the tilt-maze is
  the test case)?
- **Scale**: fixed per-world factor, auto-fit, or per-object margins?
- How much of legacy `geom::SceneObject`/`PhysicsObject` do we keep vs. rebuild
  on geom2?

## Status snapshot (committed this session)
- **physics-scene / physics-constraints** — working (render + simulate) on
  `PhysicsScene2` + `PhysicsMouseHandler2`.
- **physics-maze** — renders (walls/floor/ball/holes), interactive tilt **visible**
  but **collision doesn't tilt → ball doesn't roll** (the transform-sync problem);
  tilt resets on release. The emblematic case for the redesign.
- **physics-car** — renders + reset button, but hinge-wheel vehicle physics is
  broken (twisted wheels, won't drive). Logged as a separate physics TODO
  (Task #8) — likely needs `btRaycastVehicle`.
- **physics-paper / -paper3** — deferred (geom::Scene shadows/lights + offscreen
  `scene.render(0)` [Phase 0c] + ManipulatablePaper picking).
- Migration infra (Phase 0/1): build reorder, dirty flag, converter, bridge —
  all landed; 877/877 tests green throughout.

## Pointers
- Plan + phase status: `geom2-migration-plan.md`.
- Bridge: `icl/physics/PhysicsScene2.{h,cpp}`; converter:
  `icl/geom2/SceneObjectConverter.{h,cpp}`; renderer transform/MotionState:
  `icl/physics/{PhysicsObject,RigidObject,MotionState}.*`.
