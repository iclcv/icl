#ifndef ICL_GL_TEXTUREMAP_IMAGE
#define ICL_GL_TEXTUREMAP_IMAGE

#include <GL/gl.h>
#include <iclSize.h>
#include <iclSimpleMatrix.h>
#include <iclTypes.h>
#include <iclImg.h>
namespace icl{
  
  /// Utility class for displaying images using OpenGLs texture mapping abilities
  /** As former pixel drawing implementations used glDrawPixels(..) to draw images
      into a GL context, the visualization engine became very slow on some machines.
      Many sources mentioned already, that there are some performance problems with
      the glDrawPixels(..) procedure.\n

      The new implementation uses OpenGL's texture mapping abilities for drawing images.\n
      
      This class provides a more convenient access to the functionalities by wrapping
      all texture mapping functions. A GLTextureMapImage can be created by given size,
      channel count and a so called <em>cellSize</em> which is explained in the 
      following. To draw images on the screen, you first have to update the internal
      used textures on the graphics hardware by calling:
      \code
      updateTextures(myImage);
      \endcode
      This will break apart the given image internally into so called image cells. This
      cells are compatible to GL-textures (squared, with edge-length equal to a power of 
      2, e.g. 16, 32, 64). This edge length can be set by the constructor parameter
      cellSize (some analysis have shown, that a cellSize of 128 works very good). The
      cell parts data is than transformed into an interleaved order and copied into the
      graphics hardware memory. 
      When this is done, it is possible to call:
      \code 
      drawTo(Rect imageRect, Size windowSize 
      \endcode
      This will draw the image using Texture mapping very quickly (640x480 ~ 0.1ms !!)
      
      \section DET detail
      Internally some additional computations must be performed as the matrix of texture 
      cells will be an amount of pixels wider and higher then the image to draw. Yet, these
      pixels are filled with black pixels, so they are not visible on black background.
     
      \section TODO todo
      a further adaption should fix the problem mentioned in \ref DET by adapting some
      texture parameters
      
      \section TMP allowed template paramters
      Because OpenGL does not support double valued textures, the GLTextureMapImage class
      is not instantiated for depth64f. To visualize double images, it is recommended to
      transform them into a float representation first.
  */
  template<class T>
  class GLTextureMapImage{
    
    /// internal used allocations class for creation of a SimpleMatrix<Size>
    struct SimpleMatrixAllocSize {
      static Size create() { return Size::null; }
    };
    public:
    
    /// constructor
    GLTextureMapImage(const Size &imageSize, int channels=3, int cellSize=128);
    
    /// destructor
    ~GLTextureMapImage();

    /// updates the textures of this image in the video hardware's memory
    void updateTextures(const Img<T> *image);
  
    /// draws the image into the given rect on a GL context with given window and view port size 
    void drawTo(const Rect &rect, const Size &windowSize);
    
    /// returns current image height
    int getImageWidth() const { return m_iImageW; }

    /// returns current image width
    int getImageHeight() const { return m_iImageH; }

    /// returns current image channel count
    int getImageChannels() const { return m_iChannels; }
    
    /// returns whether this GLTextureMapImage is compatible to a given image
    bool compatible(const Img<T> *image) const;

    /// sets up current brightness contrast and intensity
    /** if b=c=i=-1 then, brightness is adapted automatically */
    void bci(int b=-1, int c=-1, int i=-1);
    private:

    /// internally used for debugging (TODO remove and make glabal function)
    static void setPackAlignment(depth d, int linewidth);

    /// internally used for debugging (TODO remove and make glabal function)
    void setUpPixelTransfer(depth d, float brightness, float constrast, float intensity, const ImgBase *image);

    /// resets scale to 1  and bias to 0 for r,g,b and a channel (TODO remove and make glabal function)
    void resetPixelTransfer();

    /// associated image width
    int m_iImageW;  
    
    /// associated image height
    int m_iImageH;

    /// count of cells in horizontal direction
    int m_iXCells;
    
    /// count of cells in vertical direction
    int m_iYCells;
    
    /// amount of horizontal pixels that are not filled in the right-most cell 
    int m_iRestX;

    /// amount of vertical pixels that are not filled in the bottom cell 
    int m_iRestY, m_iCellSize;

    /// count of channels (supported are 1 and 3)
    int m_iChannels;
    
    /// internal used size of cell data 
    int m_iCellDataSize;
    
    /// matrix containing gl texture handles
    SimpleMatrix<GLuint> m_matTextureNames;
    
    /// matrix containing ROI sizes for the cells
    SimpleMatrix<Size,SimpleMatrixAllocSize> m_matROISizes;
    
    /// buffer for cell data (only one cell needs to be buffered at one time)
    T *m_ptCellData;

    /// holds brightness contrast and intensity values
    int m_aiBCI[3];
    
  };
}

#endif
