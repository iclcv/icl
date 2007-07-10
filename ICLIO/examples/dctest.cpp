#include <QApplication>
#include <QTimer>
#include <iclCamThread.h>
#include <iclDCGrabber.h>
#include <iclProgArg.h>

using namespace std;

Size size;
format fmt;
string mode;

Grabber *initGrabber(Grabber *g){
  g->setDesiredSize(size);
  g->setDesiredFormat(fmt);
  g->setProperty("format",mode);
  return g;
}

int main(int n, char **ppc){
  QApplication app(n,ppc);
  pa_init(n,ppc,"-cams(1) -cam(1) -size(1) -format(1) -mode(1)");
  
  int NCAMS = pa_subarg("-cams",0,int(1));
  if(NCAMS <= 0 || NCAMS > 3){
    NCAMS = 1;
  }
  int USECAM = pa_subarg("-cam",0,int(-1));
  
  mode = pa_subarg("-mode",0,std::string("DC1394_VIDEO_MODE_640x480_MONO8@DC1394_FRAMERATE_30"));
  
  size = translateSize(pa_subarg("-size",0,std::string("640x480")));
  if(size == Size::null) size = Size(640,480);
  
  fmt = translateFormat(pa_subarg("-format",0,std::string("rgb")));
  if(fmt == formatMatrix) fmt = formatGray;
  
  int i = USECAM; 
  int j = -1;
  int k = -1;
  QTimer t;
  std::vector<DCDevice> devs = DCGrabber::getDeviceList();
  
  if(USECAM == -1){
    printf("Available Devices: \n");
    for(unsigned int ii=0;ii<devs.size();ii++){
      printf("%2d %s (%s)  Port:%d  Node%d \n",ii,
             devs[ii].getModelID().c_str(),
             devs[ii].getVendorID().c_str(),
             (int)(devs[ii].getPort()),
             (int)(devs[ii].getNode()) );
    }
    while(i < 0 || i>=(int)devs.size() ){
      printf("chose device:\n>>: ");
      scanf("%d",&i);
    }

    if(NCAMS>1){
      printf("chose 2nd device (or -1 for no 2nd device) :\n>>: ");
      scanf("%d",&j);
    }
    
    if(NCAMS>2){
      printf("chose 3rd device (or -1 for no 3rd device) :\n>>: ");
      scanf("%d",&k);
    }
  }else{
    if(USECAM < (int)devs.size() && USECAM >=0){
      printf("using this camera: \n");
      printf("%2d %s (%s)  Port:%d  Node%d \n",USECAM,
             devs[USECAM].getModelID().c_str(),
             devs[USECAM].getVendorID().c_str(),
             (int)(devs[USECAM].getPort()),
             (int)(devs[USECAM].getNode()) );
    }else{
      printf("camera idx %d not available possible indices are 0-%d \n",USECAM,((int)devs.size())-1);
      exit(0);
    }
  }


  CamThread ct(initGrabber(new DCGrabber(devs[i])));
  ct.setGeomery(Rect(200,200,640,480));
  QObject::connect(&t,SIGNAL(timeout()), &ct, SLOT(update()));
  
  if(j!=i && j>=0 && j < (int)devs.size()){
    CamThread ct2(initGrabber(new DCGrabber(devs[j])));    
    ct2.setGeomery(Rect(600,200,640,480));
    QObject::connect(&t,SIGNAL(timeout()), &ct2, SLOT(update()));
    if(k!=i && k!=j && k >=0 && k<(int)devs.size()){
        CamThread ct3(initGrabber(new DCGrabber(devs[k])));    
        ct3.setGeomery(Rect(600,200,640,480));
        QObject::connect(&t,SIGNAL(timeout()), &ct3, SLOT(update()));
        t.start(20);
        app.exec();
    }else{
      t.start(20);
      app.exec();
    }
  }else{
    t.start(20);
    app.exec();
  }

  

  
  return 0;
}

