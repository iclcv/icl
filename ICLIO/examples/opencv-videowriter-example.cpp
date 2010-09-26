/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLOpenCV/examples/opencv-videowriter-example.cpp      **
 ** Module : ICLOpenCV                                              **
 ** Authors: Christian Groszewski 			           			   **
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
#include <ICLQuick/Common.h>
#include <ICLIO/OpenCVCamGrabber.h>
#include <ICLIO/OpenCVVideoWriter.h>
#include <ICLUtils/Time.h>
#include <QtGui/QFileDialog>

GUI gui("hsplit");

ImgBase *image=0;
Mutex mutex;
SmartPtr<OpenCVCamGrabber> cg=0;
SmartPtr<OpenCVVideoWriter> vw = 0;

std::string filename = "";

bool cap = false;

void saveAs(){
  QString fnNew = QFileDialog::getSaveFileName(0,"save...","./","AVI-Files (*.avi)");
  if(fnNew == "")
    return;
  else
    filename = fnNew.toStdString();
}

void run(){
  Mutex::Locker lock(mutex);
  cg->grab(&image);
  gui["image"] = image;

  if(cap)
    vw->write(image);

  gui["image"].update();

  gui_FPSHandle(fps);
  fps.update();
}

///sets the new brightness for capturedevice
void set_brightness(){
  Mutex::Locker lock(mutex);
  float f = gui.getValue<float>("bright");
  ostringstream Str;
  Str << f ;
  cg->setProperty("brightness", Str.str());
}

///sets the new hue for capturedevice
void set_hue(){
  Mutex::Locker lock(mutex);
  float f = gui.getValue<float>("hue");
  ostringstream Str;
  Str << f ;
  cg->setProperty("hue", Str.str());
}

///sets the new contrast for capturedevice
void set_contrast(){
  Mutex::Locker lock(mutex);
  float f = gui.getValue<float>("cont");
  ostringstream Str;
  Str << f ;
  cg->setProperty("contrast", Str.str());
}

///sets the new saturation for capturedevice
void set_saturation(){
  Mutex::Locker lock(mutex);
  float f = gui.getValue<float>("sat");
  ostringstream Str;
  Str << f ;
  cg->setProperty("saturation", Str.str());
}

///sets the new size for capturedevice
void set_size(){
  Mutex::Locker lock(mutex);
  int i = gui["size"];
  ostringstream Str;
  Str << i <<"x"<<int(0.75*i) ;
  cg->setProperty("size", Str.str());
}

///returns a timestamp
std::string getTimestamp(std::string suffix){
  Time t;
  int64_t ms = (t.now()).toMilliSeconds();
  ostringstream Str;
  Str << ms << suffix;
  return Str.str();
}

/// starts the capturing
void startcap(){
  Mutex::Locker lock(mutex);
  if(!vw){
    if(filename == "")
      saveAs();
    gui_ComboHandle(Codec);
    //filename = getTimestamp(".avi");
    //CV_FOURCC('I', 'Y', 'U', 'V')
    //CV_FOURCC('P','I','M','1')    = MPEG-1 codec
    //CV_FOURCC('M','J','P','G')    = motion-jpeg codec (does not work well)
    //CV_FOURCC('M', 'P', '4', '2') = MPEG-4.2 codec
    //CV_FOURCC('D', 'I', 'V', '3') = MPEG-4.3 codec
    //CV_FOURCC('D', 'I', 'V', 'X') = MPEG-4 codec
    //CV_FOURCC('U', '2', '6', '3') = H263 codec
    //CV_FOURCC('I', '2', '6', '3') = H263I codec
    //CV_FOURCC('F', 'L', 'V', '1') = FLV1 codec
    if(filename == "")
      filename = getTimestamp(".avi");

    vw = new OpenCVVideoWriter(filename ,(OpenCVVideoWriter::FOURCC) (int)Codec, 30.0, icl::Size(640,480), 1);

  }
  cap = true;
}

/// stops the capturing
void stopcap(){
  Mutex::Locker lock(mutex);
  cap = false;
  filename = "";
  vw = 0;
}

///takes a snapshot of device and writes it to a file
void takeSnapshot(){
  Mutex::Locker lock(mutex);
  if(cg){
    ImgBase *img=0;
    cg->grab(&img);
    QString fnNew = QFileDialog::getSaveFileName(0,"save...","./","PNG-Files (*.png)");
    if(fnNew.toStdString() != ""){
      ostringstream Str;
      Str <<  fnNew.toStdString() << ".png";
      FileWriter f(Str.str());
      f.write(img);
    }
    delete img;
  }
}

///prints all properties of current device to shell
void printAllProperties(){
  Mutex::Locker lock(mutex);
  if(cg){
    cout << "brightness: "<<cg->getValue("brightness")<<endl;
    cout << "contrast: "<<cg->getValue("contrast")<<endl;
    cout << "saturation: "<<cg->getValue("saturation")<<endl;
    cout << "hue: "<<cg->getValue("hue")<<endl;
    cout << "size: "<<cg->getValue("size")<<endl;
  }
}

void init(){
  if(pa("-i")){
    cg = new OpenCVCamGrabber(parse<int>(pa("-input")));
  }else{
    cg = new OpenCVCamGrabber(0);
  }
  cg->setIgnoreDesiredParams(true);

  if(pa("-f")){
    filename = parse<std::string>(pa("-f"));
  }else {
    filename = getTimestamp(".avi");
  }
  gui << (GUI("vbox")
          <<	"image[@handle=image@minsize=20x20]"
          << "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]"//)
          );
  gui << (GUI("vbox[@minsize=20x1]")
          << "slider(0,1000,400)[@out=size@handle=hsize@label=size]"
          << "label(only for cameras)"
          << "fslider(0,1,0.1)[@out=bright@handle=hbright@label=brightness]"
          << "fslider(0,1,0.1)[@out=cont@handle=hcont@label=contrast]"
          << "fslider(0,1,0.1)[@out=sat@handle=hsat@label=saturation]"
          << "fslider(0,1,0.1)[@out=hue@handle=hhue@label=hue]"
          << "combo(MPEG-1,MOTION-JPEG,MPEG-4.2,MPEG-4.3,MPEG-4,H263,H263I,FLV1)[@label=codec@handle=Codec@out=codec_]"
          << "button(start videocapture)[@out=startc@handle=startc_]"
          << "button(stop videocapture)[@out=stopc@handle=stopc_]"
          << "button(info)[@out=info@handle=info_]"
          << "button(snapshot)[@out=snap@handle=snapshot]"
          );
  gui.show();

  gui["hbright"].registerCallback(new GUI::Callback(set_brightness));
  gui["hcont"].registerCallback(new GUI::Callback(set_contrast));
  gui["hsat"].registerCallback(new GUI::Callback(set_saturation));
  gui["hhue"].registerCallback(new GUI::Callback(set_hue));
  gui["hsize"].registerCallback(new GUI::Callback(set_size));
  gui["startc_"].registerCallback(new GUI::Callback(startcap));
  gui["stopc_"].registerCallback(new GUI::Callback(stopcap));
  gui["info_"].registerCallback(new GUI::Callback(printAllProperties));
  gui["snapshot"].registerCallback(new GUI::Callback(takeSnapshot));
}

int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(device) -file|-f(destinationfile)",init,run).exec();
}

