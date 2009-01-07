#include "iclImgParamWidget.h"
#include "iclBorderBox.h"
#include <iclTypes.h>
#include <iclSize.h>
#include <iclCore.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <string>



using namespace std;

namespace icl{
  
  ImgParamWidget::ImgParamWidget(QWidget *parent):QWidget(parent){
    // {{{ open

    m_poLayout = new QHBoxLayout;
    
    m_poSizeCombo = new QComboBox;
    m_poDepthCombo = new QComboBox;
    m_poFormatCombo = new QComboBox;
    
    m_poSizeCombo->addItem("-grabber-");
    m_poSizeCombo->addItem("160x120");
    m_poSizeCombo->addItem("320x240");
    m_poSizeCombo->addItem("640x480");
    m_poSizeCombo->addItem("800x600");
    m_poSizeCombo->addItem("1024x768");
    m_poSizeCombo->addItem("1600x1200");
    
    for(format f=formatGray; f<=formatLast; f=(format)(f+1)){
      m_poFormatCombo->addItem(translateFormat(f).c_str());
    }
    
    for(icl::depth d=icl::depth8u; d<=depthLast; d=(icl::depth)(d+1)){
      m_poDepthCombo->addItem(translateDepth(d).c_str());
    }
    
    m_poLayout->addWidget(new BorderBox("size",m_poSizeCombo,this));
    m_poLayout->addWidget(new BorderBox("depth",m_poDepthCombo,this)); 
    m_poLayout->addWidget(new BorderBox("format",m_poFormatCombo,this));
    
    connect(m_poSizeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(sizeChanged(const QString&)));
    connect(m_poDepthCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(depthChanged(const QString&)));
    connect(m_poFormatCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(formatChanged(const QString&)));
    
    setLayout(m_poLayout);
    
    setup(-1,-1,(int)depth8u,(int)formatRGB);
  }

  // }}}

  namespace{
    QString sizeToStr(const Size &size){
      // {{{ open
      if(size == Size(-1,-1)) return "-grabber-";
      static char buf[100];
      sprintf(buf,"%dx%d",size.width,size.height);
      return buf;
    }
    
    // }}}
    Size strToSize(const QString &s){
      // {{{ open
      if(s == "-grabber-") return Size(-1,-1);
      return Size(s.section('x',0,0).toInt(), s.section('x',1,1).toInt());
    }
    
    // }}}
    int getIndex(const QString &str, QComboBox *box){
      // {{{ open

      for(int i=0;i<box->count();i++){
        if(str == box->itemText(i)){
          return i;
        }
      }
      return -1;
    }

    // }}}
  }

  void ImgParamWidget::doEmitState(){
    // {{{ open

    emit somethingChanged(m_iWidth,m_iHeight, m_iDepth, m_iFormat);
  }

  // }}}
  
  void ImgParamWidget::getParams(int &width, int &height, int &d, int &fmt)const{
    // {{{ open

    width = m_iWidth;
    height = m_iHeight;
    d = m_iDepth;
    fmt = m_iFormat;
  }

  // }}}

  void ImgParamWidget::setup(int width, int height, int dth, int fmt){
    // {{{ open

    m_iWidth = width;
    m_iHeight = height;
    m_iDepth = dth;
    m_iFormat = fmt;
    
    QString sizeText = sizeToStr(Size(width,height));
    int sizeIdx = getIndex(sizeText,m_poSizeCombo);
    if(sizeIdx == -1){
      ERROR_LOG("invalid size \"" << width << "x" << height << "\"");
    }else{
      m_poSizeCombo->setCurrentIndex(sizeIdx);
    }
    
    QString depthText = translateDepth((icl::depth)dth).c_str();
    int depthIdx = getIndex(depthText,m_poDepthCombo);
    if(depthIdx == -1){
      ERROR_LOG("invalid depth \"" << depthText.toLatin1().data() << "\""); 
    }else{
      m_poDepthCombo->setCurrentIndex(depthIdx);
    }

    QString formatText = translateFormat((format)fmt).c_str();
    int formatIdx = getIndex(formatText,m_poFormatCombo);
    if(formatIdx == -1){
      ERROR_LOG("invalid format \"" << formatText.toLatin1().data() << "\""); 
    }else{
      m_poFormatCombo->setCurrentIndex(formatIdx);
    }
    
  }

  // }}}
  
  void ImgParamWidget::sizeChanged(const QString &val){
    Size s = strToSize(val);
    m_iWidth = s.width;
    m_iHeight = s.height;
    emit somethingChanged(m_iWidth,m_iHeight, m_iDepth, m_iFormat);
  }
  void ImgParamWidget::formatChanged(const QString &val){
    format fmt = translateFormat(string(val.toLatin1().data()));
    m_iFormat = (int)fmt;
    emit somethingChanged(m_iWidth,m_iHeight, m_iDepth, m_iFormat);
  }
  void ImgParamWidget::depthChanged(const QString &val){
    icl::depth d = translateDepth(string(val.toLatin1().data()));
    m_iDepth = (int)d;
    emit somethingChanged(m_iWidth,m_iHeight, m_iDepth, m_iFormat);
  }
}


