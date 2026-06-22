# Retiring legacy `geom::Scene` — per-app worklist

Goal: free the legacy `geom` **rendering/scene layer** (`Scene`, `SceneObject`+subclasses,
`GLRenderer`, `SceneLight`, `SceneMouseHandler`, `DemoScene`, `Sky`, `PlotWidget3D`, …) so it
can be deleted (geom2-migration Phase 4). The geom **CV/math core** stays (`Camera`, ICP/Posit,
point-cloud algorithms, `ObjectEdgeDetector`, `Primitive3DFilter`, `GeomDefs`).

Status: physics side already done (legacy `icl/physics` deleted). This is the rendering side.
**Done so far (Session 78):** pruned 6 dead/superseded demos — `camera`, `swiss-ranger`,
`gl-renderer-test`, `cycles-{overlay-viewer,renderer-test,scene-viewer}` (the 3 Cycles ones
superseded by geom2's `geom2-cycles-*`). ~30 files remain.

Reference: `icl/geom2/geom-to-geom2-porting-guide.md`; converter `geom2::fromSceneObject`
(`SceneObjectConverter.h`) for OBJ/static SceneObjects.

---

## Cross-cutting prerequisites (build these → they unblock several apps)

| # | Capability | Status | Unblocks |
|---|---|---|---|
| P1 | **Offscreen GL render → `core::Img`** (geom2 Phase 0c) — geom2 only renders offscreen via Cycles, not GL. **No GL in this sandbox → build blind, verify on a real display.** | MISSING | offscreen-rendering, rotate-image-3D, depth-camera-simulator |
| P2 | **`PlotWidget3D` on geom2** (Plot3D/PlotHandle3D are Qt widgets on `geom::Scene`) | MISSING | plot-widget-3D |
| P3 | **`Primitive3D` → geom2 node** converter (~50 LOC; Cuboid/Sphere/Cylinder nodes exist) | small | point-cloud-primitive-filter |
| P4 | **Validate** `Scene2` BVH/octree pick invalidation on cloud mutation + `CoordinateFrameNode` parity | check | point-cloud-viewer, -define-world-frame, markers |

NOT a gap: **textures work natively** (geom2 `Renderer` samples `baseColorMap`; the maze ball +
`DefaultScene` ground prove it). Only `fromSceneObject` skips textures — build textured
`Material`s directly. (Custom GLSL fragment shaders, however, have no geom2 hook — see
animated-grid.)

---

## Per-app decisions

### Plain scene viewers (geom/demos) — all PORT, textures native
| File | Action | Effort | Notes |
|---|---|---|---|
| `scene-object` | PORT | S | parametric nodes + `MeshNode::load()` for OBJ |
| `simplex-3D` | PORT | S | `CoordinateFrameNode` + dynamic `MeshNode` |
| `scene-graph` | PORT | M | custom SceneObjects→GroupNodes; light-anchor via transform hierarchy |
| `scene-shadows` | PORT | M | `LightNode` shadows; manual shadow-cam control not exposed (OK) |
| `texture-cube` | PORT | M | native textured `Material` (6-face) + light nodes |
| `generic-texture-coords` | PORT | M | native texcoords (works now) |
| `animated-grid` | REWORK | M–L | uses a **custom GLSL fragment shader** — no geom2 hook; vertex-color/animated-mesh fallback, or add a shader hook |
| `superquadric` | REWORK/FUSE | M | no `SuperquadricNode` — add one, or FUSE into a generic parametric-shape demo |

### Point-cloud apps (geom/apps) — geom2 `PointCloudNode` has strong parity → PORT
| File | Action | Effort | Notes |
|---|---|---|---|
| `simple-point-cloud-viewer` | PORT | S | 1:1; `PointCloudNode::setPointSize` |
| `point-cloud-pipe` | PORT | S | 1:1 + `GenericPointCloudOutput` (renderer-agnostic) |
| `point-cloud-creator` | PORT | S | thin adapter: `PointCloudCreator` output → `geom2::PointCloud` |
| `point-cloud-viewer` | PORT | M | BVH pick (validate P4) + `CoordinateFrameNode` |
| `point-cloud-define-world-frame` | PORT | M | BVH pick + depth-cam binding; CV math untouched |
| `point-cloud-primitive-filter` | PORT | M | needs P3 (`Primitive3D`→node); filter logic untouched |
| **FUSE**: simple-viewer + pipe + viewer → one unified geom2 cloud viewer (flags for dual-cloud/pick/output). |

### Depth / Kinect (geom/demos + apps) — backends alive (kinect/openni); segmentation = CV core, stays
| File | Action | Effort | Notes |
|---|---|---|---|
| `rgbd-mapping` | PORT | S | cleanest; `DepthCameraPointCloudGrabber` backend-agnostic → best first |
| `depth-camera-simulator` | REWORK | S→ | trivial scene, but renders **depth to an Img** → needs P1 (offscreen) |
| `kinect-pointcloud` | PORT | M | `PointCloudCreator` adapter; normals are CV |
| `kinect-segmentation` | PORT | L | `Segmentation3D` stays in geom (CV); geom2 = thin viz layer |
| `kinect-euclidean-blob-segmentation` | FUSE→ | L | subset of kinect-segmentation → consolidate then port |
| `kinect-depth-image-segmentation` | PORT | M | `ConfigurableDepthImageSegmenter` stays CV; viz→geom2 |

### Misc (geom/demos + apps)
| File | Action | Effort | Notes |
|---|---|---|---|
| `ray-cast-octree` | **DELETE** | S | already replaced by `geom2/demos/raycast-octree.cpp` |
| `show-scene` | PORT | S | multi-cam OBJ viewer; `fromSceneObject` + camera frames |
| `surf-based-object-tracking` | PORT | S | SURF/pose untouched; `SceneObject`→`CuboidNode` |
| `offscreen-rendering` | REWORK/DELETE | L | needs P1 (GL offscreen); DELETE if P1 not pursued |
| `rotate-image-3D` | REWORK | M | image-on-quad trivial; offscreen output needs P1 |
| `plot-widget-3D` | REWORK/DELETE | L | needs P2 (geom2 PlotWidget3D), ~500–1000 LOC |

### Markers (markers/apps + demos) — `fromSceneObject` + `CoordinateFrameNode` help
| File | Action | Effort | Notes |
|---|---|---|---|
| `marker-detection` | PORT | S | inline `Obj`→`GroupNode`(cuboid+frame); best markers first |
| `multi-cam-marker-demo` | PORT/FUSE | S–M | per-camera `getGLCallback(i)`; FUSE w/ marker-detection? |
| `camera-calibration-planar` (+GridIndicatorObject) | PORT | M–L | `ComplexCoordinateFrameSceneObject`→`CoordinateFrameNode`; **GridIndicatorObject** (~90 LOC, nested cells + ID text)→GroupNode/MeshNode; multi-cam |
| `camera-calibration` (+Utils) | PORT | M | OBJ+grid via `fromSceneObject`; custom `AdjustGridMouseHandler`→`Scene2MouseHandler`+hit-test; **2D overlay** needs a separate Canvas2D |

---

## Suggested order (low-risk first, prerequisites as they're hit)
1. **Easy PORTs that prove patterns:** `marker-detection`, `scene-object`, `simplex-3D`,
   `simple-point-cloud-viewer`, `rgbd-mapping`, `surf-based-object-tracking`, `show-scene`.
2. **DELETE** `ray-cast-octree`.
3. **Point-cloud batch** (+ P3, P4): pipe, creator, viewer, define-world-frame, primitive-filter
   (FUSE the viewers). Then the **kinect batch** (FUSE the two segmenters).
4. **Texture demos:** texture-cube, generic-texture-coords (native); decide animated-grid
   (shader) + superquadric (new node) — REWORK or simplify.
5. **Markers calibration** (planar, then full) — the heaviest.
6. **Prerequisite-gated:** P1 offscreen (offscreen-rendering, rotate-image-3D,
   depth-camera-simulator), P2 plot (plot-widget-3D). Build the capability or DELETE the demo.
7. **Phase 4:** once nothing references the scene layer, delete `Scene`/`SceneObject`/
   `GLRenderer`/`SceneLight`/`SceneMouseHandler`/`DemoScene`/`Sky`/`PlotWidget3D`…; keep the CV core.

**Verification limit:** no GL/hardware here — ports are build- + headless-init-checked only;
visual/hardware correctness needs a real display (same constraint as the physics work).
