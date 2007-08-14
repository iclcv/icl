#include <iclGUILabel.h>
#include <QPainter>

namespace icl{
  class InternalGUILabel : public QWidget{
  public:
    InternalGUILabel(QString s, QWidget *parent=0):QWidget(parent),m_sText(s){
      setMinimumSize(QSize(10,15));
    }
    void setText(const QString &s){
      m_sText = s;
      update();
    }
    void setNum(int i){
      m_sText = QString::number(i);
      update();
    }
    void setNum(double d){
      m_sText = QString::number(d);
      update();
    }
    void setAlignment(Qt::Alignment a){
      (void)a;
      update();
    }
    virtual void paintEvent(QPaintEvent *e){
      (void)e;
      QPainter p(this);
      p.drawText(QRect(0,0,width(),height()),Qt::AlignCenter,m_sText);
    }
  private:
    QString m_sText;
  };
  



  GUILabel::GUILabel(const std::string &initialText=""):
    m_poGUILabel(new InternalGUILabel(initialText.c_str())){
  }
  
  void GUILabel::operator=(const std::string &text){
    ICLASSERT_RETURN(m_poGUILabel);
    ((InternalGUILabel*)m_poGUILabel)->setText(text.c_str());
    m_poGUILabel->update();
  }
  void GUILabel::operator=(const QString &text){
    ICLASSERT_RETURN(m_poGUILabel);
    ((InternalGUILabel*)m_poGUILabel)->setText(text);
    m_poGUILabel->update();
  } 
  void GUILabel::operator=(const char *text){
    ICLASSERT_RETURN(m_poGUILabel);
    ((InternalGUILabel*)m_poGUILabel)->setText(QString(text));
    m_poGUILabel->update();
  }
  void GUILabel::operator=(int num){
    ICLASSERT_RETURN(m_poGUILabel);
    ((InternalGUILabel*)m_poGUILabel)->setNum(num);
    m_poGUILabel->update();
  }
  void GUILabel::operator=(double num){
    ICLASSERT_RETURN(m_poGUILabel);
    ((InternalGUILabel*)m_poGUILabel)->setNum(num);
    m_poGUILabel->update();
  }
}
