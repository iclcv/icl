#ifndef QIMAGE_CONVERTER_H
#define QIMAGE_CONVERTER_H

#include "ICLTypes.h"

// forward declared QImage class
class QImage;

namespace icl{

  /// class for conversion between QImage and ImgI
  /** The QImageConverter class provides functionality for conversion
      between the QImage class and the Img8u/Img32f classes.
      It provides an intern buffer handling for the destination images, 
      so that the user does not have to care about memory Handling. The 
      user must only take care, that the given image is persistent.
  
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
      
      Another use case is to optimize performance in a working loop,
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
      implemented and tested. Conversion from Img32f to QImage is realized
      by a workaround: The currently hold Img32f is converted to an Img8u
      in a first step. The second step converts the depth8u version of the
      image to a qimage (this could be accelerated!).

      <b>Note also:</b> If you call setImage(Img8u* xxx) before calling
      getImage8u() you will get a <em> copy of the pointer xxx</em>. This
      is essentially, as you will not have a 2nd instance of the image.
  */
  
  class QImageConverter{
    public:
    /// creates an empty QImageConverter object
    QImageConverter();

    /// creates a QImageConverter object with given ImgI
    QImageConverter(const ImgI *image);

    /// creates a QImageConverter object with given QImage
    QImageConverter(const QImage *qimage);

    /// Destructor 
    /** if the released object was the last QImageConverter object, 
        all static buffers are freed, to avoid large unused memory
    */
    ~QImageConverter();
    
    /// returns a converted QImage
    /** This function will cause an error if no images were set before.
        Images can be set by calling setImage, setQImage, or by using
        one of the not empty constructors.    
    */
    const QImage *getQImage();

    /// returns converted ImgI (of depth "depth8u")
    /** This function will cause an error if no images were set before.
        Images can be set by calling setImage, setQImage, or by using
        one of the not empty constructors.    
    */
    const ImgI *getImage();

    /// returns converted Img8u
    /** This function will cause an error if no images were set before.
        Images can be set by calling setImage, setQImage, or by using
        one of the not empty constructors.    
    */
    const Img8u *getImg8u();

    /// returns converted Img32f
    /** This function will cause an error if no images were set before.
        Images can be set by calling setImage, setQImage, or by using
        one of the not empty constructors.    
    */
    const Img32f *getImg32f();

    /// sets the current source image of type Img8u or Img32f
    /** All further set images get the state "outdated". Hence all later
        <em>getXXXX-calls</em> must perform a deep conversion first
    */
    void setImage(const ImgI *image);
    
    /// sets the current source image of type QImage
    /** All further set images get the state "outdated". Hence all later
        <em>getXXXX-calls</em> must perform a deep conversion first
    */
    void setQImage(const QImage *qimage); 
    

    private:

    /// internal conversion function
    const QImage *img32fToQImage(Img32f *image, QImage *&qimage);

    /// internal conversion function
    const QImage *img8uToQImage(Img8u *image, QImage *&qimage);

    /// internal conversion function
    const Img8u *qimageToImg8u(QImage *qimage, Img8u *&image);

    /// internal conversion function
    const Img32f *qimageToImg32f(QImage *qimage, Img32f *&image);
    
    /// internal used state struct
    enum State{ given=0,  /**< this image was given calling setImage  */
                uptodate=1, /**< this image has already been converted */
                undefined=2 /**< this image is not defined or <em>outdated</em> */
    };
    
    /// internal state buffer for the images in order [Img8u,Img32f,QImage]
    State m_eStates[3];
    
    /// internal used Img8u buffer
    Img8u *m_poImgBuffer8u;

    /// internal used Img32f buffer
    Img32f *m_poImgBuffer32f;

    /// internal used QImage buffer
    QImage *m_poQImageBuffer;
  };
}

#endif
