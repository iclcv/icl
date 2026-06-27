// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive checkerboard-detection lab (Phase B tuning tool). LEFT: a virtual
// checkerboard on a bordered "paper" in a geom2 Scene2 — rotate/pan/zoom it with
// the mouse to view from any angle. RIGHT: the *real* render of camera 0 of that
// scene (full GL shading / lighting), passed through a live lens-distortion
// model, then through cv::CheckerboardSaddleDetector — the detected corners +
// orientations are drawn on top. Cells, distortion, lighting and detector
// parameters are all live sliders.
//
// The right pane is the offscreen render of camera 0 of a dedicated capture
// scene (capScene, own lighting, tracks the interactive camera), passed through
// the distortion model + detector. The render is switchable between GL (fast)
// and Cycles (photoreal). All the tricky threading — GL captured on the GUI
// thread (widget context), Cycles polled on the worker thread — is handled by
// geom2::OffscreenView; we just requestCapture()/poll() it from run(). (See
// OffscreenView.h for why that split is necessary on macOS.)
//
// This is the interactive sibling of the synthetic calibration harness: it lets
// us watch and tune the native detector under arbitrary viewpoint + distortion.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/OffscreenView.h>   // interactive view + switchable GL/Cycles capture
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <cstdlib>   // std::_Exit
#include <iostream>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::core;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;                               // on-screen interactive scene (left pane)
Scene2 capScene;                            // capture scene (right pane source, own lighting)
OffscreenView view(scene, 0);               // interactive view + GL/Cycles offscreen capture
std::shared_ptr<MeshNode> board;            // textured quad shown in the 3D scene
std::shared_ptr<MeshNode> capBoard;         // its mirror in the capture scene
Img8u tex;                                  // current checkerboard texture (RGB)
int curX = 0, curY = 0;                     // cells the texture was built for
const Size CAMRES(480, 360);

// board world geometry (z=0 plane), centred; corners TL,TR,BR,BL
static const float BW = 280.f;              // board width [mm]
float BH = 200.f;                           // board height (set from texture aspect)

// --- build a checkerboard-on-paper texture (RGB, white border) ---
static Img8u makeCheckerboard(int xc, int yc) {
  const int cell = 30, border = cell;
  const int W = xc*cell + 2*border, H = yc*cell + 2*border;
  Img8u img(Size(W, H), formatRGB);
  for (int c = 0; c < 3; ++c) std::fill(img.begin(c), img.end(c), (icl8u)255);
  for (int j = 0; j < yc; ++j)
    for (int i = 0; i < xc; ++i)
      if ((i+j) & 1)
        for (int y = 0; y < cell; ++y)
          for (int x = 0; x < cell; ++x) {
            const int px = border+i*cell+x, py = border+j*cell+y, idx = py*W+px;
            for (int c = 0; c < 3; ++c) img.begin(c)[idx] = 0;
          }
  return img;
}

// the 4 board corners in world (TL,TR,BR,BL) and matching texture corners
static std::vector<Vec> worldCorners() {
  return { Vec(-BW/2,  BH/2, 0, 1), Vec( BW/2,  BH/2, 0, 1),
           Vec( BW/2, -BH/2, 0, 1), Vec(-BW/2, -BH/2, 0, 1) };
}
// build a fresh textured board MeshNode for the current `tex` (z=0 plane)
static std::shared_ptr<MeshNode> makeBoardNode() {
  auto node = std::make_shared<MeshNode>();
  const auto wc = worldCorners();
  for (auto &v : wc) node->addVertex(v);
  for (int i = 0; i < 4; ++i) node->addNormal(Vec(0,0,1,1));
  node->addTexCoord(0,0); node->addTexCoord(1,0);
  node->addTexCoord(1,1); node->addTexCoord(0,1);
  node->addQuad(0,1,2,3, 0,1,2,3, 0,1,2,3);
  auto mat = Material::fromColor(GeomColor(255,255,255,255));
  mat->setBaseColorMap(Image(tex));
  // Calibration paper is matte: fully rough + non-metallic so the (Cycles) sun
  // doesn't blow a specular hot-spot across the board and wash out the corners.
  mat->roughness = 1.0f;
  mat->metallic  = 0.0f;
  node->setMaterial(mat);
  node->setPrimitiveVisible(PrimLine | PrimVertex, false);
  return node;
}

static void rebuildBoard(int xc, int yc) {
  tex = makeCheckerboard(xc, yc);
  BH = BW * tex.getHeight() / (float)tex.getWidth();
  curX = xc; curY = yc;

  // rebuild in both scenes: the on-screen interactive one and the capture mirror.
  // run() is the worker thread; the GUI thread renders both scenes — lock around
  // the node swaps so a render can't observe a half-rebuilt scene.
  scene.lock();
  if (board) scene.removeNode(board.get());
  board = makeBoardNode();
  scene.addNode(board);
  scene.unlock();

  capScene.lock();
  if (capBoard) capScene.removeNode(capBoard.get());
  capBoard = makeBoardNode();
  capScene.addNode(capBoard);
  capScene.unlock();

  view.invalidate();   // capture-scene geometry changed → resync the (Cycles) backend
}

static inline void sampleRGB(const Img8u &src, float x, float y, icl8u out[3]) {
  const int W = src.getWidth(), H = src.getHeight();
  if (x < 0 || y < 0 || x > W-1 || y > H-1) { out[0]=out[1]=out[2]=70; return; } // bg gray
  const int x0=(int)x, y0=(int)y; const float ax=x-x0, ay=y-y0;
  for (int c=0;c<3;++c){
    const icl8u *d = src.begin(c);
    const float a=d[y0*W+x0], b=d[y0*W+std::min(x0+1,W-1)],
                cc=d[std::min(y0+1,H-1)*W+x0], e=d[std::min(y0+1,H-1)*W+std::min(x0+1,W-1)];
    out[c] = (icl8u)((a*(1-ax)+b*ax)*(1-ay) + (cc*(1-ax)+e*ax)*ay + 0.5f);
  }
}

// apply a radial lens-distortion model to a (real-rendered) camera image:
// for output pixel p, sample the source at q = c + (p-c)*(1+k1 r^2 + k2 r^4).
static Img8u distortImage(const Img8u &src, float k1, float k2) {
  const int W = src.getWidth(), H = src.getHeight();
  if (!W || !H) return Img8u(CAMRES, formatRGB);   // empty render (e.g. no GL)
  Img8u out(Size(W,H), formatRGB);
  const float cx=W/2.f, cy=H/2.f, f=std::max(cx,cy);
  for (int y=0;y<H;++y) for (int x=0;x<W;++x){
    float qx=x, qy=y;
    if (k1!=0.f || k2!=0.f){                 // lens distortion (inverse map)
      const float nx=(x-cx)/f, ny=(y-cy)/f, r2=nx*nx+ny*ny, s=1+k1*r2+k2*r2*r2;
      qx = cx+(x-cx)*s; qy = cy+(y-cy)*s;
    }
    icl8u rgb[3]; sampleRGB(src, qx, qy, rgb);
    for (int c=0;c<3;++c) out.begin(c)[y*W+x]=rgb[c];
  }
  return out;
}

// Undistort a distorted view using the (here known) radial parameters: for each
// output pixel p, find the distorted source x with distort(x)=p (fixed point,
// since the map is near-identity) and sample there. With correct k1,k2 the
// checkerboard lines straighten out. NOTE: in a real calibration the parameters
// are NOT known from one frame — you move the board around the scene to collect
// many views and estimate them jointly; here the simulator hands us the truth.
static Img8u undistort(const Img8u &D, float k1, float k2) {
  const int W = D.getWidth(), H = D.getHeight();
  Img8u out(Size(W,H), formatRGB);
  const float cx=W/2.f, cy=H/2.f, f=std::max(cx,cy);
  for (int y=0;y<H;++y) for (int x=0;x<W;++x){
    float dx=x, dy=y;
    if (k1!=0.f || k2!=0.f)
      for (int it=0; it<6; ++it){          // solve distort(dx,dy) = (x,y)
        const float nx=(dx-cx)/f, ny=(dy-cy)/f, r2=nx*nx+ny*ny, s=1+k1*r2+k2*r2*r2;
        dx += x-(cx+(dx-cx)*s); dy += y-(cy+(dy-cy)*s);
      }
    icl8u rgb[3]; sampleRGB(D, dx, dy, rgb);
    for (int c=0;c<3;++c) out.begin(c)[y*W+x]=rgb[c];
  }
  return out;
}

// draw a result image + the detected corners (+ their two board axes)
static void drawResult(DrawHandle &draw, const Img8u &img, const std::vector<CornerSeed> &seeds) {
  draw = img;
  draw->linewidth(1.5);
  if (gui["showCorners"].as<bool>())
    for (const auto &s : seeds) {
      draw->color(0,255,0,255); draw->sym(s.pos, 'x');
      if (gui["showOri"].as<bool>()) {
        const float L=9.f, a=s.orientation;
        draw->color(255,200,0,255);
        draw->line(s.pos, s.pos + Point32f(std::cos(a), std::sin(a))*L);
        draw->line(s.pos, s.pos + Point32f(-std::sin(a), std::cos(a))*L);
      }
    }
  draw->color(255,255,255,255);
  draw->text("corners: " + str(seeds.size()), 5, 5, 9);
  draw.render();
}

void init() {
  // on-screen interactive scene (left pane): board shown flat (raw texture).
  scene.addCamera(Camera::lookAt(Vec(0,0,600,1), Vec(0,0,0,1), Vec(0,1,0,1), CAMRES, 35.f));
  scene.setBounds(400);
  scene.setPropertyValue("enable lighting", false);

  // dedicated offscreen capture scene (right pane): same camera params (synced
  // each frame from the interactive one) + a key light so lighting/shading can
  // be toggled on. Its "enable lighting" is driven live from the checkbox.
  capScene.addCamera(scene.getCamera(0));
  capScene.setBounds(400);
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setColor(GeomColor(255, 247, 235, 255));   // 0..255 (Cycles sync divides by 255)
  light->setIntensity(1.0f);
  light->translate(180, 220, 500);
  // No shadows on the capture: the board is a flat plane, so a shadow map adds
  // nothing — and it's the dominant per-frame GL cost (a 2048² depth pass every
  // offscreen capture). Keeping it off is a big framerate win.
  light->setShadowEnabled(false);
  capScene.addLight(light);

  rebuildBoard(7, 5);

  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="board (drag to view from any angle)", .minSize={22,18}})
          << (VBox()
              << Canvas({.handle="distorted", .label="rendered camera view (+distortion) + detection", .minSize={20,14}})
              << Canvas({.handle="undistorted", .label="undistorted (known k1,k2) + detection", .minSize={20,14}})
              << (VBox({.maxSize={100,12}})
                  << (HBox()
                      << Slider(3, 15, 7, {.handle="xc", .label="x cells"})
                      << Slider(3, 15, 5, {.handle="yc", .label="y cells"}))
                  << (HBox()
                      << FSlider(-0.4, 0.4, 0, {.handle="k1", .label="distortion k1"})
                      << FSlider(-0.3, 0.3, 0, {.handle="k2", .label="distortion k2"}))
                  << (HBox()
                      << FSlider(3, 9, 5, {.handle="radius", .label="ring radius"})
                      << FSlider(0.1, 0.8, 0.35, {.handle="minScore", .label="min score"}))
                  << (HBox()
#ifdef ICL_HAVE_CYCLES
                      << Combo("GL (fast),Cycles (photoreal)", {.handle="renderer", .label="offscreen"})
#endif
                      << CheckBox("lighting", {.checked=false, .handle="lighting"}))
                  << (HBox()
                      << CheckBox("corners", {.checked=true, .handle="showCorners"})
                      << CheckBox("orientation", {.checked=true, .handle="showOri"})
                      << Fps({.handle="fps"})))))
      << Show();

  view.setCaptureSource(capScene, 0);          // render capScene/cam0 offscreen
  gui["scene"].link(view.callback());          // GUI-thread view + GL capture
  gui["scene"].install(scene.getMouseHandler(0));
}

void run() {
  static FPSLimiter fps(60);   // cap the worker loop (don't spin at 100% CPU)

  const int xc = gui["xc"], yc = gui["yc"];
  if (xc != curX || yc != curY) rebuildBoard(xc, yc);

  // Read the live controls.
  int rmode = 0;
#ifdef ICL_HAVE_CYCLES
  rmode = gui["renderer"].as<ComboHandle>().getSelectedIndex();   // 0=GL, 1=Cycles
#endif
  const bool  lighting = gui["lighting"];
  const float k1 = gui["k1"], k2 = gui["k2"], minScore = gui["minScore"];
  const int   radius   = gui["radius"];
  const Camera &cam = scene.getCamera(0);
  auto sameVec = [](const Vec &a, const Vec &b){
    return a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3]; };

  // A capture is only needed when the rendered image would change (camera moved,
  // board resized, lighting/backend changed); re-detection is needed when a new
  // capture arrives OR a distortion/detector slider moved (no re-render then).
  static bool  have = false;
  static Vec   lPos, lNorm, lUp;
  static int   lXc=-1, lYc=-1, lMode=-1, lLight=-1, lRadius=-1;
  static float lK1=1e9f, lK2=1e9f, lMs=1e9f;
  const bool camMoved = !have || !sameVec(cam.getPosition(), lPos)
      || !sameVec(cam.getNorm(), lNorm) || !sameVec(cam.getUp(), lUp);
  const bool captureDirty = camMoved || xc!=lXc || yc!=lYc || rmode!=lMode || (int)lighting!=lLight;
  const bool detectParamsChanged = radius!=lRadius || k1!=lK1 || k2!=lK2 || minScore!=lMs;
  have = true;
  lPos = cam.getPosition(); lNorm = cam.getNorm(); lUp = cam.getUp();
  lXc=xc; lYc=yc; lMode=rmode; lLight=(int)lighting; lRadius=radius;
  lK1=k1; lK2=k2; lMs=minScore;

  // Drive the offscreen view. The OffscreenView hides the GL-on-GUI-thread /
  // Cycles-on-worker split — we just pick the backend, ask for a GL capture when
  // something changed, and poll for a new frame.
  view.setBackend(rmode == 1 ? OffscreenView::Backend::Cycles
                             : OffscreenView::Backend::GL);
  if (rmode == 0)                                  // GL lighting toggle (Cycles always lights;
    capScene.setPropertyValue("enable lighting", lighting);   // mutating it per-frame would
                                                   // keep Cycles perpetually dirty)
  if (captureDirty) view.requestCapture();

  gui["scene"].render();   // GUI thread: interactive view + (when pending) the GL capture

  // Consume + detect. poll() returns a new frame (GL: after a requested capture;
  // Cycles: on each progressive refinement). Re-detect on a new frame OR when a
  // detector slider moved (reusing the last frame).
  static Img8u lastFrame;
  Img8u frame;
  const bool newFrame = view.poll(frame);
  if (newFrame) lastFrame = frame;
  if ((newFrame || detectParamsChanged) && lastFrame.getDim()) {
    const Img8u distorted = distortImage(lastFrame, k1, k2);
    const Img8u undistorted = undistort(distorted, k1, k2);   // rectified w/ known params
    CheckerboardSaddleDetector::Params p;
    p.radius = radius; p.minScore = minScore;
    CheckerboardSaddleDetector det(p);
    DrawHandle d1 = gui["distorted"], d2 = gui["undistorted"];
    drawResult(d1, distorted,   det.detect(distorted));
    drawResult(d2, undistorted, det.detect(undistorted));
  }
  gui["fps"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  const int rc = ICLApp(n, ppc, "", init, run).exec();
  // Skip static-destruction teardown of the GL/Cycles globals (`cyc`'s Cycles
  // session, the two Scene2 Renderers' GL resources): by the time the
  // window has closed their GL context / threads are already gone, so their
  // dtors fault. _Exit hands everything back to the OS cleanly. (ICL itself uses
  // _Exit for its own diagnostic exit path, for the same reason.)
  std::cout.flush();
  std::cerr.flush();
  std::_Exit(rc);
}
