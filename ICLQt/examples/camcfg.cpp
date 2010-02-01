#include <ICLQt/CamCfgWidget.h>
#include <QApplication>
#include <ICLUtils/ProgArg.h>

using namespace icl;
using namespace std;

int main(int n, char **ppc){
  paex
  ("-r","resets the dc bus on startup")
  ("-800","if this flag is set, application trys dc devices to setup\n"
   "in ieee1394-B mode with 800MBit iso transfer rate")
  ("-no-unicap","disable unicap support")
  ("-no-dc","disable dc grabber support")
  ("-no-pwc","disable pwc grabber support");
  
  painit(n,ppc,"-use-IEEE1394-B|-800 -reset-bus|-r -no-unicap|-u -no-dc|-d -no-pwc|-p -no-sr|-s");
  QApplication a(n,ppc);
  
  CamCfgWidget::CreationFlags flags(pa("-800")?800:400,
                                    pa("-r"),
                                    pa("-no-unicap"),
                                    pa("-no-dc"),
                                    pa("-no-pwc"),
                                    pa("-no-sr"));
  
  
  CamCfgWidget w(flags);

  w.setGeometry(50,50,800,800);
  w.setWindowTitle("camcfg (ICL Camera Configuration Tool)");
  w.show();

  
  return a.exec();
}
