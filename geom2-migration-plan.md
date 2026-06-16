# geom → geom2 migration plan

Full-arc plan to retire the legacy `geom` rendering layer in favour of the
`geom2` scene-graph + GL 4.1 Core renderer. Scope confirmed by user
(2026-06-16): **everything in next.md** — geom2 gaps, the ICLPhysics→geom2
bridge, porting all `geom::Scene` demos/apps, and ending the legacy scene layer.

Bridge design confirmed: **auto-mirroring** (option 1). A reusable helper class
mirrors physics/scene objects into geom2 nodes automatically. geom2 fully
replaces the geom *rendering* layer eventually; interim `geom`→`geom_old` rename
is acceptable but not required.

## Scope clarification — what "get rid of geom" actually means

`geom` is two things fused together:
1. **A rendering/scene-graph layer** — `Scene`, `SceneObject`(+subclasses),
   `GLRenderer`, `Primitive`, `SceneLight`, `SceneMouseHandler`, `DemoScene`,
   `Sky`, `ShaderUtil`. **This is what geom2 replaces and what we retire.**
2. **CV / geometry algorithms** — `Camera`, `ICP`, `Posit`, `PoseEstimator`,
   pose/RANSAC, point-cloud processing/segmentation, `ObjectEdgeDetector`,
   `Primitive3DFilter`, `GeomDefs`/`Geom.h` math. **This stays** — geom2 itself
   depends on it (`Scene2` uses `geom::Camera`, `Material`, `GeomColor`, `Vec`).

So the endgame is: delete the legacy *scene layer*, keep the geometry/CV core.
Whether the surviving core keeps the name `geom` or is renamed is a cosmetic
final step (Phase 5), not a blocker.

## Required prerequisite — build-order reorder

geom2 is currently built **after** physics (`meson.build`: physics @302, geom2
@305) and depends only on `qt`+`geom`. A physics→geom2 bridge in the physics
module is therefore impossible today. **Move geom2 to right after geom**, giving
`… geom → geom2 → markers → physics`. Safe: geom2 depends on neither markers nor
physics; markers/physics currently use only `geom::Scene`, not geom2. Mirror the
same reorder in the `targets/` block. (Step 0 below.)

---

## Phase 0 — geom2 foundational gaps

### 0a. Build-order reorder (above). Rebuild, confirm 877/877 still green.

### 0b. Per-node geometry dirty flag (replaces global `invalidateCache`)
Today `Renderer` caches each `GeometryNode`'s GL buffers keyed by raw pointer and
only rebuilds on `Renderer::invalidateCache()` — which clears **all** caches.
Dynamic meshes (soft bodies, cloth, point clouds) must call the global invalidate
every frame (see `physics-water-rocket.cpp:358`), rebuilding every node.

Fix:
- Add a monotonic `uint64_t geometryVersion` to `GeometryNode::Data` plus
  `markGeometryDirty()` (bumps the version) and `getGeometryVersion()`.
  `MeshNode`'s mutating paths (`ingest`, `clearGeometry`, `addVertex/…`, and a
  public `markGeometryDirty()` for in-place `getVertices()` edits) bump it.
- `Renderer::GeomCache` stores the version it was built from. In `renderNode`,
  if `node->getGeometryVersion() != cache.builtVersion`, rebuild **only that
  node's** buffers (re-`build()` in place) and update `builtVersion`.
- Keep `invalidateCache()` (still used by `Scene2::clear()` / material/texture
  swaps) but it's no longer needed for per-frame dynamic geometry.
- Update `physics-water-rocket.cpp` to call `chuteMesh->markGeometryDirty()` /
  `ropes` rebuild instead of `scene.getRenderer().invalidateCache()`.

Verify: water-rocket canopy still animates; no full-scene rebuild per frame.

### 0c. Offscreen `Scene2::render()` → `core::Img` (headless render-to-image)
The core-profile pipeline has no offscreen path; `geom::Scene::render()` (pbuffer)
is dead. Implement an FBO-backed offscreen render in geom2:
- `core::Img8u Scene2::render(int camIndex, const utils::Size &size)` (and/or an
  `ImgBase&` out-param) that creates/【reuses】an offscreen FBO (color+depth),
  binds it, runs `Renderer::render(...)` with the camera matrices, reads pixels
  back (`glReadPixels`, flip Y), returns the Img.
- Needs a current GL context. Document that callers must have one (Qt offscreen
  surface or the existing widget context); provide a small helper if needed.
- Port `icl/geom/demos/offscreen-rendering.cpp` to geom2 as the smoke test.

(0c is heavier than 0b; if it balloons, land 0a+0b first, then 0c.)

---

## Phase 1 — the auto-mirroring bridge

### 1a. Universal geometry converter (lives in geom2; geom2 already deps geom)
`geom2::fromSceneObject(const geom::SceneObject &so) -> std::shared_ptr<Node>`:
- Build a `MeshNode`: copy `so.getVertices()`, `getNormals()`, `getVertexColors()`,
  `getTexCoords()`; walk `so.getPrimitives()` (polymorphic `Primitive*`) and emit
  the matching geom2 primitives:
  - `LinePrimitive` → `addLine`
  - `TrianglePrimitive` → `addTriangle` (with normal/texcoord indices)
  - `QuadPrimitive` → `addQuad`
  - `PolygonPrimitive` → fan-triangulate
  - `TexturePrimitive`/grids → quad(s) (+ texture later; start untextured)
- Recurse `so.getChildren()` into a `GroupNode`; carry the child transform.
- Carry visibility, point size, line width, material/colour where available.
This single function covers every RigidObject (primitive helpers populate
vertices+primitives), RigidConvexHull (vertices, auto-normals), SoftObject
(OBJ → vertices+primitives), and compounds (children).

### 1b. `PhysicsScene2` bridge (physics module, post-reorder)
A convenience class owning a `geom2::Scene2` + `physics::PhysicsWorld`:
- `addObject(PhysicsObject*)`: add to the PhysicsWorld, build a mirror geom2 node
  via `fromSceneObject`, add it to the Scene2, and record a
  `{PhysicsObject* → shared_ptr<Node>}` mapping (note soft-body MeshNodes).
- `syncSceneFromPhysics()` (call each frame after `step()`):
  - rigid bodies → `node->setTransformation(bullet2icl(body->getWorldTransform()))`
  - soft bodies → re-read `btSoftBody::m_nodes[i].m_x` into the MeshNode's
    vertices, `createAutoNormals`, `markGeometryDirty()` (Phase 0b).
- Forward camera/light/getGLCallback through to the inner Scene2; provide a
  `getScene2()` escape hatch.
- Reuse helpers from `physics-water-rocket.cpp` (`bullet2icl`, `sbNode`); fold
  them into the bridge / `PhysicsDefs.h`.

Refactor `physics-water-rocket.cpp` to use `PhysicsScene2` as the proof
(should shrink substantially while rendering identically).

---

## Phase 2 — migrate physics demos onto the bridge
Port each to `PhysicsScene2` (or the bridge helpers), deleting the legacy
`PhysicsScene`/`geom::Scene` GL path usage:
- `physics-scene`, `physics-maze`, `physics-paper`, `physics-paper3`,
  `phyisics-car`, `phyisics-constraints`.
Soft-body demos (paper/paper3) exercise the MeshNode dynamic path; rigid demos
(scene/maze/car/constraints) exercise transform sync + constraints.
Retire `PhysicsScene` once no demo uses it (it multiply-inherits the dead
`geom::Scene`). `PhysicsMouseHandler`/`PhysicsPaper3MouseHandler` need a geom2
equivalent (Scene2 already has `Scene2MouseHandler`).

---

## Phase 3 — migrate geom + markers demos/apps
~32 geom + 5 markers targets render through `geom::Scene`. Group by pattern:
- **Plain scene viewers** (scene-graph, scene-object, scene-shadows, camera,
  texture-cube, superquadric, simplex-3D, animated-grid, generic-texture-coords):
  straight Scene→Scene2, SceneObject→nodes (reuse `fromSceneObject` or native
  geom2 primitive nodes).
- **Point-cloud apps** (point-cloud-viewer/-creator/-pipe/-primitive-filter/
  -define-world-frame, simple-point-cloud-viewer, kinect-*, rgbd-mapping,
  swiss-ranger): use `geom2::PointCloudNode`. Check feature parity
  (`PointCloudObject` vs `PointCloudNode`).
- **Cycles demos** (cycles-*): geom2 already has `CyclesRenderer`/
  `SceneSynchronizer`; point them at geom2.
- **Plot3D** (`plot-widget-3D`, `PlotWidget3D`/`Plot3D`/`PlotHandle3D`): library
  widget on `geom::Scene` — needs a geom2-backed reimplementation.
- **Markers** (camera-calibration[-planar], marker-detection, multi-cam-marker-
  demo): render calibration grids / pose overlays via geom::Scene → geom2.
- **gl-renderer-test / offscreen-rendering / ray-cast-octree / depth-camera-
  simulator / rotate-image-3D / show-scene / surf-based-object-tracking**:
  case-by-case.
Library headers to address: `DemoScene`→`DemoScene2` (geom2 has one),
`SceneLightObject` (→LightNode), `SceneMultiCamCapturer`, `Sky`.

---

## Phase 4 — retire the legacy scene layer
Once nothing renders through `geom::Scene`:
- Delete (or move to a quarantined `detail`/`geom_old`) the scene-layer files:
  `Scene`, `SceneObject`(+subclasses: Grid/CoordinateFrame/Complex…/
  OctreeObject/RayCastOctreeObject/PCLPointCloudObject/PointCloudObject*),
  `GLRenderer`, `SceneLight`, `SceneLightObject`, `SceneMouseHandler`,
  `DemoScene`, `Sky`, `ShaderUtil`, `Primitive` (if geom2 has its own — confirm),
  the geom `CyclesRenderer`/`SceneSynchronizer` (geom2 owns these now),
  `PlotWidget3D`/`Plot3D`/`PlotHandle3D` (after geom2 reimpl).
- Keep the CV/geometry core (`Camera`, ICP/Posit/pose, point-cloud algorithms,
  `ObjectEdgeDetector`, `Primitive3DFilter`, `GeomDefs`).
- Drop the now-unneeded `geom`→`geom2` dependency edge where possible; keep
  geom2→geom (for Camera/Material/GeomDefs).

## Phase 5 — (cosmetic, optional) rename
If desired, fold the surviving geom core into geom2 or rename. No functional
change. Defer / skip if low value.

---

## Build / test / verify notes
- Build: `CCACHE_DISABLE=1 PATH=~/Qt/6.11.0/macos/bin:$PATH ninja -C builddir -j 16`
- Tests: `builddir/bin/icl-tests -j 1` (expect 877/877 throughout).
- GUI smoke under sandbox: `QT_QPA_PLATFORM=offscreen` (GL context fails but
  arg/scene-build paths run); offscreen render-to-Img (0c) is the real headless
  check once it lands.
- Land in small, individually-green commits per sub-phase.

## Open questions / risks
- **PointCloudNode parity** with `PointCloudObject` (RGBD, segmentation
  colourings, feature fields) — verify before porting the kinect/rgbd apps.
- **Plot3D** is the heaviest library port (interactive 3D plotting widget).
- **Texture support** in the converter (TexturePrimitive/grids) — start
  untextured, add if a ported demo needs it.
- **Markers calibration overlays** may use geom::Scene picking / mouse handler
  features; check Scene2 hit-testing covers them.
- Soft-body self-collision / parachute deployment remains out of scope (see
  next.md "Future: parachute deployment").
