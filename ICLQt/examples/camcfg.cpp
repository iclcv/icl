#include <iclCamCfgWidget.h>
#include <QApplication>
#include <iclProgArg.h>

using namespace icl;
using namespace std;

int main(int nArgs, char **ppcArg){
  pa_explain("-r","resets the dc bus on startup");
  pa_explain("-800","if this flag is set, application trys dc devices to setup\n"
             "in ieee1394-B mode with 800MBit iso transfer rate");
  pa_explain("-no-unicap","disable unicap support");
  pa_explain("-no-dc","disable dc grabber support");
  pa_explain("-no-pwc","disable pwc grabber support");
  pa_init(nArgs,ppcArg,"-800 -r -no-unicap -no-dc -no-pwc");
  QApplication a(nArgs,ppcArg);
  
  CamCfgWidget::CreationFlags flags(pa_defined("-800")?800:400,
                                    pa_defined("-r"),
                                    pa_defined("-no-unicap"),
                                    pa_defined("-no-dc"),
                                    pa_defined("-no-pwc"));
  
  
  CamCfgWidget w(flags);

  w.setGeometry(50,50,800,800);
  w.setWindowTitle("camcfg (ICL Camera Configuration Tool)");

  
  return a.exec();
}
