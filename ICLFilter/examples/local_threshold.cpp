#include <iclQuick.h>
#include <iclGUI.h>
#include <iclQt.h>
#include <iclStringUtils.h>
#include <iclGenericGrabber.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>
#include <iclLocalThresholdOp.h>
#include <iclProgArg.h>
#include <iclThread.h>
#include <iclCC.h>
#include <iclConfigFile.h>
#include <QInputDialog>
#include <iclMutex.h>
GUI gui("hbox");
GUI zoomGUI;
static Point zoomCenter(320,240);

void init(){
  int masksize = 10;
  int thresh = 2;
  int gamma = 0;
  if(pa_defined("-config")){
    std::string configFileName = pa_subarg<string>("-config",0,"");
    if(configFileName == "") goto END;
    ConfigFile f;
    f.load(configFileName);
    masksize = f.get<int>("config.masksize");
    thresh = f.get<int>("config.threshold");
    gamma = f.get<float>("config.gammaslope");
  }
  END:
  
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
           

  zoomGUI << "image[@minsize=32x24@handle=zoom@label=zoom]";
  zoomGUI.show();
}

struct MouseIO : public MouseInteractionReceiver{
  virtual void processMouseInteraction(MouseInteractionInfo *info){
    if(info->downmask[0] || info->downmask[1] || info->downmask[2]){
      zoomCenter = Point(info->imageX,info->imageY);
    }
  }
} mouseIO;

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
    f.add("config.masksize",masksize);
    f.add("config.threshold",thresh);
    f.add("config.gammaslope",gamma);
    f.save(qname.toLatin1().data());
  }
}

void update_zoom(Img8u &prevImage){
  static ImageHandle &h= zoomGUI.getValue<ImageHandle>("zoom");
  
  Point p = zoomCenter;
  
  if(!prevImage.getImageRect().contains(p.x,p.y)) return;
  static const Size zoomSize = translateSize(pa_subarg<string>("-zoomsize",0,"640x480"));
  static Img8u zoomRegion(zoomSize,formatGray);
  prevImage.setROI(Rect(p.x-zoomSize.width/2,p.y-zoomSize.height/2,zoomSize.width,zoomSize.height) & prevImage.getImageRect());
  if(prevImage.getROISize() == zoomSize){
    prevImage.deepCopyROI(&zoomRegion);
  }
  prevImage.setFullROI();
  h = zoomRegion;
  h.update();
}

void run(){
  static GenericGrabber g(pa_subarg<string>("-input",0,"pwc,dc,file"),
                          pa_subarg<string>("-input",1,"pwc=0,dc=0,file=images/*.ppm"));
  if(g.getType() == "file"){
    g.setIgnoreDesiredParams(true);
  }else{
    g.setDesiredSize(Size(640,480));
    g.setDesiredFormat(formatGray);
    g.setDesiredDepth(depth8u);
  }

  static ImageHandle &orig = gui.getValue<ImageHandle>("orig");
  static ImageHandle &prev = gui.getValue<ImageHandle>("prev");
  (*prev)->add(&mouseIO);
  static ButtonHandle &next = gui.getValue<ButtonHandle>("next");
  static bool &loop = gui.getValue<bool>("run");
  static int &masksize = gui.getValue<int>("masksize");
  static int &thresh = gui.getValue<int>("threshold");
  static float &gamma = gui.getValue<float>("gamma");
  gui.getValue<ButtonHandle>("save").registerCallback(save);
  
  static LocalThresholdOp t;
  
  static int lastMaskSize=0;
  static int lastThresh=0;
  static float lastGamma=0;
  static Point lastZoomCenter = zoomCenter;
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


    update_zoom(*dst->asImg<icl8u>());

    
    lastMaskSize = masksize;
    lastThresh = thresh;
    lastGamma = gamma;
    lastZoomCenter = zoomCenter;
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

        update_zoom(*dst->asImg<icl8u>());
        lastZoomCenter = zoomCenter;
      }
      else if(lastZoomCenter != zoomCenter){
        update_zoom(*dst->asImg<icl8u>());
        lastZoomCenter = zoomCenter;
      }
    }
    
    Thread::msleep(20);
  }
}

void batch_mode(){
  if(pa_defined("-config")){
    std::string configFileName = pa_subarg<string>("-config",0,"");
    if(configFileName == ""){
      ERROR_LOG("no config filename !");
      return;
    }
    printf("filename = -%s- \n",configFileName.c_str());
    ConfigFile f;
    f.load(configFileName);
    int masksize = f.get<int>("config.masksize");
    int thresh = f.get<int>("config.threshold");
    float gamma = f.get<float>("config.gammaslope");

    if(!pa_defined("-output")){
      printf("please specify output file pattern\n");
      return;
    }
    if(!pa_defined("-input") || (pa_subarg<string>("-input",0,"") != "file")){
      printf("please define args -input file filename!\n");
      return;
    }
    string inputFilePattern = pa_subarg<string>("-input",1,"images/*.ppm");
    if(inputFilePattern.find("file=") == 0){
      inputFilePattern = inputFilePattern.substr(5);
    }
    printf("input file pattern was %s \n",inputFilePattern.c_str());
    static FileGrabber g(inputFilePattern);
    g.setIgnoreDesiredParams(true);

    static FileWriter w(pa_subarg<string>("-output",0,"./local_threshold_image_######.jpg"));
    static LocalThresholdOp t;
    t.setMaskSize(masksize);
    t.setGlobalThreshold(thresh);
    t.setGammaSlope(gamma);
  
    int fileCount = g.getFileCount();
    while(fileCount--){
      printf("processing image %30s ......",g.getNextFileName().c_str());
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


/* Call e.g. 
examples/local_threshold -nogui -input file 'file=/home/celbrech/Desktop/Bilder/ *jpg' -config ./local-threshold-params.xml -output './images/image_#####.ppm'    
*/
int main (int argc, char **argv) {
  pa_explain("-input","generic-grabbers generic grabbers params");
  pa_explain("-config","config file input");
  pa_explain("-zoomsize","size of zoom preview image (e.g. -zoom 320x240)");
  pa_explain("-nogui","start without gui");
  pa_explain("-output","for no gui batchmode: define output-image pattern");
  
  pa_init(argc,argv,"-input(2) -output(1) -config(1) -zoomsize(1) -nogui");
  
  if(pa_defined("-nogui")){
    batch_mode();    
  }else{
    QApplication app(argc,argv);
    
    init();
    
    exec_threaded(run);
    
    return app.exec();
  }
}
