// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// geom2 port of the legacy geom/offscreen-rendering demo: a video-feedback
// loop — the scene is rendered offscreen (Scene2::renderToImage), the result is
// fed back as the cube's own texture (with a drifting random border), so the
// cube shows ever-deeper reflections of itself.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/utils/Random.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

HBox gui;
Scene2 scene;
Img8u texImage;
std::shared_ptr<MeshNode> cube;

// a unit cube (half-extent h) with every face UV-mapped to the full texture
static std::shared_ptr<MeshNode> makeTexturedCube(float h, const Image &tex) {
  auto m = std::make_shared<MeshNode>();
  const float c[8][3] = {{-h,-h,-h},{ h,-h,-h},{ h, h,-h},{-h, h,-h},
                         {-h,-h, h},{ h,-h, h},{ h, h, h},{-h, h, h}};
  for (auto &p : c) m->addVertex(Vec(p[0], p[1], p[2], 1));
  const float nrm[6][3] = {{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}};
  for (auto &nn : nrm) m->addNormal(Vec(nn[0], nn[1], nn[2], 1));
  m->addTexCoord(0, 0); m->addTexCoord(1, 0); m->addTexCoord(1, 1); m->addTexCoord(0, 1);
  const int face[6][4] = {{4,5,6,7},{1,0,3,2},{5,1,2,6},{0,4,7,3},{7,6,2,3},{0,1,5,4}};
  for (int f = 0; f < 6; ++f)
    m->addQuad(face[f][0],face[f][1],face[f][2],face[f][3], f,f,f,f, 0,1,2,3);
  auto mat = Material::fromColor(GeomColor(255,255,255,255));
  mat->emissive = GeomColor(1,1,1,1);   // show the texture unshaded
  mat->setBaseColorMap(tex);
  m->setMaterial(mat);
  m->setPrimitiveVisible(PrimLine | PrimVertex, false);
  return m;
}

int updateDisplay(){
  Time t = Time::now();
  Img8u screen = scene.renderToImage(0).image;
  gui["image"] = Image(screen);

  screen.deepCopy(&texImage);
  texImage.setROI(Rect(10,10,280,280));
  URand r(-50,50);
  static icl64f col[] = {128,128,128};
  for(int i=0;i<3;++i){
    col[i] += r;
    if(col[i]<0) col[i]=0;
    if(col[i]>255) col[i]=255;
  }
  texImage.fillBorder(std::vector<icl64f>(col,col+3));
  cube->getMaterial()->setBaseColorMap(Image(texImage));
  return (Time::now()-t).toMilliSeconds();
}

void init(){
  gui << Canvas3D(Size(300,300), {.handle="draw", .label="3D scene", .minSize={16, 16}})
      << Canvas({.handle="image", .label="offscreen rendered image", .minSize={16, 16}})
      << Show();

  Camera cam(Vec(0,0,-13), Vec(0,0,1), Vec(1,0,0));
  cam.setResolution(Size(300,300));
  scene.addCamera(cam);

  texImage = Img8u(Size(300,300), formatRGB);
  std::fill(texImage.begin(0), texImage.end(0), 255);
  cube = makeTexturedCube(3, Image(texImage));
  scene.addNode(cube);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.0f);
  light->translate(0, 0, -200);
  scene.addLight(light);
  scene.setBounds(10);

  gui["draw"].install(scene.getMouseHandler(0));
  gui["draw"].link(scene.getGLCallback(0).get());
}

void run(){
  DrawHandle image = gui["image"];
  int ms = updateDisplay();
  image->color(255,255,255,255);
  image->text("offscreen rendering time: " + str(ms)+" ms",10,10,9);
  image.render();
  gui["draw"].render();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
