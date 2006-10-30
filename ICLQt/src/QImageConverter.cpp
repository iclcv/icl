#include "QImageConverter.h"

#include <QImage>
#include <QVector>
#include "Img.h"
#include "ICLCore.h"
#include <map>

#ifdef WITH_IPP_OPTIMIZATION
#include <ippi.h>
#include <ipps.h>
#endif

namespace icl{

  // {{{ defines

#define IDX8U 0
#define IDX32F 1
#define IDXQ 2

#define STU m_eStates[IDX8U]  
#define STF m_eStates[IDX32F]  
#define STQ m_eStates[IDXQ]  

#define CHECK ICLASSERT_RETURN_VAL( m_poImgBuffer8u || m_poImgBuffer32f || m_poQImageBuffer , 0); \
              ICLASSERT_RETURN_VAL( STU == given || STF == given || STQ == given , 0);

  // }}}
  // {{{ Constructors / Destructor

  QImageConverter::QImageConverter():
    // {{{ open

    m_poImgBuffer8u(0),
    m_poImgBuffer32f(0),
    m_poQImageBuffer(0){
    STU=STF=STQ=undefined;
  }

  // }}}
  
  QImageConverter::QImageConverter(const ImgI *image):
    // {{{ open

    m_poImgBuffer8u(0),
    m_poImgBuffer32f(0),
    m_poQImageBuffer(0)
  {
    setImage(image);
  }

  // }}}
  
  QImageConverter::QImageConverter(const QImage *qimage):
    // {{{ open

    m_poImgBuffer8u(0),
    m_poImgBuffer32f(0),
    m_poQImageBuffer(0)
  {
    setQImage(qimage);
  }

  // }}}

  QImageConverter::~QImageConverter(){
    // {{{ open

    if(m_poImgBuffer8u) delete m_poImgBuffer8u;
    if(m_poImgBuffer32f) delete m_poImgBuffer32f;
    if(m_poQImageBuffer) delete m_poQImageBuffer;
  }

  // }}}

  // }}}
  // {{{ image-getter

  const QImage *QImageConverter::getQImage(){
    // {{{ open

    CHECK;
    if(STQ <= uptodate) return m_poQImageBuffer;
    if(STU <= uptodate) return img8uToQImage(m_poImgBuffer8u,m_poQImageBuffer);
    else return img32fToQImage(m_poImgBuffer32f,m_poQImageBuffer);
  }

  // }}}

  const ImgI *QImageConverter::getImage(){
    // {{{ open

    CHECK;
    if(STU <= uptodate) return m_poImgBuffer8u;
    if(STF <= uptodate) return m_poImgBuffer32f;
    else return qimageToImg8u(m_poQImageBuffer,m_poImgBuffer8u);
  }

  // }}}

  const Img8u *QImageConverter::getImg8u(){
    // {{{ open

    CHECK;
    if(STU <= uptodate) return m_poImgBuffer8u;
    if(STF <= uptodate) return m_poImgBuffer32f->deepCopy(m_poImgBuffer8u)->asImg<icl8u>();
    else return qimageToImg8u(m_poQImageBuffer,m_poImgBuffer8u);
  }

  // }}}

  const Img32f *QImageConverter::getImg32f(){
    // {{{ open

    CHECK;
    if(STF <= uptodate) return m_poImgBuffer32f;
    if(STU <= uptodate) return m_poImgBuffer8u->deepCopy(m_poImgBuffer32f)->asImg<icl32f>();
    else return qimageToImg32f(m_poQImageBuffer,m_poImgBuffer32f);
  }

  // }}}

  // }}}
  // {{{ image-setter

  void QImageConverter::setImage(const ImgI *image){
    // {{{ open

    ICLASSERT_RETURN( image );

    if(image->getDepth() == depth8u){
      if(m_poImgBuffer8u && STU != given) delete m_poImgBuffer8u;
      STU = given;
      STF = undefined;
      STQ = undefined;
      m_poImgBuffer8u = image->asImg<icl8u>();
    }else{
      if(m_poImgBuffer32f && STF != given) delete m_poImgBuffer32f;
      STF = given;
      STU = undefined;
      STQ = undefined;
      m_poImgBuffer32f = image->asImg<icl32f>();
    }
  }

  // }}}

  void QImageConverter::setQImage(const QImage *qimage){
    // {{{ open

    ICLASSERT_RETURN( qimage );
    STU=STF=undefined;
    if(m_poQImageBuffer && STQ != given) delete m_poQImageBuffer;
    STQ = given;
    m_poQImageBuffer = const_cast<QImage*>(qimage);
    
  } 

  // }}}

  // }}}

  // {{{ global utility functions

  void ensureQImage(QImage *&qimage, int w, int h, QImage::Format f){
    // {{{ open

    static QVector<QRgb> palette;
    if(!palette.size()){ for(int i=0;i<255;++i)palette.push_back(qRgb(i,i,i)); }
    
    if(!qimage){
      qimage = new QImage(w,h,f);
      if(f == QImage::Format_Indexed8){
        qimage->setColorTable(palette);
      }
    }
    else{
      if(qimage->width() != w || qimage->height() != h || qimage->format() != f){
        *qimage = QImage(w,h,f);
        if(f == QImage::Format_Indexed8){
          qimage->setColorTable(palette);
        }
      }
    }    
  }

  // }}}
  icl32f *getBuffer32f(unsigned int size, int id=0){
    // {{{ open
    if(!id){
      static std::vector<icl32f> buffer;
      if(buffer.size() < size) buffer.resize(size);
      return &(buffer[0]);
    }else{
      static std::map<int,std::vector<icl32f> > buffer;
      std::vector<icl32f> &ref = buffer[id];
      if(ref.size() < size) ref.resize(size);
      return &(ref[0]);
    }
  }

  // }}}
  icl8u *getBuffer8u(unsigned int size,int id=0){
    // {{{ open
    if(!id){
      static std::vector<icl8u> buffer;
      if(buffer.size() < size) buffer.resize(size);
      return &(buffer[0]);
    }else{
      static std::map<int,std::vector<icl8u> > buffer;
      std::vector<icl8u> &ref = buffer[id];
      if(ref.size() < size) ref.resize(size);
      return &(ref[0]);
    }
  }

  // }}}

  // }}}

  const QImage *QImageConverter::img32fToQImage(Img32f *image, QImage *&qimage){
    // {{{ open
    printf("not yet implemented \n");
    
    return qimage;
  }

  // }}}

  const QImage *QImageConverter::img8uToQImage(Img8u *image, QImage *&qimage){
    // {{{ open

    // {{{ variable declaration

    ICLASSERT_RETURN_VAL( image && image->getChannels() > 0, 0);
   
    int c = image->getChannels();
    int w = image->getWidth();
    int h = image->getHeight();
    int dim = c*w*h;
    const Size &s = image->getSize();
    int step = image->getLineStep();
    const icl8u *ap[4];
    // }}}
    
#ifdef WITH_IPP_OPTIMIZATION
    switch(image->getChannels()){
      case 1:
        ensureQImage(qimage,w,h,QImage::Format_Indexed8);
        icl::copy(image->getData(0),image->getData(0)+w*h,qimage->bits());
        break;
      case 2:
        ap[0] = image->getData(0); //using b and r channel
        ap[1] = getBuffer8u(dim,0);
        ap[2] = image->getData(1);
        ap[3] = getBuffer8u(dim,1);
        ensureQImage(qimage,w,h,QImage::Format_RGB32);
        ippiCopy_8u_P4C4R(ap,step,qimage->bits(),4*step,s);
        break;
      case 3:
        ap[0] = image->getData(2); // qt byter order bgra
        ap[1] = image->getData(1);
        ap[2] = image->getData(0);
        ap[3] = getBuffer8u(dim,0);
        ensureQImage(qimage,w,h,QImage::Format_RGB32);
        ippiCopy_8u_P4C4R(ap,step,qimage->bits(),4*step,s);
        break;
      default:
        ap[0] = image->getData(2); // qt byter order bgra
        ap[1] = image->getData(1);
        ap[2] = image->getData(0);
        ap[3] = image->getData(3);
        ensureQImage(qimage,w,h,QImage::Format_RGB32);
        ippiCopy_8u_P4C4R(ap,step,qimage->bits(),4*step,s);
        break;
    }
#else
#warning "QImageConverter::img8uToQImage fallback not yet implemented"
#endif
    STQ=uptodate;
    return qimage;
  }

  // }}}

  const Img8u *QImageConverter::qimageToImg8u(QImage *qimage, Img8u *&image){
    // {{{ open

    ICLASSERT_RETURN_VAL(qimage && !qimage->isNull() ,0);
    ensureCompatible((ImgI**)&image,depth8u,Size(qimage->width(),qimage->height()),3);
    icl8u *ap[3] = { image->getData(0), image->getData(1), image->getData(2) };
    ippiCopy_8u_C3P3R(qimage->bits(),image->getLineStep()*3, ap, image->getLineStep(),image->getSize());

    STU = uptodate;
    return image;
  }

  // }}}

  const Img32f *QImageConverter::qimageToImg32f(QImage *qimage, Img32f *&image){
    // {{{ open

    ICLASSERT_RETURN_VAL(qimage && !qimage->isNull() ,0);  
    ensureCompatible((ImgI**)&image,depth32f,Size(qimage->width(),qimage->height()),3);
    
    int dim = image->getDim();
    icl8u *buf = getBuffer8u(image->getDim()*3);
    icl8u *ap[] = { buf,buf+dim,buf+2*dim };
    ippiCopy_8u_C3P3R(qimage->bits(),3*dim, ap, dim,image->getSize());
    icl::copy(ap[0],ap[0]+dim,image->getData(0));
    icl::copy(ap[1],ap[1]+dim,image->getData(1));
    icl::copy(ap[2],ap[2]+dim,image->getData(2));

    STF = uptodate;
    return image;  
  }

  // }}}

  
}

