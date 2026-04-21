// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <icl/filter/LocalThresholdOp.h>
#include <icl/utils/ConfigFile.h>
#include <icl/qt/Common2.h>

#include <QInputDialog>
#include <mutex>

HBox gui;
GenericGrabber grabber;
LocalThresholdOp ltop;
std::recursive_mutex mtex;
Rect draggedRoi, usedRoi, fullImageRoi;

void step();

void mouse(const MouseEvent &e){
  if(e.isRight()){
    usedRoi = fullImageRoi;
    draggedRoi = Rect::null;
    step();
  }else if(e.isLeft()){
    if(e.isPressEvent()){
      draggedRoi = Rect(e.getPos(),Size(1,1));
    }else if(e.isDragEvent()){
      draggedRoi.width = e.getX() - draggedRoi.x;
      draggedRoi.height = e.getY() - draggedRoi.y;
    }else if(e.isReleaseEvent()){
      usedRoi = draggedRoi.normalized() & fullImageRoi;
      draggedRoi = Rect::null;
    }
    step();
  }
}

void save(){
  static int &masksize = gui.get<int>("masksize");
  static int &thresh = gui.get<int>("threshold");
  static float &gamma = gui.get<float>("gamma");
  bool ok = false;

  QString qname = QInputDialog::getText(0,"Save Params",
                                        "Please specify output xml-file name"
                                        ,QLineEdit::Normal,"local-threshold-params.xml",&ok);
  if(ok){
    ConfigFile f;
    f.set("config.masksize",masksize);
    f.set("config.threshold",thresh);
    f.set("config.gammaslope",gamma);
    f.save(qname.toLatin1().data());
  }
}

void step(){
  std::scoped_lock<std::recursive_mutex> lock(mtex);
  static DrawHandle orig = gui["orig"];
  static ImageHandle prev = gui["prev"];
  static ButtonHandle next = gui["next"];
  static LabelHandle time = gui["time"];
  static ComboHandle algorithm = gui["algorithm"];
  static FPSHandle fps = gui["fps"];
  bool loop = gui["loop"];
  bool clipToROI = gui["clipToROI"];
  int masksize = gui["masksize"];
  float threshold = gui["threshold"];
  float gamma = gui["gamma"];
  gui["save"].registerCallback([]{ save(); });

  ltop.setClipToROI(clipToROI);
  ltop.setup(masksize, threshold, (LocalThresholdOp::algorithm)(int)algorithm, gamma);

  static Image image;
  if(!image || loop || next.wasTriggered()){
    bool init = !image;
    image = grabber.grabImage();
    if(init){
      draggedRoi=fullImageRoi=image.getImageRect();
    }
  }

  Image useImage = image;
  if(usedRoi != image.getImageRect()){
    useImage = image.shallowCopy();
    useImage.setROI(usedRoi);
  }

  int minDim = 2 * masksize + 1;
  Time last = Time::now();
  Image result;
  if(usedRoi.width >= minDim && usedRoi.height >= minDim){
    ltop.apply(useImage, result);
  }
  time = str((Time::now()-last).toMilliSeconds())+"ms";

  orig = image;

  if(draggedRoi != Rect::null){
    orig->color(0,100,255);
    orig->fill(0,100,255,20);
    orig->rect(draggedRoi);
  }

  orig->color(255,0,0);
  orig->nofill();
  orig->rect(usedRoi);
  orig.render();

  prev = result;
  fps.render();
}

void init(){
  int masksize = 10;
  int thresh = 2;
  float gamma = 0;
  if(pa("-config")){
    ConfigFile f(*pa("-config"));
    masksize = f["config.masksize"];
    thresh = f["config.threshold"];
    gamma = f["config.gammaslope"];
  }

  gui << Canvas().minSize(16,12).handle("orig").label("original image")
      << Display().minSize(16,12).handle("prev").label("preview image")
      << ( VBox().label("controls")
           << Slider(2,200,masksize).label("mask size").out("masksize").minSize(15,2).handle("a")
           << FSlider(-30,40,thresh).label("threshold").out("threshold").minSize(15,2).handle("b")
           << FSlider(0,15,gamma).label("gamma slope").out("gamma").minSize(15,2).handle("c")
           << Button("next image").handle("next")
           << Button("stopped","running").out("loop").handle("d")
           << Button("no clip","clip to roi").out("clipToROI").handle("e")
           << Button("save params").handle("save")
           << Combo("region mean,tiledNN,tiledLIN").handle("algorithm").label("algorithm")
           << ( HBox()
                << Label("..ms").handle("time").label("apply time").minSize(2,3)
                << Fps(10).handle("fps").minSize(4,3).label("fps")
                )
           )
      << Show();

  grabber.init(pa("-i"));
  if(grabber.getType() != "file"){
    grabber.useDesired<Size>(pa("-s"));
    if(!pa("-color")){
      grabber.useDesired(formatGray);
    }else{
      grabber.useDesired(formatRGB);
    }
    grabber.useDesired(depth8u);
  }

  gui.registerCallback(step,"a,b,c,d,e,next,algorithm");
  gui["orig"].install(new MouseHandler(mouse));

  step();
}

void run(){
  while(!gui["loop"].as<bool>()) Thread::msleep(100);
  step();
}

int main(int argc, char **argv){
  return ICLApp(argc, argv,
                "[m]-input|-i(device,device-params) "
                "-config|-c(cfg-filename) "
                "-color -size|-s(size=VGA)",
                init, run).exec();
}
