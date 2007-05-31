#include <iclDrawWidget.h>
#include <iclCamThread.h>
#include <iclImg.h>
#include <iclImgBase.h>
#include <iclPWCGrabber.h>
#include <iclCC.h>
#include <iclImgChannel.h>
#include <QApplication>
#include <QThread>
#include <QGridLayout>
#include <QPushButton>
#include <iclTimer.h>
#include <QTimer>

using namespace icl;
using namespace std;


//int main(int nArgs, char **ppcArg){
int main(int nArgs, char *ppcArg[]){
  QApplication a(nArgs,ppcArg);
  CamThread x(0);
  CamThread y(1);
  QTimer t;
  QObject::connect(&t,SIGNAL(timeout()),&x,SLOT(update()));
  QObject::connect(&t,SIGNAL(timeout()),&y,SLOT(update()));
  
  t.start(100);
  
  return a.exec();
}
