/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GLImg.h                                  **
** Module : ICLGeom                                                **
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

#ifndef ICL_GL_IMG_H
#define ICL_GL_IMG_H

#include <ICLCore/ImgBase.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Range.h>
#include <ICLQt/ImageStatistics.h>

namespace icl{
  /// OpenGL Texture Map Image class
  /** The GLImg provides a simple interface to use ICL images as OpenGL texture maps.
      
      \section _TEX_ Image Textures
      Copying an image to a texture is applied in two steps. First, the image data
      is internally buffered. Then later the data is uploaded to the graphics memory.
      The buffering mechanism is restricted by the maximum
      texture size that is supported by the OpenGL system. Usually, the maximum texture
      size is 4096 or 8192, but the OpenGL standard does only say, that at least 64 
      must be supported. Therefore, it cannot be guaranteed that each image can be
      represented by a single OpenGL texture. Instead, larger images are represented
      by a 2D array of tiles, where each tile is represented by a single OpenGL texture.

      \section _BUF_ Internal Buffering 
      Since the image data potentially needs to be split into several tiles, is is
      simpler to align the buffered data in image tiles. In order to minimize the amount
      of glTexImage2D calls, the data is furthermore buffered in interleaved data order.
      Here, byte-, short- and float- typed images (icl8u, icl16s and icl32f) are supported
      natively, i.e. they are buffered and uploaded in their native format. The other depths
      (icl32s and icl64f) are not supported natively, i.e. image of this depth are already
      buffered as floats. For int-typed images (icl32s) this, means, that very high image
      values cannot be represented correctly. And double typed images are also clipped to 
      the float range and accuracy.
      
      \section _UP_ Uploading Texture Data
      While the image buffering step is applied immediately (usually in the working
      thread), uploading the texture data to the graphics hardware is applied in a deferred
      manner. This has to reasons:
      -# It is not allowed to access OpenGL from several threads. Due to the Qt-dependency, 
         OpenGL will always <em>live</em> in the applications main thread. 
      -# If textures are visualized more frequently then their data is changed. A simple
         internal <em>dirty</em>-flag can be used to decide, whether the OpenGL texture
         data is already up-to-date. 
      
      In other words, when the texture is drawn (i.e. using GLImg::draw2D or GLImg::draw3D),
      the buffered data is automatically uploaded to the graphics hardware <b> if this is
      necessary</b>.
  */
  class GLImg : public Uncopyable{
    struct Data;  //!< internal data structure
    Data *m_data; //!< internal data pointer
    
    public:
    
    /// creates a new GLImg instance
    /** optional parameters are
        @param src optionally given source image (if null, isNull() will return true)
        @param sm texture interpolation mode (either interpolateNN or interpolateLIN)
        @param maxCellSize the cells size (see \ref _BUF_)
        */
    GLImg(const ImgBase *src=0, scalemode sm=interpolateNN, int maxCellSize=4096);
    
    /// destructor
    ~GLImg();
    
    /// set new texture data
    /** if source is null, the texture handle is deleted and isNull() will return true */
    void update(const ImgBase *src, int maxCellSize=4096);
    
    /// sets the texture interpolation mode
    void setScaleMode(scalemode sm);
    
    /// returns whether a non-null images was buffered
    bool isNull() const;
    
    /// draws the image to the given 2D rect
    /** This method is optimized for the OpenGL parameters set by the ICLQt::Widget class */
    void draw2D(const Rect &r, const Size &windowSize);
    
    /// draws the texture to the given nodes quad in 3D space
    /** the point order is 
        <pre>
        a ---- b
        |      |
        |      |
        c ---- d
        </pre>
        
        An addition to the corner 3D positions (a, b, c, and d), optionally normals (na, nb, nc and 
        nd) can be given. The normals are only used, if none of the normal pointers is null. Additionally
        non-standard texture coordinates can be used to draw only parts of the texture.

    */
    void draw3D(const float a[3],const float b[3],const float c[3],const float d[3],
                const float na[3]=0, const float nb[3]=0, const float nc[3]=0, const float nd[3]=0,
                const Point32f &texCoordsA=Point32f(0,0),
                const Point32f &texCoordsB=Point32f(1,0),
                const Point32f &texCoordsC=Point32f(0,1),
                const Point32f &texCoordsD=Point32f(1,1));     

    /// draws the texture to given quad that is spanned by two vectors
    inline void draw3D(const float a[3],const float b[3],const float c[3]){
      const float d[3] = { b[0] + c[0] -a[0], b[1] + c[1] -a[1], b[2] + c[2] -a[2]  };
      draw3D(a,b,c,d);
    }


    /// draws the single texture spread to a given grid of nodes
    /** \section _LIMIT_ size limitation
        This does only work, if the texture size (width and height) is smaller than the 
        maximum texture size supported by openGL. Usually, this is >= 4096.
        
        \section _LAYOUT_ data layout
        The grid data layout is row-major, i.e. the grid node at (x,y) is obtained
        by (xs[idx], ys[idx], zs[idx]) where idx is stride*(x+nx*y). By using the stride
        parameter, this function can be used for both, planar and interleaved data.
        
        \section _INTER_ interleaved data
        \code
        typedef FixedColVector<float,4> V4;
        const int nx = 20, ny = 30;
        icl::Array2D<V4> grid(nx,ny); // interleaved data with stride = 4
        // (...) fill grid nodes with values
        ImgQ image = create("lena");
        GLImg gli(&image);
        V4 &v = grid[0];
        gli.drawToGrid(nx,ny,&v[0],&v[1],&v[2],0,0,0,4);
        \endcode

        \section _PLANAR_ planar data
        // add code from example above
        const int dim = nx*ny;
        std::vector<float> xs(dim), ys(dim), zs(dim);
        // (...) fill xs, ys and zs with data
        gli.drawToGrid(nx,ny,xs.data(), ys.data(), zs.data());
        
    */
    void drawToGrid(int nx, int ny, const float *xs, const float *ys, const float *zs,
                    const float *nxs=0, const float *nys=0, const float *nzs=0,
                    const int stride = 1);
    
    /// 3D vector type
    typedef FixedColVector<float,3> Vec3;
    
    /// a grid function returns a 3D pos from given 2D grid position
    typedef Function<Vec3,int,int> grid_function; 
    
    /// draws the texture to an nx x ny grid whose positions and normals are defined by functions
    /** The grid results are buffered internally in order to avoid extra function evaluations.
        Once, the data is buffered, it is passed to 
        GLImg::drawGrid(int,int,const float*,const float*,const float*,const float*,const float*,const float*,int)

        \section _EX_ example
        \code
        GLImg::Vec3 grid_func(int x, int y){
          float nx = 30, ny = 20;
          return GLImg::Vec3(x-(nx/2),y-(ny/2),(nx/10.) * sin(float(x)/5 + t*5 )) * (300./nx);
        }
        
        void renderGrid(){
           GLImg gli(&image);
           static Time first = Time::now();
           float t = (Time::now() - first).toSecondsDouble();
           gli.drawToGrid(30,20,grid_func);
        }
        \endcode
    */
    void drawToGrid(int nx, int ny, grid_function gridVertices, 
                    grid_function gridNormals = grid_function());
    

    /// returns the maximum texture map size that is supported by the present OpenGL system
    static int getMaxTextureSize();
    
    /// returns the number of internal cells used for the texture
    Size getCells() const;
    
    /// binds the given texture cell using glBindTexture(...)
    void bind(int xCell=0, int yCell=0);

    /// returns the image size
    inline Size getSize() const { return Size(getWidth(), getHeight()); }

     /// returns current image height
    int getWidth() const;

    /// returns current image width
    int getHeight() const;

    /// returns current image channel count
    int getChannels() const;
    
    /// returns the image depth
    depth getDepth() const;
    
    /// returns the image format
    format getFormat() const;
    
    /// returns the image roi
    Rect getROI() const;

    /// returns the current images time stamp
    Time getTime() const;

    /// sets up current brightness contrast and intensity
    /** if b=c=i=-1 then, brightness is adapted automatically */
    void setBCI(int b=-1, int c=-1, int i=-1);

    /// returns the current minimun and maximum values for all channels
    std::vector<Range64f> getMinMax() const;
    
    /// retuns the color at a given image location or a zero sized vector, (x,y) is outside the image
    std::vector<icl64f> getColor(int x, int y)const;

    /// creates ImgBase version of the currently buffered images
    /** Please note, that the ownership is not passed to the caller! */
    const ImgBase *extractImage() const;

    /// returns statistics of the currently buffered image
    const ImageStatistics &getStats() const;

    /// sets whether to visualize the pixel-grid or not
    void setDrawGrid(bool enabled, float *color=0);

    /// sets the grid color
    void setGridColor(float *color);

    /// returns the current grid color
    const float *getGridColor() const;
    
    /// returns the current scalemode
    scalemode getScaleMode() const;
    
    /// locks the texture buffer
    void lock() const;

    /// unlocks the texture buffer
    void unlock() const;

    
  };
}



#endif
