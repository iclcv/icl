#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLCC/CCFunctions.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLQuick/Common.h>

#include <QInputDialog>
GUI gui("hbox");

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

  
  gui << "image[@minsize=32x24@handle=orig@label=original]";
  gui << "image[@minsize=32x24@handle=prev@label=preview]";
  gui << ( GUI("vbox[@label=controls]")
           << string("slider(2,200,")+str(masksize)+")[@label=mask size@out=masksize@minsize=15x2]"
           << string("slider(-30,40,")+str(thresh)+")[@label=threshold@out=threshold@minsize=15x2]"
           << string("fslider(0,15,")+str(gamma)+")[@label=gamma slope@out=gamma@minsize=15x2]"
           << "button(next image)[@handle=next]"
           << "togglebutton(stopped,running)[@out=run]"
           << "button(save params)[@handle=save]"
         );
  
  
  gui.show();
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

void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));

  if(g.getType() == "file"){
    g.setIgnoreDesiredParams(true);
  }else{
    g.setDesiredSize(Size(640,480));
    g.setDesiredFormat(formatGray);
    g.setDesiredDepth(depth8u);
  }

  static ImageHandle &orig = gui.getValue<ImageHandle>("orig");
  static ImageHandle &prev = gui.getValue<ImageHandle>("prev");
  static ButtonHandle &next = gui.getValue<ButtonHandle>("next");
  static bool &loop = gui.getValue<bool>("run");
  static int &masksize = gui.getValue<int>("masksize");
  static int &thresh = gui.getValue<int>("threshold");
  static float &gamma = gui.getValue<float>("gamma");
  gui.getValue<ButtonHandle>("save").registerCallback(new GUI::Callback(save));
  
  static LocalThresholdOp t;
  
  static int lastMaskSize=0;
  static int lastThresh=0;
  static float lastGamma=0;
  while(1){
    t.setMaskSize(masksize);
    t.setGlobalThreshold(thresh);
    t.setGammaSlope(gamma);
    
    const ImgBase *image = g.grab();
    if(image->getFormat() != formatGray){
      static ImgBase *grayImage = 0;
      ensureCompatible(&grayImage,depth8u,image->getSize(),formatGray);
      cc(image,grayImage);
      image = grayImage;
    }
    static ImgBase *dst = 0;
    
    t.apply(image,&dst);

    orig = image;
    orig.update();
    
    prev = dst;
    prev.update();
    
    lastMaskSize = masksize;
    lastThresh = thresh;
    lastGamma = gamma;
    while(!loop && !next.wasTriggered()){
      Thread::msleep(100);
      if(lastMaskSize != masksize || lastThresh != thresh || lastGamma != gamma){
        t.setMaskSize(masksize);
        t.setGlobalThreshold(thresh);
        t.setGammaSlope(gamma);
        lastMaskSize = masksize;
        lastThresh = thresh;
        lastGamma = gamma;
        

        t.apply(image,&dst);
        orig = image;
        orig.update();
        
        prev = dst;
        prev.update();
      }
    }
    
    Thread::msleep(20);
  }
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

int main (int argc, char **argv) {
  paex
  ("-input","generic-grabbers generic grabbers params")
  ("-config","config file input")
  ("-nogui","start without gui")
  ("-output","for no gui batchmode: define output-image pattern\n"
   "use ##### for the image index in this pattern");

  painit(argc,argv,"[m]-input|-i(device,device-params) "
         "-output|-o(output-file-pattern) -config|-c(cfg-filename) "
         " -nogui|-n");
  
  
  if(pa("-nogui")){
    batch_mode();    
  }else{
    return ICLApp(argc,argv,"",init,run).exec();
  }
}
