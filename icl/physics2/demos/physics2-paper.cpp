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

  // dispatching mouse handler (resolves fold/mover from the node by type)
  mouse = std::make_unique<PaperMouseHandler>(0, &scene.scene(), paper);
  mouse->setSensitivities(10.f, 3.0f);   // snappier camera rotation (default rotation=1)

  gui << (VBox()
          << (HSplit()
              << ui::Canvas3D(Size(2400,1800), {.handle="draw", .minSize={32,24}})
              << (VBox().maxSize(16,99)
                  << ui::Prop(paper, {.label="paper"})
                  << ui::Prop(fold,  {.label="fold"})
                  << ui::Prop(mover, {.label="move"})
                  << ui::Button("reset", {.handle="reset"})
                  << ui::CheckBox("collision debug", {.handle="dbg"})
                  << ui::Display({.handle="foldmap", .minSize={16,12}})))
          << ui::StatusBar())
      << ui::Show();
  // Modifier help in the status bar (handle "status" is built into StatusBar).
  gui["status"] = std::string(
      "Paper:  Ctrl+drag = fold   |   Shift+drag = grab point   |   "
      "Shift+Ctrl+drag = move sheet   |   plain/right drag = orbit, wheel = zoom   "
      "(lower 'bend range' to see creases hinge)");
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(mouse.get());
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

  // rebuild the overlay: existing creases (yellow) + the live drag preview (cyan).
  // Gather first (getCreaseSegments locks the physics world), then mutate the node
  // under the SCENE lock — Scene2::render() holds it, so mutating a scene node off
  // the lock would race the GL thread (a SIGSEGV).
  const auto segs = paper->getCreaseSegments();
  Vec pa, pb;
  const bool hasPreview = fold->getPreview(pa, pb);
  {
    std::scoped_lock<Scene2> lk(scene.scene());
    overlay->clearGeometry();
    // MeshNode colors are 0..255 (addVertex/addLine scale by 1/255 internally).
    const GeomColor crease(255, 210, 0, 255), preview(0, 220, 255, 255);
    int li = 0;
    for (const auto &s : segs) {
      overlay->addVertex(s.first, crease); overlay->addVertex(s.second, crease);
      overlay->addLine(li, li + 1, crease); li += 2;
    }
    if (hasPreview) {
      overlay->addVertex(pa, preview); overlay->addVertex(pb, preview);
      overlay->addLine(li, li + 1, preview); li += 2;
    }
  }

  gui["draw"].render();
  gui["foldmap"] = paper->getFoldMap();   // creases show up as dark lines
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
