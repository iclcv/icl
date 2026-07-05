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

  **REMAINING:** (a) `Primitive3DFilter` → cv3d (filter logic; drop `toSceneObject()` helper, which
  `Primitive3DConverter` already replaces; check its `PointCloudObject` use → retarget to
  `geom2::PointCloud` or `DataSegment`). (b) Decide `geom2::PointCloud`'s final relocation to cv3d
  (it's the data type cv3d "owns" but is used mainly by geom2 scene classes — not blocking; can move
  during the rename). (c) Port the 2–3 tests off `PointCloudObject`. (d) Everything else (data-model
  types, creators, grabbers, outputs, serializer, `SQFitter`, `FeatureGraphSegmenter`,
  `ConfigurableDepthImageSegmenter`, `PointCloudSegment`) → **delete with the scene graph in Phase 6**.
  Drop `PCLPointCloudObject` (see decisions).

**Transitional debt (whole cv3d):** files still declare `namespace icl::geom` and use the
`ICLGeom_API` export macro (a no-op on macOS/Linux; `__declspec` only on Windows). Both are fixed in
one dedicated pass — add `ICLCv3d_API` + rename `geom::`→`cv3d::` — after the moves settle.

**Phase 3 — Port the remaining scene-entangled algorithm bits.** `OctreeObject`/
`RayCastOctreeObject` → native drawable node in the scene module over `BVH` (or drop, keeping
only the query role geom2 has). `Primitive3DFilter` filter logic → `cv3d`; its `toSceneObject()`
helper → native node construction in the scene module (retire `Primitive3DConverter`).

**Phase 4 — Native parity backfill in the scene module for kept features** (§gaps): `GridNode`,
labelled coord-frame, light gizmo + `setDrawLightsEnabled`, Sky/HDRI (or drop), polygon/texture/
text primitive fidelity, Scene2 API completeness. Scope = how much of geom's showcase we keep;
this is what lets `SceneObjectConverter` be deleted.

**Phase 5 — Port the last external scene-graph consumers.** Rewrite the 5
`markers/apps/camera-calibration*` files + `tests/test-io-scene-source.cpp` onto the native scene
module. Best folded into the **Phase C extrinsic-calibration app** (same problem, fresh code).

**Phase 6 — Delete old `geom` entirely** (all 136 files) + the geom2 converters + build/demo
wiring. Suite green throughout.

**Phase 7 — Rename the scene module** (today's geom2 → `viz3d`): dir
`icl/geom2`→`icl/<name>`, namespace `icl::geom2`→`icl::<name>` (68 files), includes + meson
targets repo-wide. End state: **`cv3d`** (Qt-free 3D CV) + **`viz3d`** (3D scene/render).

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
