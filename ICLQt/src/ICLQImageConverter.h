#include <ICLTypes.h>
#ifndef QIMAGE_CONVERTER_H
#define QIMAGE_CONVERTER_H


// forward declared QImage class
class QImage;

namespace icl{

  /// class for conversion between QImage and ImgBase/Img\<T\>
  /** The QImageConverter class provides functionality for conversion
      between the QImage class and the Img\<T\> classes.
      It provides an internal buffer handling for the destination images, 
      so that the user does not have to care about memory handling. The 
      user must only take care, that the given image is persistent.
  
      <h2>Use cases</h2>
      The basic use case is just to convert one Image into another:
      <pre>       
      ImgBase *i = imgNew(...);
      QImage *q = QImageConverter(i).getImage();
      </pre>
      This will temporarily create a converter object on the stack,
      that converts the given image <em>i</em> into a qimage. 
      The opposite direction (QImage to ImgBase) behaves identically.
      <b>Note:</b> that the converted image is only persistent as long
      as the QImageConverter object is.
      
      Another use-case is to optimize performance in a working loop,
      by reusing the same instance of QImageConverter. By writing
      <pre>
      QImageConverter c;
      while(true){
         ...
         ImgBase *i = ...
         c.setImage(i);
         QImage *q = q.getQImage();
         ...
      }
      </pre>
      The converter will internally adapt itself to this use-case
      (getting pointers to ImgBase objects and returning pointers to
      QImages) that no memory allocation must be performed during the
      iteration. Only if several use cases are performed alternating, it
      might be necessary to allocate and release memory during lifetime.
     
      <b>Note:</b> If you call setImage(Img8u* xxx) before calling
      getImage8u() you will get a <em> copy of the pointer xxx</em>. This
      is essentially, as you will not have a 2nd instance of the image.
  */
  
  class QImageConverter{
    public:
    /// creates an empty QImageConverter object
    QImageConverter();

    /// creates a QImageConverter object with given ImgBase
    QImageConverter(const ImgBase *image);

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

    /// returns converted ImgBase (of given depth")
    /** This function will cause an error if no images were set before.
        Images can be set by calling setImage, setQImage, or by using
        one of the not empty constructors.    
    */
    const ImgBase *getImgBase(icl::depth d=depth8u);


    /// template returing an image of given datatype
    template<class T>
    const Img<T> *getImg();

    /// sets the current source image of type Img8u or Img32f
    /** All further set images get the state "outdated". Hence all later
        <em>getImg[Base]-calls</em> must perform a deep conversion first
    */
    void setImage(const ImgBase *image);
    
    /// sets the current source image of type QImage
    /** All further set images get the state "outdated". Hence all later
        <em>getImg[Base]-calls</em> must perform a deep conversion first
    */
    void setQImage(const QImage *qimage); 
    

    private:

    /// internal used state struct
    enum State{ given=0,  /**< this image was given calling setImage  */
                uptodate=1, /**< this image has already been converted */
                undefined=2 /**< this image is not defined or <em>outdated</em> */
    };
    
    /// internal buffer for Imgs of all depths
    ImgBase *m_apoBuf[5];
    
    /// internal qimage buffer
    QImage *m_poQBuf;

    /// internal state buffer (states indicate if images are uptodate, given or outdated
    State m_aeStates[5];
    
    /// internal state buffer for the QImage buffer
    State m_eQImageState;
  };
}

#endif
