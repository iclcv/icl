#include <QApplication>
#include <QTimer>
#include <iclCamThread.h>
#include <iclDCGrabber.h>


int main(int n, char **ppc){
  QApplication app(n,ppc);
  
  QTimer t;
  std::vector<DCDevice> devs = DCGrabber::getDeviceList();
  
  printf("Available Devices: \n");
  for(unsigned int i=0;i<devs.size();i++){
    printf("%2d %s (%s)  Port:%d  Node%d \n",i,
           devs[i].getModelID().c_str(),
           devs[i].getVendorID().c_str(),
           (int)(devs[i].getPort()),
           (int)(devs[i].getNode()) );
  }

  int i = -1;
  while(i < 0 || i>=(int)devs.size() ){
    printf("chose device:\n>>: ");
    scanf("%d",&i);
  }

  int j = -1;
  printf("chose 2nd device (or -1 for no 2nd device) :\n>>: ");
  scanf("%d",&j);
  

  CamThread ct(new DCGrabber(devs[i]));
  ct.setGeomery(Rect(200,200,640,480));
  QObject::connect(&t,SIGNAL(timeout()), &ct, SLOT(update()));
  
  if(j!=i && j>=0 && j < (int)devs.size()){
    CamThread ct2(new DCGrabber(devs[j]));    
    ct.setGeomery(Rect(600,200,640,480));
    QObject::connect(&t,SIGNAL(timeout()), &ct2, SLOT(update()));
    t.start(20);
    app.exec();
  }else{
    t.start(20);
    app.exec();
  }

  

  
  return 0;
}

