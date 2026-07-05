# geom / geom2 de-duplication — scoping (Session 98)

Audit to stage the retirement of the old `icl/geom/` scene graph in favour of the clean
`icl/geom2/` rewrite. Three parallel audits (consumers, feature-parity gaps, file
classification) + direct grep probing. **No code changed this session — scope only.**

## End-state: SPLIT into two modules — `cv3d` (Qt-free algorithms) + a scene/render module

Old `geom` conflates two things: 3D **computer-vision algorithms** and a 3D **scene
graph + renderer**. The end-state splits them, rather than folding everything into one module:

- **`cv3d`** (`icl::cv3d`) — the 88 CV-algorithm files + the shared geometry foundation
  (`Camera`, `ViewRay`, `PlaneEquation`, `GeomDefs`) + the `PointCloud` **data type**.
  **Qt-FREE** (verified: no CV-algorithm file includes Qt; the *only* thing coupling them to
  the GL world today is `PointCloudObjectBase : SceneObject`, which we sever anyway). Slots
  **below ICLQt** in the hierarchy → enables headless/server 3D vision.
- **`viz3d`** (`icl::viz3d`) — today's geom2: the scene graph, all
  `*Node` types, `PointCloudNode` (scene wrapper), `Renderer`/`CyclesRenderer`, `Material`,
  `Primitive`, `Plot3D`, `SceneCapture`, `Loader`. Qt/GL. Sits **above ICLQt**.

`PointCloud` splits cleanly: the **data type** → `cv3d`, the **scene wrapper**
(`PointCloudNode`) → scene module. geom2 already separates these two files, so it falls out.

Target hierarchy: `… ICLCV → cv3d (Qt-free) → ICLQt → viz3d → ICLMarkers → ICLPhysics`.
markers uses `cv3d` (Camera/pose); physics uses `viz3d` + `cv3d`.

Everything old `geom` offers is ported **natively** — the geom2 `SceneObjectConverter` /
`Primitive3DConverter` are migration scaffolding, deleted at the end. Then old `geom` is
deleted; today's geom2 is **renamed** to `viz3d`.

`geom` is **136 top-level files**, dispositioned:

| Bucket | Files | Destination |
|---|---:|---|
| **ALGORITHM / CV** (pose, ICP, point-cloud, segmentation, features, edge-detect, fitting) | **88** | port into **`cv3d`** — mostly CLEAN → near-verbatim; point-cloud clusters retarget the `cv3d::PointCloud` data type |
| **SHARED FOUNDATION** (`Camera`, `ViewRay`, `GeomDefs`, `PlaneEquation`) | 8 | **`cv3d`** (lowest layer; scene depends on it) |
| `Material` | 1 | **scene** (rendering type, used 37× by geom2) |
| **SCENE-GRAPH / RENDER** (duplicated by geom2) | **39** | delete once the scene module has native parity (§gaps) |

## Naming — `cv3d` + `viz3d` (symmetric 3D pair)

- **Algorithms → `cv3d`** (`icl::cv3d`): pairs with existing `ICLCV` (2D CV); "3D computer vision"
  is exactly these files. The namespace-legal form of the original "3D" idea.
- **Scene/render → `viz3d`** (`icl::viz3d`): "3D visualization" — scene graph + GL/Cycles render +
  Plot3D. Symmetric with `cv3d` (the two 3D modules read as a pair), no `viz::Scene`-style
  redundancy. Preferred over bare `viz`/`scene`/`render`/keeping `geom` (geometry ≈ the algorithms).

## Consumer reality (why deletion is nearly unblocked)

Outside `icl/geom/` and `icl/geom2/`, the retiring scene-graph headers have **6 consumers total**:

| Consumer | Headers | Kind |
|---|---|---|
| `markers/apps/camera-calibration.cpp` | `Scene.h` | app |
| `markers/apps/camera-calibration-CameraCalibrationUtils.{h,cpp}` | `Scene.h`, `SceneObject.h`, `GridSceneObject.h` | app |
| `markers/apps/camera-calibration-planar-GridIndicatorObject.h` | `SceneObject.h` | app |
| `markers/apps/camera-calibration-planar.cpp` | `Geom.h` umbrella (transitive) | app |
| `tests/test-io-scene-source.cpp` | `PointCloudObject.h` | test |

- **`icl/qt`** — zero scene-graph refs (only `GeomDefs.h` in one demo).
- **`icl/physics2`** — fully on **geom2** already; pulls only shared `Camera`/`Material` from geom.
- **`icl/cv`, `icl/io` (lib), `icl/markers` (lib)** — no scene-graph deps.
- The 5 `markers/apps/camera-calibration*` files **are the legacy version of Phase C**
  (extrinsic calibration). They are slated to be rewritten anyway — the de-dup and Phase C
  overlap here.

## The load-bearing entanglement

Point clouds in old geom **are** scene objects:

```
PointCloudObjectBase : public SceneObject        ← the single root of all friction
  └─ PointCloudObject : PCLPointCloudObject : PointCloudSegment
```

Through this one inheritance edge, the entire surviving CV point-cloud pipeline
(grabbers, IO, creation, normals, segmentation, superquadric fitting — ~40 files)
transitively depends on `SceneObject`. **Only 3 algorithm files include a scene-graph
header directly:**

1. `PointCloudObjectBase.h` → derives `SceneObject` (the root; drags clusters C–I).
2. `OctreeObject.h` → derives `SceneObject` (drags `RayCastOctreeObject`); superseded by
   geom2 `BVH`/`RayCastOctree`.
3. `Primitive3DFilter.h` → uses `SceneObject*` only in a `toSceneObject()` render helper
   (the filter logic is CV; geom2's `Primitive3DConverter` already replaces that helper).

Everything else in the 88 is either **CLEAN** (pose estimation, ICP, feature extraction,
edge detection, normals — no scene include) or **DATA-ENTANGLED only through
`PointCloudObjectBase`** — i.e. it decouples *automatically* once that base is split from
`SceneObject`.

**Break these three edges and essentially all entanglement is gone.**

## geom2 feature-parity gaps

geom2 is at or ahead of geom on core rendering (PBR, soft/PCF shadows, SSR, multi-light,
debug modes, headless CPU+GL capture, glTF/OBJ, Cycles photoreal, PlotWidget3D, animation
drivers). It ships two one-way bridges — `SceneObjectConverter` and `Primitive3DConverter` —
which are **migration scaffolding to be deleted at the end**, not the destination. Since the
end-state is native geom2 coverage of everything, **each gap below must be ported natively
into geom2 or explicitly dropped** (there is no "leave it behind in geom" option):

| geom capability geom2 lacks | Resolution |
|---|---|
| `PCLPointCloudObject` / PCL interop | **Recommend DROP** — only consumer is the `Geom.h` umbrella; PCL is conditionally built, no real user. Port natively only if a PCL path is actually wanted. |
| Full `Sky`/HDRI environment model (GL + Cycles) | Port natively (image-based lighting / reflections) — or drop if procedural sky suffices. |
| Light-gizmo node (`SceneLightObject`) + `setDrawLightsEnabled` | Port as a geom2 `Node` + Scene2 toggle. |
| Renderable `OctreeObject` node | Port as a drawable geom2 node (geom2 `BVH`/`RayCastOctree` cover only the *query* role). |
| `GridSceneObject` native node | Port as a geom2 `GridNode` (also needed by the calib apps). |
| `ComplexCoordinateFrameSceneObject` (labelled/thick axes) | Port as a labelled variant of `CoordinateFrameNode` (compose `TextNode`). |
| `Primitive` **polygon** + **texture** + **text** primitive fidelity | Native node coverage (converter currently skips texture/text) — becomes moot once nothing uses the converter. |
| Scene2 API completeness: `removeCamera`/`removeLight`, material presets, `getObject(recursiveIndices)`, bg-color getter/setter, Sky get/set | Add to Scene2 as the ports demand. |

**Net:** since we're going native-everywhere, the gaps *are* the parity backlog — sized by how
much of old geom's showcase we want to preserve. Deletion is only cheap where a capability is
dropped; anything kept must first exist natively in geom2.

## LANDED — Phases 0 + 1 (Session 98, suite 1070/1070, cv3d links zero Qt)

- **`icl/cv3d/` module created** (`icl::cv3d` namespace deferred — files keep `icl::geom`
  transitionally; the `geom::`→`cv3d::` rename is a dedicated later pass). Slotted **below ICLQt**
  in `meson.build` (unconditional, between `icl/cv` and `icl/qt`). `icl_cv3d_dep` is propagated by
  `icl_geom_dep`, so geom2/markers/physics2 pick it up transitively with no meson edits.
- **Phase 0 — foundation moved:** `Camera`, `ViewRay`, `PlaneEquation`, `GeomDefs` (+ `.cpp`).
  Verified `core/DataSegment.h`'s `GeomDefs` include was Doxygen-comment-only → no backwards edge.
- **Phase 1 — CLEAN CV clusters moved:** pose (`PoseEstimator`/`Posit`/`CoplanarPointPoseEstimator`/
  `RansacBasedPoseEstimator`/`PlanarRansacEstimator`), ICP (`ICP`/`ICP3D`/`IterativeClosestPoint`
  +CLCode), feature extractors (Surface/Curvature/CoPlanarity/CutfreeAdjacency/RemainingPoints),
  object edge detection (`ObjectEdgeDetector`+CPU/GPU/Data/Plugin), `PointCloudNormalEstimator`,
  `SegmenterUtils`, `RGBDMapping`. cv3d = **48 files**, geom = **88**.
- **`SoftPosit` DEFERRED** — the audit's "CLEAN = no scene-graph include" missed that it is
  Qt-coupled (a `qt::ICLDrawWidget* dw` member + Qt-taking `run()` overloads + dead `visualize()`).
  Left in geom (nothing depends on it, so no cv3d→geom backwards edge). Decouple its viz, then move.
- **cv3d OpenCL:** several kernels (ObjectEdgeDetectorGPU, ICP, normals, planar RANSAC) compile
  OpenCL paths under a macro guard → cv3d meson adds `opencl_dep` when found (mirrors old geom).
- **Verified:** full build (all modules + apps/demos/tests); `otool -L libicl-cv3d` shows
  filter/core/math/utils, **no Qt**; `icl-tests -j1` → 1070/1070.

## Staged plan (create cv3d → absorb → delete → rename scene module)

**Phase 0 — Stand up `cv3d` module + move foundation.** ✅ DONE (see LANDED above).

**Phase 1 — Move the CLEAN CV clusters into `cv3d`.** ✅ DONE (SoftPosit deferred — Qt-coupled).

**Phase 2 — point-cloud pipeline. 🟡 IN PROGRESS — reality is far smaller than "port 40 files".**
Investigation (S98) changed the picture:
- **`geom2::PointCloud` already exists as the clean Qt-free scene-free data model** — same
  `selectXYZ/Normal/RGBA32f/Label` `DataSegment` API as `PointCloudObjectBase`, PLUS it already
  natively reimplements creation (`unprojectDepth` = PointCloudCreator), color mapping, and box/
  sphere/depth filters. So geom2 already ported the *live* creation/filter parts.
- **The OLD geom point-cloud pipeline is almost entirely unconsumed externally** (per-header
  external-consumer counts): `PointCloudObjectBase`=0, `PointCloudObject`=1 (a test),
  `PointCloudCreator`=1 (a test), grabbers/outputs/serializer/`SQFitter`/`PointCloudSegment`/
  `FeatureGraphSegmenter`/`ConfigurableDepthImageSegmenter`/`PointCloudCreatorCL`=**0**. It's used
  only by geom's own scene graph (internal) + a couple of tests. `SQFitter` is superseded by the
  qt-playground CMA-ES fitter. So most of it is **dead → delete WITH the scene graph (Phase 6)**,
  not port. The 2–3 tests (`test-io-scene-source`, PointCloudCreator test) get ported to
  `geom2::PointCloud`.
- **Live borrowers from old geom** (must move to cv3d): the segmenters `Segmentation3D` +
  `EuclideanBlobSegmenter` (kinect-segmentation demo — they take `DataSegment<float,4>`, not
  `PointCloudObjectBase`), and `Primitive3DFilter` (point-cloud-primitive-filter app + the geom2
  `Primitive3DConverter`).

  **DONE (S98):** `Segmentation3D` + `EuclideanBlobSegmenter` → cv3d. Both were DataSegment-based;
  removed vestigial dead includes (`PointCloudObjectBase.h` in both, `qt/Quick2.h` in
  Segmentation3D — zero symbol uses), added an explicit `core/DataSegment.h`. Suite 1070/1070.

  **DONE (S98):** `Primitive3D` descriptor extracted → `cv3d/Primitive3D.h` (the live part:
  `Primitive3D` + nested `PrimitiveType`/`Quaternion`, kept nested to avoid colliding with the
  existing scene-graph `geom::PrimitiveType`). Dropped the dead `toSceneObject()` helper
  (`Primitive3DConverter` replaces it; nothing called it). The `PointCloudObjectBase`-coupled
  `Primitive3DFilter` filter machinery stays in geom to die in Phase 6 (now `using Primitive3D =
  cv3d's`). Retargeted the 3 consumers (`Primitive3DConverter`, the filter app, the converter test)
  onto `cv3d/Primitive3D.h`. Suite 1070/1070.

  **REMAINING:** (b) Decide `geom2::PointCloud`'s final relocation to cv3d
  (it's the data type cv3d "owns" but is used mainly by geom2 scene classes — not blocking; can move
  during the rename). (c) Port the 2–3 tests off `PointCloudObject`. (d) Everything else (data-model
  types, creators, grabbers, outputs, serializer, `SQFitter`, `FeatureGraphSegmenter`,
  `ConfigurableDepthImageSegmenter`, `PointCloudSegment`) → **delete with the scene graph in Phase 6**.
  Drop `PCLPointCloudObject` (see decisions).

**Functional sub-folders — ✅ DONE (S98).** cv3d: pose/ icp/ features/ edge/ segmentation/ (root=foundation). viz3d: nodes/ render/ scene/ plot/ pointcloud/ detail/ (root=Loader+Primitive3DConverter). Include paths + meson updated repo-wide; commits `061e214a7`, `7bebe0141`; suite 1074/1074.
functional sub-directories like the rest of ICL (`io/detail`, `math/tree|la|transform`, `utils/cl`,
`utils/detail/pugi`): e.g. cv3d → `pose/`, `icp/`, `segmentation/`, `features/`, `edge/`, `pointcloud/`;
viz3d → `nodes/`, `render/`, `plot/`, `detail/`. Do this after the moves + rename, not during.

**Transitional debt (whole cv3d):** files still declare `namespace icl::geom` and use the
`ICLGeom_API` export macro (a no-op on macOS/Linux; `__declspec` only on Windows). Both are fixed in
one dedicated pass — add `ICLCv3d_API` + rename `geom::`→`cv3d::` — after the moves settle.

**Phase 3 — remaining scene-entangled algorithm bits. ✅ DONE (S98).**
- `OctreeObject`/`RayCastOctreeObject` (SceneObject-derived renderable octree) were a **self-contained
  dead island** (zero consumers anywhere; geom2's `RayCastOctree` already covers the live query role
  and there is no consumer for a renderable octree node) → **deleted** (4 files). No native node built
  — nothing needs one.
- `Primitive3DFilter` was handled in Phase 2 (descriptor extracted; dead `toSceneObject()` dropped;
  the `PointCloudObjectBase`-coupled `apply()` machinery is dead → dies in Phase 6). Nothing to port.
The rest of the scene-entangled files are the dead point-cloud pipeline → deleted with the scene
graph in Phase 6.

**Phase 4 — Native parity backfill. ✅ DONE (S98).** Investigation narrowed it sharply — "build only
what a kept consumer needs, drop the unused":
- **Labelled coordinate frame: already done** — geom2 `CoordinateFrameNode` has a complex mode
  (cylinder bars + cone arrowheads + X/Y/Z `TextNode` labels). The scoping gap for
  `ComplexCoordinateFrameSceneObject` was stale.
- **`GridNode` BUILT** — `icl/geom2/GridNode.{h,cpp}`: a `MeshNode` subclass = nx*ny lattice drawn as
  grid lines / quad cells with mutable `getNode(x,y)` (faithful port of `geom::GridSceneObject`, the
  one gap with a kept consumer — the calib app). Renders via the generic `GeometryNode` path (no
  registration). Test `tests/test-geom2-grid-node.cpp` (4 cases). Suite 1074/1074.
- **DROP (no consumer → build nothing, dies in Phase 6):** `SceneLightObject` light gizmo +
  `setDrawLightsEnabled`, full `Sky`/HDRI env model, material presets, polygon/texture/text converter
  fidelity, `PCLPointCloudObject`/PCL interop. None has a live consumer; recording the drop shrinks
  Phase 6. (Renderable octree already dropped in Phase 3.)
- Scene2 API completeness (`removeCamera`/`removeLight`, `getObject(recursiveIndices)`, bg-color
  getter/setter) → add on demand when Phase 5/C actually needs them, not speculatively.

**Phase 5 — Port the last external scene-graph consumers. 🔶 SCOPED (S98) — the real gate.**
The 6 remaining consumers: `tests/test-io-scene-source.cpp` (trivial) + two apps (5 files):

- **`test-io-scene-source`** — one case (`reconstruct_cloud`) uses `geom::PointCloudCreator` +
  `geom::PointCloudObject`; the rest already use `geom2::PointCloud`. Swap to
  `geom2::PointCloud::unprojectDepth(depth, cam, /*distToCamPlane=*/true)` (semantics match). **~10-line
  change, independent, do anytime.**
- **App 2 — `camera-calibration-planar` (~755 L, MEDIUM)** — genuinely multi-camera against a *planar*
  target (marker-grid or checkerboard), intrinsics from udist files, **per-frame extrinsic pose only**
  (`MarkerGridPoseEstimator` / `CoplanarPointPoseEstimator`) — i.e. it *already* uses the good decoupled
  path. Scene use: `GridIndicatorObject` (custom SceneObject w/ child boxes + text labels),
  `ComplexCoordinateFrameSceneObject` (per-view + world), camera-frustum viz, `SceneMouseHandler`,
  multi-view GL-callback dispatch, a live variance `Plot`.
- **App 1 — `camera-calibration` (~1780 L, HIGH)** — single camera vs 3D objects via the **drift-prone
  joint DLT** (`calibrate_pinv`) — *exactly what the redesign's Phase C replaces*. Port cost is
  dominated by NON-scene logic (manual-grid mouse editing, best-of-N threaded saver, config format w/
  embedded `.obj`); scene-graph is only ~13-15%.

**All geom2 building blocks now exist:** Scene→Scene2 ✅ (multi-cam, getGLCallback/getMouseHandler),
SceneObject-mesh→`MeshNode::load` ✅, GridSceneObject→`GridNode` ✅ (P4), ComplexCoordFrame→
`CoordinateFrameNode` complex mode ✅, GridIndicatorObject→`GroupNode`+`CuboidNode`+`TextNode` ✅,
camera-frustum→Scene2 "show cameras" ✅, calibration solvers→`cv3d/Camera` ✅. And
**`icl-cam-calib-intrinsic` is already a working geom2 calib app** (Scene2+Canvas3D+OffscreenView) — the
port template.

**DECISION (S98, user): MECHANICAL PORT NOW.** Migrate both apps' scene usage onto geom2/cv3d/viz3d
(Scene→Scene2, GridSceneObject→GridNode, ComplexCoordFrame→CoordinateFrameNode, GridIndicatorObject→
GroupNode+CuboidNode+TextNode, GL-callback wiring) — keep the calibration ALGORITHM logic unchanged (the
quality rethink stays deferred to the redesign arc). Re-link once they build+run. Old geom stays until
Phase 6, so the apps can be ported incrementally against geom2 while old geom still exists.

- `test-io-scene-source` — ✅ ported (`reconstruct_cloud` → `geom2::PointCloud::unprojectDepth`).
- **App 1 `camera-calibration`** — ✅ ported (commit `2a63e9a1a`): Scene→Scene2, obj→NodePtr (MeshNode::load
  in a GroupNode; material via a child-mesh helper), helper plane→GridNode, coord-frame toggle→a
  CoordinateFrameNode, per-primitive visibility dropped (material alpha covers it). Offscreen `-is list`
  path runs the geom2 scene setup end-to-end.
- **App 2 `camera-calibration-planar`** — ✅ ported (commit `d678d477f`): Scene→Scene2,
  ComplexCoordinateFrameSceneObject→CoordinateFrameNode, `GridIndicatorObject` rebuilt as a GroupNode
  (per-cell MarkerObj = box MeshNode + TextNode label; checkerboard = line-grid MeshNode),
  `prop("visualize cameras")`→`prop("show cameras")`, multi-view GLCallback unchanged.
- **Also fixed** a latent Phase-1 leftover: `cv3d/IterativeClosestPoint.h` carried an unused
  `#include <icl/geom/Geom.h>` (backwards cv3d→geom edge) — removed (commit `dca676a7d`).

**Phase 5 ✅ DONE — audit confirms NO external consumer of old geom's scene graph remains.** Suite
1074/1074. Only geom-internal refs + the geom2 converters (scaffolding) are left → **Phase 6 fully
unblocked**: delete old geom's scene graph (39 files) + the dead point-cloud pipeline + the geom2
converters.

**Phase 6 — Delete old geom's scene graph + dead pipeline. ✅ DONE (S98, commit `f09c3c298`).**
78 files / ~22k lines deleted, no external consumer left. geom dropped 80 → **4-file rump**
(`Material` + `SoftPosit`). Also deleted geom2's `SceneObjectConverter` (last geom2→geom-scene-graph
link; `Primitive3DConverter` kept). Relocated the Cycles build config from geom's meson into geom2's.
Full build (incl. Cycles targets) + suite 1074/1074; residual sweep clean.

**Phase 7 — Rename + dissolve. ✅ DONE (S98). END STATE REACHED: `cv3d` + `viz3d`, no `geom`.**
- Stage A (`6efa10828`): geom2 → viz3d (dir + `icl::geom2`→`icl::viz3d` + `geom2::`→`viz3d::` +
  `ICLGeom2_API`→`ICLViz3d_API` + includes + meson + test/demo file renames).
- Stage B1 (`6be5266d0`): `Material` → viz3d (namespace + `geom::Material`→`viz3d::Material` + fixed
  stale forward-decls).
- Stage B (`837cb55c8`): `SoftPosit` → cv3d (stripped its dead `#ifdef ICL_HAVE_QT` viz → Qt-free);
  geom apps/demos → cv3d, scenes → viz3d, doc → cv3d, proto deleted; **deleted icl/geom entirely**;
  repointed markers/math/viz3d to `icl_cv3d_dep` (markers regained `icl_qt_dep`).
- Stage B5 (`bf1d949fc`): `icl::geom`→`icl::cv3d` namespace rename (199 files).
Full build + suite 1074/1074 throughout.

**Remaining cosmetic follow-ups (non-blocking):** `ICLGeom_API`→`ICLCv3d_API`/`ICLViz3d_API` (retained
as a globally-defined empty alias for now); `Scene2`/`DemoScene2` class names still carry the `2`;
functional sub-folders in cv3d/viz3d (the post-split TODO above).

## Open decisions (need the user)

1. **PCL — DROP (decided).** `PCLPointCloudObject` has no real consumer (only the `Geom.h`
   umbrella) and PCL is optional. Drop it in Phase 2. **Leave a documented placeholder** in
   `cv3d` for a future lightweight PCL-compat layer (thin `cv3d::PointCloud` ⇄ `pcl::PointCloud`
   conversion structs) — an idea for later, not built now.
2. **Split into `cv3d` + scene module — recommended (confirm).** Verified viable: CV algorithms
   are Qt-free once `PointCloudObjectBase` is severed. Buys headless 3D vision. Alternative is one
   combined module; the split is near-free given the disentangling is required anyway.
3. **Naming — `cv3d` + `viz3d` (confirm).** Algorithms → `cv3d` (`icl::cv3d`); scene/render →
   `viz3d` (`icl::viz3d`). Symmetric 3D pair.
4. **How much rendering showcase to preserve (Phase 4 scope)?** Sky/HDRI, light gizmos, labelled
   coord-frames, renderable octree, material presets — none has a live consumer. Recommend **drop
   the unused, port only what a kept app/demo needs**.
5. **Rename timing** — the scene-module rename is the **final** phase (7), after old geom is gone,
   so names never collide.
