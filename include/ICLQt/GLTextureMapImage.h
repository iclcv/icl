/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/GLTextureMapImage.h                      **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Michael Götting                  **
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
*********************************************************************/

#ifndef ICL_GL_TEXTUREMAP_IMAGE
#define ICL_GL_TEXTUREMAP_IMAGE

#ifdef SYSTEM_APPLE
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <ICLUtils/Size.h>
#include <ICLUtils/SimpleMatrix.h>
#include <ICLQt/ImageStatistics.h>
#include <ICLCore/Types.h>
#include <ICLCore/Img.h>
namespace icl{

  /// Utility class for displaying images using OpenGLs texture mapping abilities \ingroup UNCOMMON
  /** As former pixel drawing implementations used glDrawPixels(..) to draw images 
      into a GL context, the visualization engine became very slow on some machines.
      Many sources mentioned already, that there are some performance problems with
      the glDrawPixels(..) procedure.\n

      The new implementation uses OpenGL's texture mapping abilities for drawing images.\n
      It allows you to draw an Img<T> into a specific rect on the screen by using 
      hardware accelerated drawing function of OpenGL.
      
      This class provides a more convenient access to this functionalities by wrapping
      all texture mapping functions. A GLTextureMapImage can be created by given size,
      channel count and a so called <em>cellSize</em> which is explained in the 
      following. It will break a given image apart into several rectangular pieces, that
      are transferred (in interleaved data order) into the video memory of the graphics 
      device to be able to draw these parts as simple textures. The given cellSize 
      defines how large theses cells are an therewith, how many cells are needed. \n

      In addition, it knows two modes
      - single buffer mode
      - multi buffer mode
      
      which have different initialization and drawing procedures for different use cases.
      The <em>single buffer mode</em> is used, when the image should just be drawn now. This mode
      has to be used, when the GLTextureMapImage instance is created just inside of OpenGL's
      drawing function (not outside or in another Thread!). In this case, the image drawing
      procedure is optimized to use as less dynamic memory as possible. If the image 
      is created outside of OpenGL's drawing function, the singleBuffer flag has to be
      set to false. In this case, no GL commands are used except in the drawTo(..) function.
      
      Disregarding the mode, there are 3 steps that must be performed to draw
      an image into the screen. 
      -# creation
        - <b>single buffer mode</b>: in this case, some OpenGL commands are already   
          used in the constructor. A data for a single image cell (size = channel count *
          cellSize * cellSize) is allocated and glTexture handles are reserved.
        - <b>multi buffer mode</b>: here, no OpenGL commands must be called, but 
          instead of this, data buffers are created for all image cells.

      -# updating the textures (by given image)
        - <b>single buffer mode</b>: This step will transfer the image data into the
          video memory. As in OpenGL all data of one texture must be transferred by a 
          single TexImage2D call, the data is temporarily transformed into interleaved data
          order by calling ICLCC's planarToInterleaved method. Here, the cell data buffer,
          which was created in the constructor is exploited, to avoid redundant data 
          allocation during run-time. 
        - <b>multi buffer mode</b>: In this case, however no GL commands may be used. So the
          data is not transferred into the video memory, be it is pre-buffered in interleaved
          order into the array of cell data buffers, that have been reserved in the 
          constructor.
      -# drawing step
        - <b>single buffer mode</b>: In this mode, all image data is available in the
          video devices memory, so it can be drawn directly cell by cell. The drawing
          function supports implicit scaling and transforming of GL's matrices to draw
          the image into a given rectangle in the window.
        - <b>multi buffer mode</b>: In this case, the interleaved image data, available
          in the cell data buffer matrix must be transferred into the video memory, before
          it can be drawn exactly as if single buffer mode was enabled here.

      
      \section DET detail
      Internally some additional computations must be performed as the matrix of texture 
      cells will be an amount of pixels wider and higher then the image to draw. 
     
      \section TMP allowed template parameters
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
    GLTextureMapImage(const Size &imageSize, bool useSingleBuffer, int channels=3, int cellSize=128);
    
    /// destructor
    ~GLTextureMapImage();

    /// updates the textures of this image in the video hardware's memory
    void updateTextures(const Img<T> *image);
  
    /// draws the image into the given rect on a GL context with given window and view port size 
    void drawTo(const Rect &rect, const Size &windowSize, scalemode mode=interpolateNN);

    /// draw the image into the rectangle specified by Center and two given axis
    /** Example:
        <pre>
        Center------------->FirstAxis
          |................
          |...         ....
          |...  Image  ....
          |...         ....
          V................
        SecondAxis
        </pre>
    **/
    void drawTo3D(float *pCenter, float *pFirstAxis, float *pSecondAxis);
    
    /// returns current image height
    int getImageWidth() const { return m_iImageW; }

    /// returns current image width
    int getImageHeight() const { return m_iImageH; }

    /// returns current image channel count
    int getImageChannels() const { return m_iChannels; }
    
    /// returns whether this GLTextureMapImage is compatible to a given image
    bool compatible(const Img<T> *image) const;

    /// returns whether this image is in single buffer mode or not2
    bool hasSingleBuffer() const { return m_bUseSingleBuffer; }
    
    /// sets up current brightness contrast and intensity
    /** if b=c=i=-1 then, brightness is adapted automatically */
    void bci(int b=-1, int c=-1, int i=-1);

    /// returns the current minimun and maximum value (only available in mutli buffer mode)
    /** This function works very inefficient as it converts the current array of
        cell datas back into an image (using interleavedToPlanar) before it searches for
        the min and max value of the tmp image using its getMinMax(channel) function */
    std::vector<Range<T> > getMinMax() const;
    
    /// retuns the color at a given image location or a zero sized vector, (x,y) is outside the image
    std::vector<icl64f> getColor(int x, int y)const;

    /// creates a deep copy of the current buffered image (only available in multi buffer mode)
    Img<T> *deepCopy() const;

    /// called from the top level GLTextureMapBaseImage
    const ImageStatistics &updateStatistics(ImageStatistics &s);

    /// sets whether to visualize the pixel-grid or not
    void setDrawGrid(bool enabled, float *color);
    
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
    
    /// indicates whether to use a single buffer or one buffer per texture
    bool m_bUseSingleBuffer;
    
    /// if mode is multiTextureBuffer, the data is stored here
    SimpleMatrix<T*> m_matCellData;
    
    /// buffer for cell data (only one cell needs to be buffered at one time)
    T *m_ptCellData;

    /// holds brightness contrast and intensity values
    int m_aiBCI[3];
    
    /// holds state if the grid is currently visualized
    bool m_drawGrid;
    
    /// holds the current grid color
    float m_gridColor[4];
    
  };
}

#endif
