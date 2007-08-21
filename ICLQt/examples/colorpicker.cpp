#include <iclProgArg.h>
#include <iclQuick.h>
#include <iclGUI.h>
#include <iclMouseInteractionReceiver.h>
#include <iclFileReader.h>
#include <iclThread.h>
#include <iclCC.h>
#include <iclMutex.h>

GUI *gui;
ICLDrawWidget *widget;
FileReader *grabber;
bool *running;
int *sleeptime;
string *colormode;
Mutex mutex;


struct XC{
  XC(float a=0,float b=0,float d=0){
    c[0] = a;
    c[1] = b;
    c[2] = d;
  }
  XC(const float *f){
    c[0] = f[0];
    c[1] = f[1];
    c[2] = f[2];
  }
  float c[3];
  float &operator[](int i){return c[i]; }
};

vector<XC> colorbuffer;

  
class Handler : public MouseInteractionReceiver{
  virtual void processMouseInteraction(MouseInteractionInfo *info){
    mutex.lock();
    if(info->type == MouseInteractionInfo::pressEvent){
      std::vector<float> &c = info->color; 
      if(c.size() == 1){
        printf("color: %d \n",(int)c[0]);
      }else if(c.size() > 2){
        // Assertion ; input type is rgb!!
        if(*colormode == "rgb"){
          printf("rgb: %d %d %d \n",(int)c[0],(int)c[1],(int)c[2]);          
          colorbuffer.push_back(XC(c[0],c[1],c[2]));
        }else if (*colormode == "hls"){
          icl32f hls[3];
          cc_util_rgb_to_hls (c[0],c[1],c[2],hls[0],hls[1],hls[2]);
          printf("hls: %d %d %d \n",(int)hls[0],(int)hls[1],(int)hls[2]);      
          colorbuffer.push_back(XC(hls));
        }
        else if (*colormode == "yuv"){
          icl32s yuv[3];
          cc_util_rgb_to_yuv ((int)c[0],(int)c[1],(int)c[2],yuv[0],yuv[1],yuv[2]);
          printf("yuv: %d %d %d \n",yuv[0],(int)yuv[1],(int)yuv[2]);          
          colorbuffer.push_back(XC(yuv[0],yuv[1],yuv[2]));
        }
        else if(*colormode == "gray"){
          printf("gray: %d \n",(int)((c[0]+c[1]+c[2])/3));
        }
        else{
          printf("error color mode is (%s) \n",colormode->c_str());
        }
      }
    }
    mutex.unlock();
  }  
};

void reset_list(){
  mutex.lock();
  colorbuffer.clear();
  printf("cleared! \n----------------------------------------\n");
  mutex.unlock();
}
void calc_mean(){
  mutex.lock();
  if(!colorbuffer.size()){
    mutex.unlock();
    return;
  }
  XC xM,xV;
  for(unsigned int i=0;i<colorbuffer.size();i++){
    xM[0] += colorbuffer[i][0];
    xM[1] += colorbuffer[i][1];
    xM[2] += colorbuffer[i][2];
  }
  xM[0] /= colorbuffer.size();
  xM[1] /= colorbuffer.size();  
  xM[2] /= colorbuffer.size();

  for(unsigned int i=0;i<colorbuffer.size();i++){
    xV[0] += pow(colorbuffer[i][0]-xM[0],2);
    xV[1] += pow(colorbuffer[i][1]-xM[1],2);
    xV[2] += pow(colorbuffer[i][2]-xM[2],2);
  }

  xV[0] /= colorbuffer.size();
  xV[1] /= colorbuffer.size();  
  xV[2] /= colorbuffer.size();


  printf("mean   = %3d %3d %3d \n",(int)xM[0],(int)xM[1],(int)xM[2]);
  printf("stddev = %3d %3d %3d \n",(int)sqrt(xV[0]),(int)sqrt(xV[1]),(int)sqrt(xV[2]));
  printf("-------------------------------------------------\n");  

  mutex.unlock();
}

void run_func(){
  while(!(*running)) usleep(100*1000);
  const ImgBase *image = grabber->grab();
  widget->setImage(image);
  widget->update();
  usleep(1000*(*sleeptime));
}

class Runner : public Thread{
  virtual void run(){
    while(1){
      run_func();
    } 
  } 
};




int main(int n,char **ppc){
  pa_explain("-input","input-file or filepattern (madatory)");
  pa_init(n,ppc,"-input(1)");
  if(!pa_defined("-input")){ pa_usage("please define input file!"); exit(0); }
  
  grabber = new FileReader(pa_subarg("-input",0,string("no input file defined!")));
  
  QApplication app(n,ppc);
  
  gui = new GUI;
  (*gui) << "draw[@label=image@inp=image@size=32x24]";
  (*gui) << "togglebutton(Run!,Stop!)[@out=run]";
  (*gui) << ( GUI("hbox") 
              << "slider(0,400,10)[@out=sleep@label=sleeptime]" 
              << "combo(!rgb,hls,gray,yuv)[@out=colormode@label=colormode]" 
              << "button(Reset List)[@out=reset]"
              << "button(Calculate Mean)[@out=calc]"
              );
  
  (*gui).show();
  
  widget = gui->getValue<ICLDrawWidget*>("image");
  running = &gui->getValue<bool>("run");
  sleeptime = &gui->getValue<int>("sleep");
  colormode = &gui->getValue<string>("colormode");
  gui->getValue<ButtonHandle>("reset").registerCallback(reset_list);
  gui->getValue<ButtonHandle>("calc").registerCallback(calc_mean);
  
  
  
  Handler h;
  QObject::connect(widget,SIGNAL(mouseEvent(MouseInteractionInfo*)),&h,SLOT(mouseInteraction(MouseInteractionInfo*)));
  Runner r;
          
          
  r.start();
  
  
  
  return app.exec();
}
