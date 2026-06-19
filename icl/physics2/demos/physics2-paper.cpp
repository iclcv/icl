// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: fold-aware paper manipulation (the PhysicsPaper3 transplant)
// as *composable drivers*. One MeshNode carries three drivers:
//   - PaperDriver       (substrate: the soft-body paper + fold primitives)
//   - FoldDriver        (behaviour: creases the paper)
//   - PaperMoverDriver  (behaviour: soft point-drag + whole-sheet move)
// A PaperMouseHandler dispatches a *modified* LEFT-drag to the right behaviour:
//   Ctrl = crease,  Shift = pull/hold a point,  Shift+Ctrl = carry the sheet.
//   Plain drag / right-drag / wheel always orbit/zoom. Each driver shows its own
//   Prop panel; a status bar at the bottom shows the modifier help.
// Physics runs on its own thread; the run loop only syncs the mesh + renders.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/Node.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/PaperDriver.h>
#include <icl/physics2/FoldDriver.h>
#include <icl/physics2/PaperMoverDriver.h>
#include <icl/physics2/PaperMouseHandler.h>
#include <icl/geom2/Scene2MouseHandler.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene(SoftBodyMode::SoftRigid);   // paper needs the legacy solver
PaperDriver *paper = nullptr;
FoldDriver *fold = nullptr;
PaperMoverDriver *mover = nullptr;
std::shared_ptr<MeshNode> overlay;     // crease highlight + live fold-line preview
std::unique_ptr<PaperMouseHandler> mouse;
Time lastTick;

void init() {
  scene.setupDefault(DefaultScene::SceneType::Studio, 1200.f);   // ground top ~ z=-624

  // start the camera twice as close to the paper (halve its distance to the
  // look-at target), keeping the viewing direction
  {
    auto &cam = scene.scene().getCamera(0);
    const Vec tgt = scene.scene().getCursor();
    cam.setPosition(cam.getPosition() + (tgt - cam.getPosition()) * 0.5f);
  }

  // a wide, thin static slab the sheet rests / drapes on (5x wider+longer, 1/10
  // the height of the original 300^3 cube -> 1500 x 1500 x 30)
  auto box = CuboidNode::create(0,0,0, 1500,1500,30);
  box->setMaterial(Material::fromColor(GeomColor(180,140,70,255)));
  box->translate(0,0,-474);
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);

  // an A3-ish flat sheet, dropped low so it settles instead of tunneling
  const Vec corners[4] = { Vec(-420,-300,-260,1), Vec(420,-300,-260,1),
                           Vec(-420, 300,-260,1), Vec(420, 300,-260,1) };
  paper = scene.addPaper(Size(20,20), corners, /*selfCollision*/ false);
  dynamic_cast<MeshNode*>(paper->node())
      ->setMaterial(Material::fromColor(GeomColor(235,232,225,255)));   // matte paper white

  // stack the two behaviour drivers onto the SAME node as the substrate
  fold  = paper->node()->addDriver<FoldDriver>().get();
  mover = paper->node()->addDriver<PaperMoverDriver>().get();

  // overlay for the crease highlight + live fold-line preview (drawn over the paper)
  overlay = std::make_shared<MeshNode>();
  overlay->setRenderOnTop(true);
  overlay->setLineWidth(3.f);
  scene.addNode(std::static_pointer_cast<Node>(overlay));

  scene.start(240);   // small fixed timestep -> less soft-body tunneling

  // paper-interaction handler (resolves fold/mover from the node by type);
  // installed first in the chain, the camera handler is installed after it.
  mouse = std::make_unique<PaperMouseHandler>(0, &scene.scene(), paper);
  scene.scene().getMouseHandler(0)->setSensitivities(10.f, 3.0f);   // snappier camera rotation

  gui << (VBox()
          << (HSplit()
              << Canvas3D(Size(2400,1800), {.handle="draw", .minSize={32,24}})
              << (VBox().maxSize(16,99)
                  // the three driver panels share one tab so they don't stack and
                  // crowd out the paper view below (each Prop is itself scrollable)
                  << (Tab("paper,fold,move", {.minSize={16,10}})
                      << Prop(paper)
                      << Prop(fold)
                      << Prop(mover))
                  << Button("reset", {.handle="reset"})
                  << CheckBox("collision debug", {.handle="dbg"})
                  << (HBox().label("show")
                      << CheckBox("faces",  {.handle="vFaces"})
                      << CheckBox("creases",{.checked=true, .handle="vCreases"})
                      << CheckBox("1st",    {.handle="v1st"})
                      << CheckBox("2nd",    {.handle="v2nd"}))
                  << Canvas(Size(210, 297), {.handle="paperview", .minSize={12,16}})))
          << StatusBar())
      << Show();
  // Modifier help in the status bar (handle "status" is built into StatusBar).
  gui["status"] = std::string(
      "Paper:  Ctrl+drag = fold   |   Shift+drag = grab point   |   "
      "Shift+Ctrl+drag = move sheet   |   plain/right drag = orbit, wheel = zoom   "
      "(lower 'bend range' to see creases hinge)");
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(mouse.get());                      // paper: highest priority
  gui["draw"].install(scene.scene().getMouseHandler(0)); // camera nav: installed last
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  static ButtonHandle reset = gui["reset"];
  if (reset.wasTriggered()) paper->reset();

  scene.setDebugDrawEnabled(gui["dbg"]);
  scene.sync(dt);

  // rebuild the overlay from the toggled debug-geometry categories + the live
  // drag preview. Gather first (getDebugGeometry locks the physics world), then
  // mutate the node under the SCENE lock — Scene2::render() holds it, so mutating
  // a scene node off the lock would race the GL thread (a SIGSEGV).
  const auto dbg = paper->getDebugGeometry();
  Vec pa, pb;
  const bool hasPreview = fold->getPreview(pa, pb);
  const bool vFaces = gui["vFaces"], vCreases = gui["vCreases"],
             v1st = gui["v1st"], v2nd = gui["v2nd"];
  {
    std::scoped_lock<Scene2> lk(scene.scene());
    overlay->clearGeometry();
    int li = 0;
    // MeshNode colors are 0..255 (addVertex/addLine scale by 1/255 internally).
    auto addSegs = [&](const std::vector<std::pair<Vec, Vec>> &segs, const GeomColor &c) {
      for (const auto &s : segs) {
        overlay->addVertex(s.first, c); overlay->addVertex(s.second, c);
        overlay->addLine(li, li + 1, c); li += 2;
      }
    };
    if (vFaces)   addSegs(dbg.faces,       GeomColor(150, 150, 150, 255));   // gray wireframe
    if (v2nd)     addSegs(dbg.secondOrder, GeomColor(255, 140,   0, 255));   // orange bending
    if (v1st)     addSegs(dbg.firstOrder,  GeomColor( 60, 200,  80, 255));   // green structural
    if (vCreases) addSegs(dbg.creases,     GeomColor(255, 210,   0, 255));   // yellow creases
    if (hasPreview) {
      const GeomColor preview(0, 220, 255, 255);
      overlay->addVertex(pa, preview); overlay->addVertex(pb, preview);
      overlay->addLine(li, li + 1, preview); li += 2;
    }
  }

  gui["draw"].render();

  // 2D "pseudo paper" view: the flat unit sheet + the crease primitives in paper
  // space (independent of the deformed 3D mesh). Viewport is the paper's intrinsic
  // A4-portrait aspect (paper-x = short edge, paper-y = long edge — PAPER_W:PAPER_H).
  {
    const float VW = 210.f, VH = 297.f;
    DrawHandle pv = gui["paperview"];
    pv->abs();
    pv->color(0, 0, 0, 0);   pv->fill(245, 245, 235, 255);
    pv->rect(0, 0, VW, VH);                              // the sheet
    pv->color(120, 120, 120, 255); pv->fill(0, 0, 0, 0);
    pv->rect(0, 0, VW, VH);                              // border
    pv->color(255, 210, 0, 255); pv->linewidth(2);
    for (const auto &c : paper->getCreases())
      pv->line(c.a.x * VW, c.a.y * VH, c.b.x * VW, c.b.y * VH);
    pv->render();
  }
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
