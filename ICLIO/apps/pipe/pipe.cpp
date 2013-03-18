/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/apps/pipe/pipe.cpp                               **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/GenericImageOutput.h>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSEstimator.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/ConsoleProgress.h>
#include <ICLIO/IOFunctions.h>

#include <ICLFilter/MedianOp.h>
#include <ICLFilter/ConvolutionOp.h>

#include <pthread.h>

#ifdef HAVE_QT
VBox gui;
#endif

// also comment in include/ICLUtils/PThreadFix.h and .cpp
// EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK;

bool first = true;
bool *ppEnabled = 0;

GenericGrabber grabber;

std::vector<string> remove_size(const vector<string> &v){
  vector<string> r;
  for(unsigned int i=0;i<v.size();++i){
    if(v[i] != "size") r.push_back(v[i]);
  }
  return r;
}  

void init_grabber(){
  grabber.init(pa("-i"));
  //grabber->setIgnoreDesiredParams(true);
   if(pa("-size")){
    grabber.useDesired<Size>(pa("-size"));
  }
  if(pa("-depth")){
    grabber.useDesired<depth>(pa("-depth"));
  }
  if(pa("-format")){
    grabber.useDesired<format>(pa("-format"));
  }

  
  if(pa("-camera-config")){
    grabber.loadProperties(pa("-camera-config"));
  }  
}

const ImgBase *grab_image(){
  const ImgBase *img = 0;
  //  const ImgBase *image = grabber.grab();
  if(!pa("-flip")){
    img = grabber.grab();
  }else{
    ImgBase *hack = const_cast<ImgBase*>(grabber.grab());
    std::string axis = pa("-flip");
    if(axis  ==   "x"){
      hack->mirror(axisVert);
    }else if(axis  ==  "y"){
      hack->mirror(axisHorz);
    }else if(axis == "both" || axis == "xy"){
      hack->mirror(axisBoth);
    }else{
      ERROR_LOG("nothing known about axis " <<  axis << "(allowed arguments are x,y or both)");
    }
    img = hack;
  }

  /* this does not work in the intended way because the images are converted to dst-format by the
     grabbers grab function using useDesired ...
  static bool reint = pa("-reinterpret-input-format");
  if(reint){
    static SmartPtr<const ImgBase> re;
    static format fmt = pa("-reinterpret-input-format");
    format ifmt = img->getFormat();
    if(fmt != formatMatrix && getChannelsOfFormat(fmt) != img->getChannels()){
      ERROR_LOG("cannot reinterpret image format " << str(ifmt) 
                << " to " << str(fmt) << " (cannel cout missmatch)");
    }else{
      re = img->reinterpretChannels(fmt);
      img = re.get();
    }
  }
   */
  
  if(!pa("-clip")){
    return img;
    
  }else{
    if(*pa("-clip")=="interactive"){
      throw ICLException("interactive clipmode is not yet implemented ...");
    }else{
      static Rect *r = 0;
      static ImgBase *clipped = 0;
      if(!r){
        r = new Rect;
        *r = pa("-clip");
        
        ICLASSERT_THROW(r->width <= img->getWidth(),ICLException("clipping rect width is larger then image width"));
        ICLASSERT_THROW(r->height <= img->getHeight(),ICLException("clipping rect height is larger then image height"));
        ICLASSERT_THROW(r->x>= 0,ICLException("clipping x-offset < 0"));
        ICLASSERT_THROW(r->y>= 0,ICLException("clipping y-offset < 0"));
        ICLASSERT_THROW(r->right() < img->getWidth(),ICLException("clipping rect's right edge is outside the image rect"));
        ICLASSERT_THROW(r->bottom() < img->getHeight(),ICLException("clipping rect's right edge is outside the image rect"));
        clipped = imgNew(img->getDepth(),r->getSize(),img->getChannels(),img->getFormat()); 
      }
      const ImgBase *tmp = img->shallowCopy(*r);
      tmp->deepCopyROI(&clipped);
      delete tmp;
      img = clipped;
    }
  }
  return img;
}

void send_app(){
  static GenericImageOutput output(pa("-o"));
#ifdef HAVE_QT
  ImageHandle IH;
  FPSHandle FPS;
  if(!pa("-no-gui")){
    IH = gui.get<ImageHandle>("image");
    FPS= gui.get<FPSHandle>("fps");
  }
#endif
 
  while(first || !pa("-single-shot")){
    const ImgBase *grabbedImage = grab_image();

    const ImgBase *ppImage = 0;
    if(pa("-pp") && *ppEnabled){
      static UnaryOp *pp = 0;
      if(!pp){
        static std::string pps = pa("-pp");
        if(pps == "gauss"){
          pp = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss3x3));
        }else if(pps == "gauss5") {
          pp = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss5x5));
        }else if(pps == "median"){
          pp = new MedianOp(Size(3,3));
        }else if(pps == "median5"){
          pp = new MedianOp(Size(5,5));
        }else{
          ERROR_LOG("undefined preprocessing mode");
          ::exit(0);
        }
      }
      pp->setClipToROI(false);
      ppImage  = pp->apply(grabbedImage);
      static const bool ppp = pa("-ppp");
      if(!ppp){
        const_cast<ImgBase*>(ppImage)->setFullROI();
      }
    }else{
      ppImage = grabbedImage;
    }
    const ImgBase *normImage = 0;
    if(pa("-normalize")){
      static ImgBase *buf = 0;
      ppImage->deepCopy(&buf);
      buf->normalizeAllChannels(Range64f(0,255));
      normImage = buf;
    }else{
      normImage = ppImage;
    }

    output.send(normImage);
#ifdef HAVE_QT
    if(!pa("-no-gui")){
      bool &updateImages = gui.get<bool>("updateImages");
      if(updateImages){
        IH = normImage;
      }
      FPS.render();
    }
#endif
    first = false;
    
    bool useGUI = false;
#ifdef HAVE_QT
    int fpsLimit = 0;
    if(!pa("-no-gui")){
      fpsLimit = gui.get<int>("fpsLimit");
      useGUI = true;
    }else{
      fpsLimit = pa("-fps").as<int>();
    }
#else
    int fpsLimit = pa("-fps");
#endif
    
    if(!useGUI){
      if(pa("-progress")){
          static int curr = 0;
          static bool first = true;
          if(first){
            progress_init("icl-pipe sending next 100 images");
            first = false;
          }
          static FPSEstimator fpsEst(10);
          progress(curr++,99,"(" + fpsEst.getFPSString() + ")");
          if(curr == 100){
            curr = 0;
          }
        }
    }

    static FPSLimiter limiter(15,10);
    if(limiter.getMaxFPS() != fpsLimit) limiter.setMaxFPS(fpsLimit);
    
    limiter.wait();

  }
}

#ifdef HAVE_QT
void init_gui(){
  bool idu = pa("-idu");
  
  if(pa("-pp")){
    gui << Image().handle("image").minSize(12,8) 
        << ( VBox().maxSize(100,8) 
             <<  ( HBox() 
                   << CamCfg().maxSize(5,2)
                   << Spinner(1,100,pa("-fps").as<int>()).out("fpsLimit").label("max fps")
                   << Fps(10).handle("fps")
                   )
             <<  ( HBox() 
                   << Button("off","on",!idu).out("updateImages").label("update images")
                   << Button("off","!on").handle("_").out("pp-on").label("preprocessing").minSize(5,2)
                   )
             );
    gui.show();

    ppEnabled = &gui.get<bool>("pp-on");
  }else{
    gui << Image().handle("image").minSize(12,8) 
        << ( VBox().maxSize(100,8) 
             <<  ( HBox() 
                   << CamCfg().maxSize(5,2)
                   << Spinner(1,100,pa("-fps").as<int>()).out("fpsLimit").label("max fps")
                   )
             <<  ( HBox() 
                   << Fps(10).handle("fps")
                   << Button("off","on",!idu).out("updateImages").label("update images")
                   )
             );
    gui.show();

    ppEnabled = new bool(false);
  }
}
#endif


int main(int n, char **ppc){
  pa_explain
  ("-input","for sender application only allowed ICL default\n"
   " input specification e.g. -input pwc 0 or -input file bla/*.ppm")
  ("-single-shot","no loop application")
  ("-size","output image size (sending only, default: VGA)"
   "[please note that -format, -size and -depth use the grabbers desired params."
   " I.e. usually none or all of these three parameters have to be given]")
  ("-depth","output image size (sending only, default: depth8u)"
   "[please note that -format, -size and -depth use the grabbers desired params."
   " I.e. usually none or all of these three parameters have to be given]")
  ("-format","if given the source image is converted into this format"
   "[please note that -format, -size and -depth use the grabbers desired params."
   " I.e. usually none or all of these three parameters have to be given]")
  ("-o","analog to -input , this can be used to specify the output device and parameters\n"
   " output specification e.g. -output file image_###.ppm or -o sm MySharedMem")
  ("-fps","initial max FPS count, further adjustable in the GUI")
  ("-no-gui","dont display a GUI (sender app only this is default if Qt is not available)")
  ("-flip","define axis to flip (allowed sub arguments are"
   " x, y or both")
  ("-clip","define clip-rect ala ((x,y)WxH) or string interactive (which is not yet supported)")
  ("-pp","select preprocessing (one of \n"
   "\t- gauss 3x3 gaussian blur\n"
   "\t- gauss5 5x5 gaussian blur\n"
   "\t- median 3x3 median filter\n"
   "\t- median5 5x5 median filter\n")
  ("-ppp","if this flag is set, the image ROI, that results form preprocessing is actually sent")
  //("-dist","give 4 parameters for radial lens distortion.\n"
  // "\tThis parameters can be obtained using ICL application\n"
  //"\ticl-calib-radial-distortion")
  ("-reset","reset bus on startup")
  //("-reinterpret-input-format","can be used to e.g. reinterpret input matrix-format images as gray") 
  ("-progress","show progress bar (only used in -no-gui mode)")
  ("-idu","if this is given, image updates are initally switched off which means, that no"
   "image is visualized in the preview widget. This helps to reduce network traffic!")
  ("-normalize","normalize resulting image to [0,255]")
  ("-camera-config","if a valid xml-camera configuration file was given here, the grabber is set up "
   "with this parameters internally. Valid parameter files can be created with icl-camera-param-io or with "
   "the icl-camcfg tool. Please note: some grabber parameters might cause an internal grabber crash, "
   "so e.g. trigger setup parameters or the isospeed parameters must be removed from this file");

  pa_init(n,ppc,"[m]-output|-o(output-type-string,output-parameters) "
         "-flip|-f(string) -single-shot [m]-input|-i(device,device-params) "
         "-size|(Size) -no-gui -pp(1) "
          //"-reinterpret-input-format(fmt) "
	 //-dist|-d(float,float,float,float) -reset|-r "
	 "-dist|-d(fn) -reset|-r "
         "-fps(float=15.0) -clip|-c(Rect) -camera-config(filename) -depth(depth) -format(format) -normalize|-n "
         "-perserve-preprocessing-roi|-ppp -progress "
         "-initially-disable-image-updates|-idu");

  if(pa("-reset")){
    GenericGrabber::resetBus();
  }
  
 
  init_grabber();  

#ifdef HAVE_QT
  if(!pa("-no-gui")){
    return ICLApp(n,ppc,"",init_gui,send_app).exec();
  }else{
    static bool alwaysTrue = 1;
    ppEnabled = &alwaysTrue;
    send_app();
  }
#else
  static bool alwaysTrue = 1;
  ppEnabled = &alwaysTrue;
  send_app();
#endif
}
