/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/local-threshold-op-test.cpp         **
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
#include <ICLCC/CCFunctions.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLQuick/Common.h>

#include <QInputDialog>
GUI gui("hbox");
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
  static int &masksize = gui.getValue<int>("masksize");
  static int &thresh = gui.getValue<int>("threshold");
  static float &gamma = gui.getValue<float>("gamma");
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
  gui_DrawHandle(orig);
  gui_ImageHandle(prev);
  gui_ButtonHandle(next);
  gui_LabelHandle(time);
  gui_ComboHandle(algorithm);
  gui_FPSHandle(fps);
  gui_bool(loop);
  gui_bool(clipToROI);
  gui_int(masksize);
  gui_float(threshold);
  gui_float(gamma);
  gui["save"].registerCallback(new GUI::Callback(save));

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

  
  orig->lock();
  orig->reset();
  if(selroi[0] != Rect::null){
    orig->color(0,100,255);
    orig->fill(0,100,255,20);
    orig->rect(selroi[0]);
  }
  
  orig->color(255,0,0);
  orig->nofill();
  orig->rect(selroi[1]);

  orig->unlock();
  
  
  orig.update();
    
  prev = result;
  prev.update();
  fps.update();
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

  
  gui << "draw[@minsize=32x24@handle=orig@label=original]";
  gui << "image[@minsize=32x24@handle=prev@label=preview]";
  gui << ( GUI("vbox[@label=controls]")
           << string("slider(2,200,")+str(masksize)+")[@label=mask size@out=masksize@minsize=15x2@handle=a]"
           << string("fslider(-30,40,")+str(thresh)+")[@label=threshold@out=threshold@minsize=15x2@handle=b]"
           << string("fslider(0,15,")+str(gamma)+")[@label=gamma slope@out=gamma@minsize=15x2@handle=c]"
           << "button(next image)[@handle=next]"
           << "togglebutton(stopped,running)[@out=loop@handle=d]"
           << "togglebutton(no clip,clip to roi)[@out=clipToROI@handle=e]"
           << "button(save params)[@handle=save]"
           << "combo(region mean,tiledNN,tiledLIN)[@handle=algorithm@label=algorithm]"
           << ( GUI("hbox")
                << "label(..ms)[@handle=time@label=apply time@minsize=2x3]"
                << "fps(10)[@handle=fps@minsize=4x3@label=fps]")
         );
  
  
  gui.show();
  
  grabber.init(FROM_PROGARG("-input"));
  if(grabber.getType() == "file"){
    grabber.setIgnoreDesiredParams(true);
  }else{
    grabber.setIgnoreDesiredParams(false);
    grabber.setDesiredSize(pa("-s"));
    if(!pa("-color")){
      grabber.setDesiredFormat(formatGray);
    }else{
      grabber.setDesiredFormat(formatRGB);
    }
    grabber.setDesiredDepth(depth8u);
  }
  
  gui.registerCallback(new GUI::Callback(step),"a,b,c,d,e,next,algorithm");
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
  
    static GenericGrabber g(FROM_PROGARG("-input"));
    FileList fl;
    int maxSteps = -1;
    if(g.getType() == "file"){
      fl = FileList(*pa("-input",1));
      maxSteps = fl.size();
    }
    g.setIgnoreDesiredParams(true);

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
  gui_bool(loop);
  while(!loop) Thread::msleep(100);
  step();
}

int main (int argc, char **argv) {
  paex
  ("-input","generic-grabbers generic grabbers params")
  ("-config","config file input")
  ("-size","grabbers desired image size")
  ("-nogui","start without gui")
  ("-output","for no gui batchmode: define output-image pattern\n"
   "use ##### for the image index in this pattern");

  painit(argc,argv,"[m]-input|-i(device,device-params) "
         "-output|-o(output-file-pattern) -config|-c(cfg-filename) "
         " -nogui|-n -color -size|-s(size=VGA)");
  
  
  if(pa("-nogui")){
    batch_mode();    
  }else{
    return ICLApp(argc,argv,"",init,run).exec();
  }
}
