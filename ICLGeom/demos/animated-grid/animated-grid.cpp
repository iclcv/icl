// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLQt/GLFragmentShader.h>
#include <mutex>
HBox gui;
Scene scene;
GenericGrabber grabber;
GenericGrabber backFaceGrabber;
Img8u image;
Img8u backImage;

struct Grid : public SceneObject{
  std::recursive_mutex mutex;
  int w,h;
  int idx(int x, int y) const { return x + w* y; }
  Grid(const ImgBase *image):w(50),h(50){

    m_vertices.resize(w*h,Vec(0,0,0,1));
    m_vertexColors.resize(w*h,GeomColor(1,0,0,1));


    if(pa("-b")){
      addCustomPrimitive(new TwoSidedTextureGridPrimitive(w,h,image,&backImage,
                                                          &m_vertices[0][0],
                                                          &m_vertices[0][1],
                                                          &m_vertices[0][2],
                                                          0,0,0, 4, false, false ));
    }else{
      addTextureGrid(w,h,image,
                     &m_vertices[0][0],
                     &m_vertices[0][1],
                     &m_vertices[0][2],
                     0,0,0,4,false);
    }

    for(int x=1;x<w;++x){
      addLine(idx(x-1,0),idx(x,0),geom_red());
      addLine(idx(x-1,w-1),idx(x,w-1),geom_red());
    }
    for(int y=1;y<h;++y){
      addLine(idx(0,y-1),idx(0,y),geom_red());
      addLine(idx(w-1,y-1),idx(w-1,y),geom_red());
    }

    setVisible(Primitive::texture,true);
    setVisible(Primitive::vertex,false);


    setFragmentShader(new GLFragmentShader( "","uniform sampler2D texSampler;"
                                            "void main(void){"
                                            "   vec4 tmp;"
                                            "   tmp = texture2D(texSampler, vec2(gl_TexCoord[0]));"
                                            "   for (int i=0; i<4; i++){"
                                            "     if ( tmp[i] > float(0.3)) gl_FragColor[i] = float(1);"
                                            "     else gl_FragColor[i] = tmp[i];"
                                            "   }"
                                            "}"));


  }

  void prepareForRendering(){
    const float freq = 10*(gui["freq"].as<float>()+0.05);
    const float ar = float(image.getWidth())/image.getHeight();
    static Time ref = Time::now();
    float t = ref.age().toSecondsDouble() * freq;
    for(int x=0;x<w;++x){
      for(int y=0;y<h;++y){
        Vec &v = m_vertices[idx(x,y)];
        v[0] = (x-w/2)*ar;
        v[1] = y-h/2;
        v[2] = 5 * sin(x/10. +t) + 3 * cos(y/5.+t*2);
        v[3] = 1;
      }
    }
  }
} *obj = 0;


void run(){
  grabber.grabImage().ptr()->convert(&image);
  if(pa("-b")){
    backFaceGrabber.grabImage().ptr()->convert(&backImage);
  }
  gui["draw"].render();
#if 0
  gui["offscreen0"] = scene.render(0);
  gui["offscreen1"] = scene.render(1);
#endif
}

void init(){
  scene.setBounds(100);
  scene.setDrawCamerasEnabled(false);

  grabber.init(pa("-i"));
  if(pa("-b")){
    backFaceGrabber.init(pa("-b"));
  }
  gui << Canvas3D().handle("draw").minSize(20,15).label("interaction area")
      << Display().handle("offscreen0").label("offscreen rendered cam 0")
      << Display().handle("offscreen1").label("offscreen rendered cam 1")
      << FSlider(0,1,0.2,true).out("freq").label("frequence")
      << Show();

  grabber.grabImage();
  grabber.grabImage();
  grabber.grabImage().ptr()->convert(&image);
  obj = new Grid(&image);

  scene.addObject(obj);
  scene.addCamera(Camera(Vec(0,0,60,1),
                         Vec(0,0,-1,1),
                         Vec(0,1,0,1)));
  scene.addCamera(scene.getCamera(0));

  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
}



int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2) -back-face-input|-b(2)" ,init,run).exec();
}
