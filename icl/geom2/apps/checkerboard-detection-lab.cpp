// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive checkerboard-detection lab (Phase B tuning tool). LEFT: a virtual
// checkerboard on a bordered "paper" in a geom2 Scene2 — rotate/pan/zoom it with
// the mouse to view from any angle. RIGHT: the camera's view of that planar board
// (computed GL-free as a homography warp of the board texture), passed through a
// live lens-distortion model, then through cv::CheckerboardSaddleDetector — the
// detected corners + orientations are drawn on top. Cells, distortion, and
// detector parameters are all live sliders.
//
// This is the interactive sibling of the synthetic calibration harness: it lets
// us watch and tune the native detector under arbitrary viewpoint + distortion.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/math/transform/Homography2D.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::core;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;
std::shared_ptr<MeshNode> board;            // textured quad shown in the 3D scene
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
static std::vector<Point32f> texCorners() {
  const float tw = tex.getWidth(), th = tex.getHeight();
  return { Point32f(0,0), Point32f(tw,0), Point32f(tw,th), Point32f(0,th) };
}

static void rebuildBoard(int xc, int yc) {
  tex = makeCheckerboard(xc, yc);
  BH = BW * tex.getHeight() / (float)tex.getWidth();
  curX = xc; curY = yc;

  if (board) scene.removeNode(board.get());
  board = std::make_shared<MeshNode>();
  const auto wc = worldCorners();
  for (auto &v : wc) board->addVertex(v);
  for (int i = 0; i < 4; ++i) board->addNormal(Vec(0,0,1,1));
  board->addTexCoord(0,0); board->addTexCoord(1,0);
  board->addTexCoord(1,1); board->addTexCoord(0,1);
  board->addQuad(0,1,2,3, 0,1,2,3, 0,1,2,3);
  auto mat = Material::fromColor(GeomColor(255,255,255,255));
  mat->emissive = GeomColor(1,1,1,1);       // unlit → raw pattern, distortion-clean
  mat->setBaseColorMap(Image(tex));
  board->setMaterial(mat);
  board->setPrimitiveVisible(PrimLine | PrimVertex, false);
  scene.addNode(board);
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

// render the camera's view of the planar board (homography warp of the texture)
// then apply a radial lens-distortion model: src = c + (q-c)*(1+k1 r^2 + k2 r^4)
static Img8u renderCameraView(const Camera &cam, float k1, float k2) {
  const int W = CAMRES.width, H = CAMRES.height;
  // homography image->texture from the 4 corner correspondences
  std::vector<Point32f> img;
  for (auto &v : worldCorners()) img.push_back(cam.project(v));
  const auto tc = texCorners();
  GenericHomography2D<float> H2img2tex(img.data(), tc.data(), 4);

  Img8u out(Size(W,H), formatRGB);
  const float cx=W/2.f, cy=H/2.f, f=std::max(cx,cy);
  for (int y=0;y<H;++y) for (int x=0;x<W;++x){
    float qx=x, qy=y;
    if (k1!=0.f || k2!=0.f){                 // lens distortion (inverse map)
      const float nx=(x-cx)/f, ny=(y-cy)/f, r2=nx*nx+ny*ny, s=1+k1*r2+k2*r2*r2;
      qx = cx+(x-cx)*s; qy = cy+(y-cy)*s;
    }
    const Point32f t = H2img2tex.apply(Point32f(qx,qy));
    icl8u rgb[3]; sampleRGB(tex, t.x, t.y, rgb);
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
  scene.addCamera(Camera::lookAt(Vec(0,0,600,1), Vec(0,0,0,1), Vec(0,1,0,1), CAMRES, 35.f));
  scene.setBounds(400);
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->translate(0,0,600); scene.addLight(light);
  rebuildBoard(7, 5);

  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="board (drag to view from any angle)", .minSize={22,18}})
          << (VBox()
              << Canvas({.handle="distorted", .label="distorted camera view + detection", .minSize={20,14}})
              << Canvas({.handle="undistorted", .label="undistorted (known k1,k2) + detection", .minSize={20,14}})
              << (VBox({.maxSize={100,11}})
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
                      << CheckBox("corners", {.checked=true, .handle="showCorners"})
                      << CheckBox("orientation", {.checked=true, .handle="showOri"})
                      << Fps({.handle="fps"})))))
      << Show();

  gui["scene"].link(scene.getGLCallback(0).get());
  gui["scene"].install(scene.getMouseHandler(0));
}

void run() {
  const int xc = gui["xc"], yc = gui["yc"];
  if (xc != curX || yc != curY) rebuildBoard(xc, yc);

  gui["scene"].render();

  const float k1 = gui["k1"], k2 = gui["k2"];
  const Img8u distorted = renderCameraView(scene.getCamera(0), k1, k2);
  const Img8u undistorted = undistort(distorted, k1, k2);   // rectified w/ known params

  CheckerboardSaddleDetector::Params p;
  p.radius = gui["radius"]; p.minScore = gui["minScore"];
  CheckerboardSaddleDetector det(p);

  DrawHandle d1 = gui["distorted"], d2 = gui["undistorted"];
  drawResult(d1, distorted,   det.detect(distorted));
  drawResult(d2, undistorted, det.detect(undistorted));
  gui["fps"].render();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
