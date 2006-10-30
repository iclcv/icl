#ifndef QIMAGE_CONVERTER_H
#define QIMAGE_CONVERTER_H

#include "ICLTypes.h"

// forward declared QImage class
class QImage;

namespace icl{

  /// class for conversion between QImage and ImgI
  /** The QImageConverter class provides functionality for conversion
      between the QImage class and the Img8u/Img32f classes.
      It povides an intern buffer handling for the destination images, 
      so that the user does not have to care about memory Handling. The 
      user must only take care, that the given image is persitent.
  
      As the class is highly optimized, some additional static image
      buffers are allocated in the background. These buffers are deleted,
      when the last QImageConverter object is deleted (reference counting).

      <h2>Use cases</h2>
      The basic use case is just to convert one Image into another:
      <pre>       
      ImgI *i = imgNew(...);
      QImage *q = QImageConverter(i).getImage();
      </pre>
      This will temporarily create a converter object on the stack,
      that converts the given image <em>i</em> into a qimage. 
      The opposite direction (QImage to ImgI) behaves identically.
      
      Anoter use case is to optimize performance in a working loop,
      by reusing the same instance of QImageConverter. By writing
      <pre>
      QImageConverter c;
      while(true){
         ...
         ImgI *i = ...
         c.setImage(i);
         QImage *q = q.getQImage();
         ...
      }
      </pre>
      The converter will internally adapt itself to this use-case
      (getting pointers to ImgI objects and returning pointers to
      QImages) that no memory allocation must be performed during the
      iteration. Only if several use cases are performed alternating, it
      might be necessary to allocate and release memory during lifetime.
     
      <b>Note:</b> This time, only conversion for Img8u to QImage is 
      implemented and tested.
  */
  class QImageConverter{
    public:
    QImageConverter();
    QImageConverter(const ImgI *image);
    QImageConverter(const QImage *qimage);
    ~QImageConverter();
    
    const QImage *getQImage();
    const ImgI *getImage();
    const Img8u *getImg8u();
    const Img32f *getImg32f();

    void setImage(const ImgI *image);
    void setQImage(const QImage *qimage); 
    
    private:

    const QImage *img32fToQImage(Img32f *image, QImage *&qimage);
    const QImage *img8uToQImage(Img8u *image, QImage *&qimage);
    const Img8u *qimageToImg8u(QImage *qimage, Img8u *&image);
    const Img32f *qimageToImg32f(QImage *qimage, Img32f *&image);
    
    enum State{ given=0, 
                uptodate=1, 
                undefined=2 };
    
    State m_eStates[3];
    Img8u *m_poImgBuffer8u;
    Img32f *m_poImgBuffer32f;
    QImage *m_poQImageBuffer;
  };
}

#endif
