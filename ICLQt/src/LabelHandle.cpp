#include <ICLQt/LabelHandle.h>
#include <QPainter>
#include <QLabel>

namespace icl{
  void LabelHandle::operator=(const std::string &text){
    lab()->setText(text.c_str());
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(const QString &text){
    lab()->setText(text);
    lab()->updateFromOtherThread();
  } 
  void LabelHandle::operator=(const char *text){
    lab()->setText(QString(text));
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(int num){
    lab()->setNum(num);
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(double num){
    lab()->setNum(float(num));
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator+=(const std::string &text){
    lab()->setText(lab()->text() + text.c_str());
    lab()->updateFromOtherThread();
  }
}
