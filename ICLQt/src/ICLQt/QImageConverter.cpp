/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/QImageConverter.cpp                    **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/QImageConverter.h>
#include <QtGui/QImage>
#include <QtCore/QVector>
#include <ICLCore/Img.h>
#include <ICLCore/CCFunctions.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace qt{
    static const icl8u pseudo_colors[3][256]={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,6,9,12,15,19,22,25,28,31,35,38,41,44,
                                               48,51,54,57,60,64,67,70,73,77,80,83,86,90,93,96,99,102,106,109,112,115,
                                               119,122,125,128,131,135,138,141,144,148,151,154,157,160,164,167,170,173,
                                               177,180,183,186,190,193,196,199,202,206,209,212,215,219,222,225,228,231,
                                               235,238,241,244,248,251,254,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,250,246,241,237,232,228,
                                               223,218,214,209,205,200,196,191,187,182,178,173,168,164,159,155,150,146,
                                               141,137,132,128,128},
                                              {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8,12,
                                               17,21,25,29,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,
                                               108,112,116,120,124,128,133,136,141,144,149,152,157,160,165,168,173,176,
                                               181,184,189,192,197,200,205,208,213,216,221,224,229,232,237,240,245,248,
                                               253,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,252,248,245,
                                               241,237,234,230,226,222,219,215,211,208,204,200,196,193,189,185,182,178,
                                               174,171,167,163,159,156,152,148,145,141,137,134,130,126,122,119,115,111,
                                               108,104,100,96,93,89,85,82,78,74,71,67,63,59,56,52,48,45,41,37,34,30,26,
                                               22,19,15,11,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                              {132,137,141,146,150,155,159,164,168,173,177,182,187,191,196,200,205,209,
                                               214,218,223,227,232,237,241,246,250,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                               255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,251,248,244,
                                               241,238,235,231,228,225,222,219,215,212,209,206,202,199,196,193,190,186,
                                               183,180,177,173,170,167,164,160,157,154,151,148,144,141,138,135,131,128,
                                               125,122,119,115,112,109,106,102,99,96,93,90,86,83,80,77,73,70,67,64,60,57,
                                               54,51,48,44,41,38,35,31,28,25,22,19,15,12,9,6,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                               0,0,0,0}};
    template<class T>
    void img_to_qimage(const Img<T> *src, QImage *&dst, bool useSpeudoColors){
      // {{{ open
  
    ICLASSERT_RETURN(src);
    static QVector<QRgb> palette;
    static QVector<QRgb> pc_palette;
    if(!palette.size()){ 
      for(int i=0;i<256;++i){
        palette.push_back(qRgb(i,i,i));
        pc_palette.push_back(qRgb(pseudo_colors[0][i], pseudo_colors[1][i], pseudo_colors[2][i] ));
      }
    }
    int w = src->getWidth();
    int h = src->getHeight();
    if(!dst){
      dst = new QImage(1,1,QImage::Format_Indexed8);
    }
    dst->setColorTable( useSpeudoColors ? pc_palette : palette );

  
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
    void qimage_to_img(const QImage *src, Img<T> **ppDst, bool useSpeudoColors){
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

    QImageConverter::QImageConverter():m_usePC(false){
      // {{{ open

      for(int i=0;i<5;i++){
        m_aeStates[i]=undefined;
        m_apoBuf[i]=0;
      }
      m_poQBuf = 0;
      m_eQImageState=undefined;
    }

    // }}}

    QImageConverter::QImageConverter(const ImgBase *image):m_usePC(false){
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

    QImageConverter::QImageConverter(const QImage *qimage):m_usePC(false){
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
              img_to_qimage(m_apoBuf[i]->asImg<icl8u>(), m_poQBuf, m_usePC); 
              break;
            }
            case depth16s: img_to_qimage(m_apoBuf[i]->asImg<icl16s>(), m_poQBuf, m_usePC); break;
            case depth32s: img_to_qimage(m_apoBuf[i]->asImg<icl32s>(), m_poQBuf, m_usePC); break;
            case depth32f: img_to_qimage(m_apoBuf[i]->asImg<icl32f>(), m_poQBuf, m_usePC); break;
            case depth64f: img_to_qimage(m_apoBuf[i]->asImg<icl64f>(), m_poQBuf, m_usePC); break;
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

    void QImageConverter::setUseSpeudoColors(bool use){
      m_usePC = use;
    }

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
        qimage_to_img(m_poQBuf,reinterpret_cast<Img<T>**>(&m_apoBuf[d]), m_usePC);
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
  } // namespace qt

 
} // namespace icl
