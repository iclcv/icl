#include <iclLabelHandle.h>
#include <QPainter>
#include <QLabel>

namespace icl{
  void LabelHandle::operator=(const std::string &text){
    lab()->setText(text.c_str());
    lab()->update();
  }
  void LabelHandle::operator=(const QString &text){
    lab()->setText(text);
    lab()->update();
  } 
  void LabelHandle::operator=(const char *text){
    lab()->setText(QString(text));
    lab()->update();
  }
  void LabelHandle::operator=(int num){
    lab()->setNum(num);
    lab()->update();
  }
  void LabelHandle::operator=(double num){
    lab()->setNum(num);
    lab()->update();
  }
}
