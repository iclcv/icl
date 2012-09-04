/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/local-thresh.cpp                    **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLQt/Common.h>
#include <ICLIO/FileList.h>

#include <QtGui/QInputDialog>
HBox gui;
GenericGrabber grabber;
LocalThresholdOp ltop;
Mutex mutex;
Rect selroi[3];

void step();

void mouse(const MouseEvent &e){
  if(e.isRight()){
    selroi[1] = selroi[2];
    selroi[0] = Rect::null;
    step();
  }else if(e.isLeft()){
    if(e.isPressEvent()){
      selroi[0] = Rect(e.getPos(),Size(1,1));
    }else if(e.isDragEvent()){
      selroi[0].width = e.getX() - selroi[0].x;
      selroi[0].height = e.getY() - selroi[0].y;
    }else if(e.isReleaseEvent()){
      selroi[1] = selroi[0].normalized();
      selroi[0] = Rect::null;
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
  Mutex::Locker lock(mutex);
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
  gui["save"].registerCallback(save);

  ltop.setClipToROI(clipToROI);
  ltop.setup(masksize, threshold, (LocalThresholdOp::algorithm)(int)algorithm, gamma);

  static const ImgBase *image = 0;
  if(!image || loop || next.wasTriggered()){
    bool init = !image;
    image = grabber.grab();
    if(init){
      selroi[0]=selroi[2]=image->getImageRect();
    }
  }

  const ImgBase *useImage = image;
  if(selroi[1] != image->getImageRect()){
    useImage = useImage->shallowCopy(selroi[1]);
  }
  Time last = Time::now();
  const ImgBase *result = ltop.apply(useImage);
  time = str((Time::now()-last).toMilliSeconds())+"ms";

        
  orig = image;
  
  if(image != useImage){
    delete useImage;
  }

  
  if(selroi[0] != Rect::null){
    orig->color(0,100,255);
    orig->fill(0,100,255,20);
    orig->rect(selroi[0]);
  }
  
  orig->color(255,0,0);
  orig->nofill();
  orig->rect(selroi[1]);
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

  gui << Draw().minSize(16,12).handle("orig").label("original image")
      << Image().minSize(16,12).handle("prev").label("preview image")
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


void batch_mode(){
  if(pa("-config")){
    ConfigFile f(*pa("-config"));
    int masksize = f["config.masksize"];
    int thresh = f["config.threshold"];
    float gamma = f["config.gammaslope"];
    if(!pa("-output")){
      printf("please specify output file pattern\n");
      return;
    }
  
    static GenericGrabber g(pa("-i"));
    FileList fl;
    int maxSteps = -1;
    if(g.getType() == "file"){
      fl = FileList(*pa("-input",1));
      maxSteps = fl.size();
    }

    static FileWriter w(*pa("-output"));
    static LocalThresholdOp t;
    t.setMaskSize(masksize);
    t.setGlobalThreshold(thresh);
    t.setGammaSlope(gamma);
  
    int i=0;
    while(maxSteps < 0 || maxSteps--){
      if(maxSteps > 0){
        printf("processing image %30s ......",fl[i++].c_str());
      }
      const ImgBase *image = g.grab();
      if(image->getFormat() != formatGray){
        printf("...");
        static ImgBase *grayImage = 0;
        ensureCompatible(&grayImage,depth8u,image->getSize(),formatGray);
        cc(image,grayImage);
        printf("...");
        image = grayImage;
      }
      static ImgBase *dst = 0;
      printf("...");
      t.apply(image,&dst);
      printf("...");
      w.write(dst); 
      printf("done! \n");
      printf("writing image to %30s ... done\n",w.getFilenameGenerator().showNext().c_str());
    }
  }else{
    ERROR_LOG("please run with -config config-filename!");
    return;
  }
}

void run(){
  while(!gui["loop"].as<bool>()) Thread::msleep(100);
  step();
}

int main (int argc, char **argv) {
  pa_explain("-input","generic-grabbers generic grabbers params")
            ("-config","config file input")
            ("-size","grabbers desired image size")
            ("-nogui","start without gui")
            ("-output","for no gui batchmode: define output-image pattern\n"
             "use ##### for the image index in this pattern");

  pa_init(argc,argv,"[m]-input|-i(device,device-params) "
          "-output|-o(output-file-pattern) -config|-c(cfg-filename) "
          " -nogui|-n -color -size|-s(size=VGA)");
  
  
  if(pa("-nogui")){
    batch_mode();    
  }else{
    return ICLApp(argc,argv,"",init,run).exec();
  }
}
