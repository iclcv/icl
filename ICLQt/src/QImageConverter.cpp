/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/QImageConverter.cpp                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLQt/QImageConverter.h>
#include <QtGui/QImage>
#include <QtCore/QVector>
#include <ICLCore/Img.h>
#include <ICLCC/CCFunctions.h>


namespace icl{

template<class T>
void img_to_qimage(const Img<T> *src, QImage *&dst){
  // {{{ open

  ICLASSERT_RETURN(src);
  static QVector<QRgb> palette;
  if(!palette.size()){ for(int i=0;i<255;++i)palette.push_back(qRgb(i,i,i)); }
  int w = src->getWidth();
  int h = src->getHeight();
  if(!dst){
    dst = new QImage(1,1,QImage::Format_Indexed8);
    dst->setColorTable(palette);
  }

  if(src->getChannels() == 1){
    if(dst->width() != w || dst->height() != h || dst->format() != QImage::Format_Indexed8){
      delete dst;
      dst = new QImage(w,h,QImage::Format_Indexed8);
      dst->setColorTable(palette);
    }
  }else{
    if(dst->width() != w || dst->height() != h || dst->format() != QImage::Format_RGB32){
      delete dst;
      dst = new QImage(w,h,QImage::Format_RGB32);
    }
  }
  
  static Img<T> bBuf(Size(1,1),1),wBuf(Size(1,1),1);
  std::vector<T*> channelVec;
  
  switch(src->getChannels()){
    case 1: 
      planarToInterleaved<T,icl8u>(src, dst->bits());
      break;
    case 2:{
      if(wBuf.getDim() < src->getDim()){
        wBuf.setSize(src->getSize());
        wBuf.clear(-1,255,false);
        bBuf.setSize(src->getSize());
      }
      channelVec.push_back(const_cast<T*>(src->getData(0)));
      channelVec.push_back(bBuf.getData(0));
      channelVec.push_back(const_cast<T*>(src->getData(1)));
      channelVec.push_back(wBuf.getData(0));
      const Img<T> tmp(src->getSize(),4,channelVec);
      planarToInterleaved<T,icl8u>(&tmp,dst->bits());
      break;
    }
    case 3:{
      if(wBuf.getDim() < src->getDim()){
        wBuf.setSize(src->getSize());
        wBuf.clear(-1,255,false);
      }
      channelVec.push_back(const_cast<T*>(src->getData(2)));
      channelVec.push_back(const_cast<T*>(src->getData(1)));
      channelVec.push_back(const_cast<T*>(src->getData(0)));
      channelVec.push_back(wBuf.getData(0));
      const Img<T> tmp(src->getSize(),4,channelVec);

      planarToInterleaved<T,icl8u>(&tmp,dst->bits());
      break;
    }
    default:{
      channelVec.push_back(const_cast<T*>(src->getData(2)));
      channelVec.push_back(const_cast<T*>(src->getData(1)));
      channelVec.push_back(const_cast<T*>(src->getData(0)));
      channelVec.push_back(const_cast<T*>(src->getData(3)));
      const Img<T> tmp(src->getSize(),4,channelVec);
      planarToInterleaved<T,icl8u>(&tmp,dst->bits());
      break;
    }
  }
}

  // }}}

template<class T>
void qimage_to_img(const QImage *src, Img<T> **ppDst){
  // {{{ open
  Img<T> *&dst = *ppDst;
  
  ICLASSERT_RETURN(src);
  ICLASSERT_RETURN(!src->isNull());
  if(!dst) dst = new Img<T>(Size(1,1),1);
  
  if(src->format() == QImage::Format_Indexed8){
    dst->setFormat(formatGray);
  }else{
    dst->setFormat(formatRGB);
  }
  dst->setSize(Size(src->width(),src->height()));
  //append temporarily a white image channel
  static Img<T> wBuf(Size(1,1),formatGray);
  if(wBuf.getDim() < src->width()*src->height()){
    wBuf.setSize(Size(src->width(),src->height()));
  }
  std::vector<T*> vecChannels;
  vecChannels.push_back(dst->getData(2));
  vecChannels.push_back(dst->getData(1));
  vecChannels.push_back(dst->getData(0));
  vecChannels.push_back(wBuf.getData(0));
  Img<T> tmp(dst->getSize(),4,vecChannels);

  interleavedToPlanar(src->bits(),&tmp);
}

  // }}}

QImageConverter::QImageConverter(){
  // {{{ open

  for(int i=0;i<5;i++){
    m_aeStates[i]=undefined;
    m_apoBuf[i]=0;
  }
  m_poQBuf = 0;
  m_eQImageState=undefined;
}

// }}}

QImageConverter::QImageConverter(const ImgBase *image){
  // {{{ open

  for(int i=0;i<5;i++){
    m_aeStates[i]=undefined;
    m_apoBuf[i]=0;
  }
  m_poQBuf = 0;
  m_eQImageState=undefined;
  setImage(image);
}

// }}}

QImageConverter::QImageConverter(const QImage *qimage){
  // {{{ open

  for(int i=0;i<5;i++){
    m_aeStates[i]=undefined;
    m_apoBuf[i]=0;
  }
  m_poQBuf = 0;
  m_eQImageState=undefined;
  setQImage(qimage);
  
}

// }}}

QImageConverter::~QImageConverter(){
  // {{{ open

  if(m_poQBuf && m_eQImageState != given){
    delete m_poQBuf;
  }
  for(int i=0;i<5;i++){
    if(m_apoBuf[i] && m_aeStates[i] != given){
      delete m_apoBuf[i];
    }
  }
}

// }}}
    

const QImage *QImageConverter::getQImage(){
  // {{{ open

  if(m_eQImageState < 2) return m_poQBuf;
  for(int i=0;i<5;i++){
    if(m_aeStates[i] < 2){
      switch((depth)i){
        case depth8u:{
          img_to_qimage(m_apoBuf[i]->asImg<icl8u>(), m_poQBuf); 
          break;
        }
        case depth16s: img_to_qimage(m_apoBuf[i]->asImg<icl16s>(), m_poQBuf); break;
        case depth32s: img_to_qimage(m_apoBuf[i]->asImg<icl32s>(), m_poQBuf); break;
        case depth32f: img_to_qimage(m_apoBuf[i]->asImg<icl32f>(), m_poQBuf); break;
        case depth64f: img_to_qimage(m_apoBuf[i]->asImg<icl64f>(), m_poQBuf); break;
        default: ICL_INVALID_DEPTH;
      }          
      m_eQImageState = uptodate;
      return  m_poQBuf;
    }
  }
  WARNING_LOG("could not create a QImage because no image/qimage was given yet!");
  return 0;
}

// }}}

const ImgBase *QImageConverter::getImgBase(depth d){
  // {{{ open

  switch(d){
    case depth8u:  return getImg<icl8u>();
    case depth16s: return getImg<icl16s>();
    case depth32s: return getImg<icl32s>();
    case depth32f: return getImg<icl32f>();
    case depth64f: return getImg<icl64f>();
    default:
      ICL_INVALID_DEPTH;
  }
  return 0;
}

// }}}

template<class T>
const Img<T> *QImageConverter::getImg(){
  // {{{ open

  depth d = getDepth<T>();
  if(m_aeStates[d] < 2) return m_apoBuf[d]->asImg<T>();
  // else find a given image
  for(int i=0;i<5;i++){
    if(i!=d && m_aeStates[i] < 2){
      //      m_apoBuf[d] = m_apoBuf[i]->deepCopy(m_apoBuf[d]);
      m_apoBuf[d] = m_apoBuf[i]->convert(m_apoBuf[d]->asImg<T>());
      m_aeStates[d] = uptodate;
      return m_apoBuf[d]->asImg<T>();
    }
  }
  // check if the qimage was given
  if(m_eQImageState < 2){
    qimage_to_img(m_poQBuf,reinterpret_cast<Img<T>**>(&m_apoBuf[d]));
    m_aeStates[d] = uptodate;
    return m_apoBuf[d]->asImg<T>();
  }
  WARNING_LOG("could not create an Image because no image/qimage was given yet!");
  return 0;
}

// }}}

void QImageConverter::setImage(const ImgBase *image){
  // {{{ open

  ICLASSERT_RETURN( image );
  depth d = image->getDepth();

  for(int i=0;i<5;i++){
    if(i==d){
      if(m_apoBuf[i] && m_aeStates[i] != given){
        delete m_apoBuf[i];
      }
      m_aeStates[i] = given;
      m_apoBuf[i] = const_cast<ImgBase*>(image);
    }else{
      if(m_aeStates[i] == given){
        m_apoBuf[i] = 0;
      }
      m_aeStates[i] = undefined;
    }
  }  
  if(m_eQImageState == given){
    m_poQBuf = 0;
  }
  m_eQImageState = undefined;
}

// }}}

void QImageConverter::setQImage(const QImage *qimage){
  // {{{ open

  ICLASSERT_RETURN( qimage );
  ICLASSERT_RETURN( !qimage->isNull() );
  
  for(int i=0;i<5;i++){
    if(m_apoBuf[i] && m_aeStates[i] == given){
      m_apoBuf[i] = 0;
    }
    m_aeStates[i] = undefined;
  }
  if(m_poQBuf && m_eQImageState != given){
    delete m_poQBuf;
  }
  m_poQBuf = const_cast<QImage*>(qimage);
  m_eQImageState = given;
}

// }}}

 
} // namespace icl
