#include <iclCamCfgWidget.h>
#include <QApplication>
#include <iclProgArg.h>

using namespace icl;
using namespace std;

int main(int nArgs, char **ppcArg){
  pa_explain("-r","resets the dc bus on startup");
  pa_explain("-o","makes DCGrabbers omit doubled frames");
  pa_explain("-800","if this flag is set, application trys dc devices to setup\n"
             "in ieee1394-B mode with 800MBit iso transfer rate");
  pa_init(nArgs,ppcArg,"-800 -r -o");
  QApplication a(nArgs,ppcArg);
  
  CamCfgWidget w(pa_defined("-800")?800:400, pa_defined("-r"), pa_defined("-o"));
  return a.exec();
}
