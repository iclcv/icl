/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ColorLabel.cpp                               **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/ColorLabel.h>
#include <QtGui/QPainter>

namespace icl{

  ColorLabel::ColorLabel(Color4D &color, bool useAlpha, QWidget *parent):
    ThreadedUpdatableWidget(parent),m_color(color),m_hasAlpha(useAlpha){}
    

    /// reimplemented drawin function (draw the current text centered)
  void ColorLabel::paintEvent(QPaintEvent *evt){
    m_oMutex.lock();
    QColor c(m_color[0],m_color[1],m_color[2]);
    m_oMutex.unlock();

    float a = m_color[3];
    
    QPainter p(this);
    p.setBrush(c);
    p.setPen(QColor(255,255,255,128));
    
    p.drawRect(QRect(2,2,width()-4,height()-4));
    
    if(m_hasAlpha && (a != 255) ){
      if((m_color[0]+m_color[1]+m_color[2]) > 1.5 * 255){
        p.setPen(QColor(0,0,0));
      }else{
        p.setPen(QColor(255,255,255));
      }
      
      p.drawText(QRect(2,2,width()-4,height()-4),Qt::AlignCenter|Qt::Horizontal,QString::number((int)(a*100./256.)) + "%");;
    }
  }

    /// sets new color rgb
  void ColorLabel::setColor(const Color &color){
    m_oMutex.lock();
    m_color = Color4D(color[0],color[1],color[2],0);
    m_oMutex.unlock();
    updateFromOtherThread();
  }

  /// sets new color rgba
  void ColorLabel::setColor(const Color4D &color){
    m_oMutex.lock();
    m_color = Color4D(color[0],color[1],color[2],color[3]);
    m_oMutex.unlock();
    updateFromOtherThread();
  }

    /// returns current color
  Color ColorLabel::getRGB() const{
    return Color(m_color[0],m_color[1],m_color[2]);
  }
    
    /// returns current rgba color
  Color4D ColorLabel::getRGBA() const{
    return m_color;
  }
    
    /// returns current red value
  int ColorLabel::getRed() const{
    return m_color[0];
  }

    /// returns current green value
  int ColorLabel::getGreen() const{
    return m_color[1];
  }

    /// returns current blue value
  int ColorLabel::getBlue() const{
    return m_color[2];
  }

    /// return current alpha value
  int ColorLabel::getAlhpa() const{
    return m_color[3];
  }
    
    /// returns wheter internal color uses alpha value
  bool ColorLabel::hasAlpha() const{
    return m_hasAlpha;
  }
}


