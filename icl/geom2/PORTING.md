# Porting from ICLGeom to geom2

## Namespace

```cpp
// Old
using namespace icl::geom;
// New
using namespace icl::geom2;
```

## Node Types

| geom | geom2 | Notes |
|------|-------|-------|
| `SceneObject` | `MeshNode` | Freeform geometry (mutable vertices) |
| `SceneObject` (as group) | `GroupNode` | Pure children container, no geometry |
| `SceneObject::sphere(...)` | `SphereNode::create(...)` | Returns `shared_ptr<SphereNode>` |
| `SceneObject::cuboid(...)` | `CuboidNode::create(...)` | Returns `shared_ptr<CuboidNode>` |
| `SceneObject::cylinder(...)` | `CylinderNode::create(...)` | |
| `SceneObject::cone(...)` | `ConeNode::create(...)` | |
| `SceneObject("sphere", params)` | `SphereNode(cx,cy,cz,r)` | No string-based construction |
| `CoordinateFrameSceneObject` | `CoordinateFrameNode` | GroupNode with 3 CuboidNode axes |
| `SceneObject(filename)` | `MeshNode::load(filename)` | Returns `vector<shared_ptr<MeshNode>>` |
| `SceneLight` + `SceneLightObject` | `LightNode` | Single class, position from transform |
| `PointCloudObjectBase` | `PointCloudNode` | TODO |

## Key Differences

### No more hybrid objects

In geom, a SceneObject could have both geometry AND children. In geom2:

- **GroupNode** has children, no geometry
- **MeshNode** has geometry, no children
- **Parametric nodes** (SphereNode etc.) have geometry, no children, no mutable vertex access

```cpp
// Old: object with both geometry and children
auto *obj = new SceneObject();
obj->addVertex(...);
obj->addChild(new SceneObject("sphere", ...));

// New: GroupNode with MeshNode + SphereNode children
auto group = std::make_shared<GroupNode>();
auto mesh = std::make_shared<MeshNode>();
mesh->addVertex(...);
group->addChild(mesh);
group->addChild(SphereNode::create(...));
```

### No raw pointer ownership

```cpp
// Old
scene.addObject(new SceneObject("cuboid", data), true);  // passOwnership=true

// New
scene.addNode(CuboidNode::create(x, y, z, dx, dy, dz));  // shared_ptr
```

### Parametric shapes are immutable leaves

```cpp
// Old: sphere vertices are mutable, can drift from params
auto *s = SceneObject::sphere(0, 0, 0, 50, 30, 30);
s->getVertices()[0] = ...; // breaks sphere params!

// New: SphereNode has no mutable vertex access
auto s = SphereNode::create(0, 0, 0, 50);
s->setRadius(75);       // regenerates mesh automatically
s->retessellate(60, 60); // change resolution
// s->getVertices() — compile error! (only const access via GeometryNode)
```

### Bulk geometry loading with ingest()

```cpp
// Old: per-vertex calls
for (auto &p : positions) obj->addVertex(p);
for (auto &n : normals) obj->addNormal(n);
for (...) obj->addTriangle(...);

// New: zero-copy bulk move
mesh->ingest({
    .vertices  = std::move(positions),
    .normals   = std::move(normals),
    .triangles = std::move(tris),
});
```

### Material (unchanged)

Material is shared between geom and geom2 — same `Material::fromColor()`,
`Material::fromColors()`, `Material::fromPhong()`, same sub-structs
(TextureMaps, TransmissionParams).

```cpp
auto mat = Material::fromColor(GeomColor(220, 60, 60, 255));
mat->roughness = 0.3f;
node->setMaterial(mat);
```

### Lights are scene graph nodes

```cpp
// Old: separate light system
scene.getLight(0).setPosition(Vec(200, 300, 100, 1));
scene.getLight(0).setDiffuse(GeomColor(255, 255, 240, 255));

// New: LightNode in scene graph, position from transform
auto light = std::make_shared<LightNode>(LightNode::Point);
light->setColor(GeomColor(1.0f, 0.97f, 0.92f, 1.0f));
light->translate(200, 300, 100);
scene.addLight(light);
```

### Scene setup

```cpp
// Old
Scene scene;
scene.addCamera(cam);
scene.addObject(obj);
gui["draw"].link(scene.getGLCallback(0));

// New
Scene2 scene;
scene.addCamera(cam);
scene.addNode(node);
gui["canvas"].link(scene.getGLCallback(0).get());
```

### Copy/Move semantics

All geom2 nodes have proper Rule of 5 via PIMPL (unique_ptr<Data>):

```cpp
SphereNode copy = *original;            // deep copy
SphereNode moved = std::move(*original); // O(1) move
auto s = scene.addNode(SphereNode(0,0,0,50)); // move into scene
```

## Common Migration Patterns

### Planet/compound object

```cpp
// Old
struct Planet : public SceneObject {
    Planet(float r) { addSphere(0,0,0,r,30,30); }
};

// New
struct Planet : public GroupNode {
    Planet(float r) { addChild(SphereNode::create(0,0,0,r)); }
};
```

### Coordinate frame

```cpp
// Old
auto *frame = new CoordinateFrameSceneObject(100, 5);

// New
auto frame = CoordinateFrameNode::create(100, 5);
frame->setParams(200, 3);  // dynamic resize
```

### Physics/dynamic mesh

```cpp
// Old
auto *paper = new SoftObject(...);
paper->getVertices()[i] = newPos;  // direct vertex mutation

// New: MeshNode provides mutable access
auto paper = std::make_shared<MeshNode>();
paper->getVertices()[i] = newPos;  // same — MeshNode exposes non-const
```

### File loading

```cpp
// Old
scene.addObject(new SceneObject("model.obj"));
// or
auto objs = scene.loadFiles({"model.glb"});

// New
for (auto &m : MeshNode::load("model.glb"))
    scene.addNode(m);
```

## What's NOT yet ported

- PointCloudNode (replaces PointCloudObjectBase)
- Mouse interaction / picking (hit testing)
- Cycles renderer compilation (headers ready, build wiring TODO)
- Text primitives / billboard text
- Texture primitives
- Display list equivalent (not needed — GL Core renderer uses geometry cache)
