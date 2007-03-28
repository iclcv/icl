#include <iclCamCfgWidget.h>
#include <QApplication>

using namespace icl;
using namespace std;

int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  
  CamCfgWidget w;
  return a.exec();
}
