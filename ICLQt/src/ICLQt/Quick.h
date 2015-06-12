/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/Quick.h                                **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/Img.h>
#include <QtWidgets/QCompleter>

namespace icl{
  namespace utils{}
  namespace math{}
  namespace core{}
  namespace filter{}
  namespace io{}
  namespace qt{}
  namespace geom{}
  namespace cv{}
  namespace markers{}
  namespace physics{}
}

#ifndef ICL_NO_USING_NAMESPACES
using namespace icl;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;
using namespace icl::io;
using namespace icl::qt;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::markers;
using namespace icl::physics;
#endif

/// just used by default
using namespace std;

/// all ICLQuick functions are placed here
namespace icl{
  namespace qt{
  
  #ifdef ICL_HAVE_QT
    /// uses Qt to spawn an open-file dialog with given filter
    /** throws an exception if cancel was pressed. The function is thread-safe and can savely be called from a working thread */
    ICLQt_API std::string openFileDialog(const std::string &filter = "",
                               const std::string &caption="open file",
                               const std::string &initialDirectory="_____last", 
                               void *parentWidget=0) throw (utils::ICLException);
  
    /// uses Qt to spawn a save-file dialog with given filter
    /** throws an exception if cancel was pressed. The function is thread-safe and can savely be called from a working thread */
    ICLQt_API std::string saveFileDialog(const std::string &filter = "",
                               const std::string &caption="save file",
                               const std::string &initialDirectory="_____last", 
                               void *parentWidget=0) throw (utils::ICLException);

    /// uses Qt to spawn a text input dialog
    /** throws an exception if cancel was pressed. The function is thread-safe and can savely be 
        called from a working thread. If the visImage pointer is not null, it will be visualized 
        in the dialog */
    std::string textInputDialog(const std::string &caption="text ...",
                                const std::string &message="please write your text here",
                                const std::string &initialText="",
                                void *parentWidget=0,
                                core::ImgBase *visImage=0,
                                QCompleter *textCompleter=0) throw (utils::ICLException);

   
    /// executes the given command as a child process and returns it output
    /** Internally, popen is used */
    ICLQt_API std::string execute_process(const std::string &command);
  #endif
  
  #define ICL_QUICK_DEPTH depth32f
  #define ICL_QUICK_TYPE icl32f
    
      
    /// typedef for the quick images type (this time core::Img<icl32f>)
    typedef core::Img<ICL_QUICK_TYPE> ImgQ;
  
    /** @{ @name creator functions **/
    /* {{{ open */
    /// create an empty (black) ImgQ (affinity for floats)
    /** @param width image width
        @param height image height
        @param channels image channel count
    **/
    template<class T> ICLQt_API
    core::Img<T> zeros(int width, int height, int channels=1);
    
    /***/
    inline ImgQ zeros(int width, int height, int channels=1){ 
      return zeros<ICL_QUICK_TYPE>(width,height,channels); 
    }
    
    
    /// create an ImgQ that is pre-initialized with ones (affinity for floats)
    /** @param width image width
        @param height image height
        @param channels image channel count      
    **/
    template<class T> ICLQt_API
    core::Img<T> ones(int width, int height, int channels=1);
  
    /** \cond affinity version */
    inline ImgQ ones(int width, int height, int channels=1){ return ones<ICL_QUICK_TYPE>(width,height,channels);  }
    /** \endcond */
    
    /// load an image file read file (affinity for floats) 
    /** @param filename filename/pattern to read 
    **/
    template<class T> ICLQt_API
    core::Img<T> load(const std::string &filename);
  
     /** \cond affinity version */
    inline ImgQ load(const std::string &filename) { return load<ICL_QUICK_TYPE>(filename); }
    /** \endcond */
    
    /// loads an image file and returns image in given core::format (affinity for floats)
    /** @param filename filename to read (*.jpg, *.ppm, *.pgm)
        @param fmt image core::format to convert the result to
    **/
    template<class T> ICLQt_API
    core::Img<T> load(const std::string &filename, core::format fmt);
  
    /** \cond affinity version */
    inline ImgQ load(const std::string &filename, core::format fmt) { return load<ICL_QUICK_TYPE>(filename,fmt); }
    /** \endcond */
    
    /// create a test image (converted to destination core::format) (affinity for floats)
    /** @param name identifier for the image:
                    names are: parrot, lena, cameraman, mandril, 
                    windows, flowers, women, house and tree 
        @param fmt image core::format to convert the result to
    **/
    template<class T> ICLQt_API
    core::Img<T> create(const std::string &name, core::format fmt=icl::core::formatRGB);
  
    /** \cond affinity version */
    inline ImgQ create(const std::string &name, core::format fmt=icl::core::formatRGB) { return create<ICL_QUICK_TYPE>(name,fmt); }
    /** \endcond */
    
    /// grabs a new image from given device (affinity for floats) 
    /** @param dev device driver type (see Generic Grabber for more details)
        @param devSpec device specifier
        @param size output image size (grabbers size if utils::Size::null)
        @param fmt output format
        @param releaseGrabber if set to true, the 
               correspondig grabbers are deleted 
               immediately */
    template<class T> ICLQt_API
    core::Img<T> grab(const std::string &dev, const std::string &devSpec, 
                const utils::Size &size=utils::Size::null, core::format fmt=core::formatRGB,
                bool releaseGrabber=false);
  
    /** \cond affinity version */
    inline ImgQ grab(const std::string &dev, const std::string &devSpec, 
                     const utils::Size &size=utils::Size::null, core::format fmt=icl::core::formatRGB,
                     bool releaseGrabber=false){
      return grab<ICL_QUICK_TYPE>(dev,devSpec,size,fmt,releaseGrabber);
    }
    /** \endcond */
  
    /** @} **/
    /* }}} */
  
    /** @{ @name converting function **/
    /* {{{ open */
  
    /// Converts the image into depth8u
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API core::Img8u cvt8u(const ImgQ &image);
  
    /// Converts the image into depth16
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API core::Img16s cvt16s(const ImgQ &image);
  
    /// Converts the image into depth32s
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API core::Img32s cvt32s(const ImgQ &image);
  
    /// Converts the image into depth32f
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API core::Img32f cvt32f(const ImgQ &image);
  
    /// Converts the image into depth64f
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API core::Img64f cvt64f(const ImgQ &image);
  
    /// Converts a given core::Img8u into an ImgQ
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API ImgQ cvt(const core::Img8u &image);
  
    /** \cond */
    /// Converts a given core::Img16s into an ImgQ
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API ImgQ cvt(const core::Img16s &image);
  
    /// Converts a given core::Img32s into an ImgQ
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API ImgQ cvt(const core::Img32s &image);
  
    /// Converts a given core::Img32f into an ImgQ
    /** @param image source image 
        @return converted image 
    **/

    ICLQt_API ImgQ cvt(const core::Img32f &image);

    /// Converts a given core::Img64f into an ImgQ
    /** @param image source image 
        @return converted image 
    **/
    ICLQt_API ImgQ cvt(const core::Img64f &image);
    /** \endcond */

    /// Converts a given core::ImgBase into an ImgQ
    /** @param image source image 
        @return conveted image 
    **/
    ICLQt_API ImgQ cvt(const core::ImgBase *image);
  
  
    /// Converts a given core::ImgBase into an ImgQ
    /** @param image source image 
        @return conveted image 
    **/
    ICLQt_API ImgQ cvt(const core::ImgBase &image);
    /** @} **/
    /* }}} */
  
    /** @{ @name filtering function **/
    /* {{{ open */
  
    /// applies a filter operation on the source image (affinity for float)
    /** @param image source image
        @param filter filter name, possible: sobelx, sobely, gauss, 
                      laplacs, median, dilation,erosion, opening and
                      closing 
    **/
    template<class T> ICLQt_API
    core::Img<T> filter(const core::Img<T> &image, const std::string &filter);
    
    /** \cond affinity version */
    inline ImgQ filter(const ImgQ &image, const std::string &filter){
      return icl::qt::filter<float>(image,filter);
    }
    /** \endcond */
  
    /// applies gaussian blur to a given image (using a maskRadius*2+1-sized gaussian filter)
    /** affinity for floats */
    template<class T> ICLQt_API
    core::Img<T> blur(const core::Img<T> &image, int maskRadius=1);
    
    /** \cond */
    inline ImgQ blur(const ImgQ &image, int maskRadius=1){
      return icl::qt::blur<float>(image,maskRadius);
    }
    /** \endcond */
    
    /// applies a color conversion
    /** @param image source image*
        @param fmt destination image format
    **/
    ICLQt_API ImgQ cc(const ImgQ& image, core::format fmt);
    
    /// converts a given image to formatRGB
    /** @param image source image **/
    ICLQt_API ImgQ rgb(const ImgQ &image);
    
    /// converts a given image to formatHLS
    /** @param image source image **/
    ICLQt_API ImgQ hls(const ImgQ &image);
  
    /// converts a given image to formatLAB
    /** @param image source image **/
    ICLQt_API ImgQ lab(const ImgQ &image);
  
    /// converts a given image to formatGray
    /** @param image source image **/
    ICLQt_API ImgQ gray(const ImgQ &image);
    
    /// scales an image by a given factor
    /** @param image source image
        @param factor scale factor 
    **/
    ICLQt_API ImgQ scale(const ImgQ& image, float factor);
  
    /// scales an image to the given size
    /** @param image source image
        @param width destination image width
        @param height destination image height
    **/
    ICLQt_API ImgQ scale(const ImgQ& image, int width, int height);
    
    /// picks a specific image channel
    /** @param image source image
        @param channel channel index to pick
    **/
    ICLQt_API ImgQ channel(const ImgQ &image, int channel);
  
    /// reduces an images quantisation levels
    /** Internally the image is coverted to core::Img8u and back to
        apply this operation.
        @param image source image
        @param levels gray level count for each channel of the 
                      destination image
    **/
    ICLQt_API ImgQ levels(const ImgQ &image, icl8u levels);
    
    /// performs an image binarisation for each channel with given threshold
    /** @param image source image
        @param threshold threshold to compare each pixel with
    **/
    ICLQt_API ImgQ thresh(const ImgQ &image, float threshold);
   
    /// deep copy for an image
    /** @param image source image **/
    template<class T> ICLQt_API
    core::Img<T> copy(const core::Img<T> &image);
    
    /// deep copy of an images roi
    /** @param image source image **/
    template <class T> ICLQt_API
    core::Img<T> copyroi(const core::Img<T> &image);
    
    /// normalize an images range to [0,255]
    /** @param image source image **/
    template<class T> ICLQt_API
    core::Img<T> norm(const core::Img<T> &image);
    
    /// horizontal flip of an image
    /** @param image source image **/
    ICLQt_API ImgQ flipx(const ImgQ& image);
  
    /// vertical flip of an image
    /** @param image source image **/
    ICLQt_API ImgQ flipy(const ImgQ& image);
  
    /** @} **/
    /* }}} */
    
    /** @{ @name output functions **/
    /* {{{ open */
    
    /// write an image to HD 
    /** @param image source image
        @param filename filename to write the image to.
    **/
    ICLQt_API void save(const core::ImgBase &image, const std::string &filename);
    
    /// shows an image using TestImages::show
    /** The image is wrote to disk into a temporary file. Then it
        is shown using a specifi show command, which can be set, using
        the the showSetup function, which is also available in this 
        package.
        @param image image to show
        @see showSetput(const string&, const stirng&, int)
    **/
    ICLQt_API void show(const icl::core::ImgBase &image);
    
    /// setup image visualisation programm
    /** when images are shown using an extrenal viewer like gnu's xv, the image is temporarily 
        written to the hard disk. Then the show command is called. Before the rmCommand is
        called usleep(1000*msecBeforeDelete); is called, to ensure, that the viewer has read the
        image completely. By default, the <b>"iclxv"</b> image viewer is used, which is available
        as example of the ICLQt package. In this case, the showCommand is "iclxv -input %s -delete".
        As iclxv automatically deletes the image, when it was read, the rmCommand is empty and
        the wait time is 0. To use this, you have to place at least a link to ICLQt/examples/iclxv
        into any directory contained in your path variable.
        @see show
        @see ICLIO/TestImages 
    **/
    ICLQt_API void showSetup(const string &showCommand = "xv %s", const string &rmCommand = "rm -rf %s", int msecBeforeDelete = 500);
    
    /// print the images parameters to std::out
    /** @param image image to print to std::out **/
    template<class T>
    void print(const core::Img<T> &image);
  
    /** @} **/
    /* }}} */
    
    /** @{ @name ImgQ arithmetical operators **/
    /* {{{ open */
  
    /// adds two images pixel-wise
    /** @param a first source image 
        @param b second source image 
    **/
    ICLQt_API ImgQ operator+(const ImgQ &a, const ImgQ &b);
    
    /// subtracts two images pixel-wise
    /** @param a first source image 
        @param b second source image 
    **/
    ICLQt_API ImgQ operator-(const ImgQ &a, const ImgQ &b);
    
    /// multiplies two images pixel-wise
    /** @param a first source image 
        @param b second source image 
    **/
    ICLQt_API ImgQ operator*(const ImgQ &a, const ImgQ &b);
  
    /// divides two images pixel-wise
    /** @param a first source image 
        @param b second source image 
    **/
    ICLQt_API ImgQ operator/(const ImgQ &a, const ImgQ &b);
  
    /// adds a constant to each pixel value
    /** @param image source image 
        @param val const addition value
    **/
    ICLQt_API ImgQ operator+(const ImgQ &image, float val);
  
    /// subtracts a constant to each pixel value
    /** @param image source image 
        @param val const subtraction value
    **/
    ICLQt_API ImgQ operator-(const ImgQ &image, float val);
  
    /// multiplies each pixel value with a constant
    /** @param image source image 
        @param val const multiplication value
    **/
    ICLQt_API ImgQ operator*(const ImgQ &image, float val);
  
    /// divides each pixel value by a constant
      /** @param image source image 
        @param val const division value
    **/
    ICLQt_API ImgQ operator/(const ImgQ &image, float val);
  
    /// adds a constant to each pixel value
    /** @param image source image 
        @param val const addition value
    **/
    ICLQt_API ImgQ operator+(float val, const ImgQ &image);
  
    /// subtracts each pixel value from a constant
    /** @param image source image 
        @param val const left value for subtraction
    **/
    ICLQt_API ImgQ operator-(float val, const ImgQ &image);
    
    /// multiplies each pixel value with a constant
      /** @param image source image 
        @param val const multiplication value
    **/
    ICLQt_API ImgQ operator*(float val, const ImgQ &image);
    
    /// divides a constant by each pixel value
    /** @param image source image 
        @param val nominator for the division operation
    **/
    ICLQt_API ImgQ operator/(float val, const ImgQ &image);
  
    /// returns image*(-1)
    /** @param image source image **/
    ICLQt_API ImgQ operator-(const ImgQ &image);
  
    /** @} **/
    /* }}} */
    
    /** @{ @name ImgQ arithmetical functions **/
    /* {{{ open */
  
    /// calls exp( each pixel )
    /** @param image source image **/
    ICLQt_API ImgQ exp(const ImgQ &image);
  
    /// calls ln( each pixel )  
    /** @param image source image **/
    ICLQt_API ImgQ ln(const ImgQ &image);
  
    /// calls ( each pixel )²
    /** @param image source image **/
    ICLQt_API ImgQ sqr(const ImgQ &image);
    
    /// calls sqrt( each pixel)
    /** @param image source image **/
    ICLQt_API ImgQ sqrt(const ImgQ &image);
    
    /// calls abs ( each pixel)
    /** @param image source image **/
    ICLQt_API ImgQ abs(const ImgQ &image);
    
    /** @} **/
    /* }}} */
    
    /** @{ @name ImgQ logical operators **/
    /* {{{ open */
  
    
    /// pixel-wise logical or
    /** @param a first source image 
        @param b second source image
    **/
    ICLQt_API ImgQ operator||(const ImgQ &a, const ImgQ &b);
  
    /// pixel-wise logical and
    /** @param a first source image 
        @param b second source image
    **/
    ICLQt_API ImgQ operator&&(const ImgQ &a, const ImgQ &b);
  
    /// pixels-wise binary or (each value is converted to T brefore binary or)
    template<class T> ICLQt_API
    ImgQ binOR(const ImgQ &a, const ImgQ &b);
    
    /// pixels-wise binary or (each value is converted to T brefore binary or)
    template<class T> ICLQt_API
    ImgQ binXOR(const ImgQ &a, const ImgQ &b);
  
    /// pixels-wise binary or (each value is converted to T brefore binary or)
    template<class T> ICLQt_API
    ImgQ binAND(const ImgQ &a, const ImgQ &b);
    
    /** @} **/
    /* }}} */
    
    /** @{ @name ImgQ concatenation operators **/
    /* {{{ open */
  
    /// horizontal image concatenation
    /** <pre>
        example: ImgQ a,b,c;
        a=aaa  b = bbbbb   
          aaa      bbbbb
          aaa
        
        c = a,b,a;
        
        c = aaabbbbbaaa
            aaabbbbbaaa
            aaa00000aaa
        </pre>
        empty spaces are set to 0;
        
        @param a left image
        @param b right image
    **/
    ICLQt_API ImgQ operator,(const ImgQ &a, const ImgQ &b);
    
    /// vertical image combination (as ,-operator)
    /** @param a upper image
        @param b lower image
    **/
    ICLQt_API ImgQ operator%(const ImgQ &a, const ImgQ &b);
  
    /// channel concatenation of images
    /** @param a first image (channels f1,f2,f3,...)
        @param b second image (channels s2, s2,s3,...)
        @return image with channels (f1,f2,... s1, s2, ...)
    **/
    ICLQt_API ImgQ operator|(const ImgQ &a, const ImgQ &b);
  
    /** @} **/
    /* }}} */
    
    /** @{ @name ImgQ roi copy **/
    /* {{{ open */
  
  
    /// internal stuct, used for deep image copies
    /** @see icl::roi(core::Img<icl32f> &)*/
    struct ICLQt_API ImgROI{
      /// image data
      ImgQ image;
      
      /// sets up the member images ROI to the i's ROI
      /** @param i source image **/
      ImgROI &operator=(const ImgQ &i);
      
      /// sets up the member images ROI to val
      /** @param val value to set up each pixel with **/
      ImgROI &operator=(float val);
      
      /// sets up the member images ROI to the given ROI
      /** @param r source roi**/
      ImgROI &operator=(const ImgROI &r);
      
      /// implicit cast operator
      operator ImgQ();
    };
    
    /// creates a ROI-struct from an image
    /** This function helps to copy images ROIs:
        <pre>
        ImgQ a,b;
        
        a = aaaaa (A = ROI pixel)
            aaAAA (a = no ROI pixel)
            aaAAA
        
        b = bbb
            bbb
        
        roi(a) = b;
        
        a = aaaaa
            aabbb
            aabbb
        
        also possible: roi(a) = roi(b);
        </pre>
        @param r image to wrap
    **/
    ICLQt_API ImgROI roi(ImgQ &r);
    
    /// creates full ROI ROI-struct
    /** this can also be used for deep copies
        <pre>
        ImgQ a,b;
        
        a = aaa    b = bBBBb ( B = ROI pixel )
            aaa        bBBBb ( b = no ROI pixel )
            aaa        bBBBb
                       bbbbb
        
        data(a) = roi(b)
        
        a = BBB
            BBB
            BBB
  
        equal to:
        or a = copyroi(b) // this will release a and reallocate its data 
        </pre>
        @param r image to wrap
    **/
    ICLQt_API ImgROI data(ImgQ &r);
  
    /** @} **/
    /* }}} */
    
    /** @{ @name drawing functions **/
    /* {{{ open */
  
    /// sets the current color to given r,g,b,alpha value
    /** @param r red value 
        @param g green value (if < 0, g is set to r) 
        @param b blue value (if < 0, b is set to r) 
        @param alpha alpha value 255 = no transparency, 0 = full transparency
    **/
    ICLQt_API void color(float r, float g = -1, float b = -1, float alpha = 255);
  
    /// sets the current fill color to given r,g,b,alpha value
    /** @param r red value 
        @param g green value (if < 0, g is set to r) 
        @param b blue value (if < 0, b is set to r) 
        @param alpha alpha value 255 = no transparency, 0 = full transparency
    **/
    ICLQt_API void fill(float r, float g = -1, float b = -1, float alpha = 255);
  
    /// returns the current color state
    /** @param color destintaion array for the current draw color 
        @param fill destinaion array for the current fill color 
    **/
    ICLQt_API void colorinfo(float color[4], float fill[4]);
    
    /// draws a 6x6-cross into an image
    /** @param image destination image 
        @param x x-pos of the cross
        @param y y-pos of the cross
    **/
    ICLQt_API void cross(ImgQ &image, int x, int y);
  
    /// draws a 6x6-cross into an image
    /** @param image destination image 
        @param p position 
    **/
    inline void cross(ImgQ &image, const utils::Point &p) { cross(image,p.x,p.y); }
  
    /// draws a rect into an image
    /** @param image destination image
        @param x x-pos of the rect
        @param y x-pos of the rect
        @param w width of the rect
        @param h height of the rect
        @param rounding rounded corners (in pixels)
    **/
    ICLQt_API void rect(ImgQ &image, int x, int y, int w, int h, int rounding = 0);
  
    /// draws a rect into an image
    /** @param image destination image
        @param r rect to draw
        @param rounding rounded corners (in pixels)
    **/
    inline void rect(ImgQ &image, const utils::Rect &r, int rounding=0){ rect(image,r.x,r.y,r.width,r.height,rounding); }
    
    /// draws a triangle into an image
    /** given 3 points (x1,y1),(x2,y2),(x3,y3) */
    ICLQt_API void triangle(ImgQ &image, int x1, int y1, int x2, int y2, int x3, int y3);
  
    /// draws a triangle into an image
    inline void triangle(ImgQ &image, const utils::Point &a, const utils::Point &b, const utils::Point &c){
      triangle(image,a.x,a.y,b.x,b.y,c.x,c.y);
    }
    
    /// draws a line into an image
    /** @param image destination image 
        @param x1 fist point x coord 
        @param y1 fist point y coord 
        @param x2 second point x coord 
        @param y2 second point y coord      
    **/
    ICLQt_API void line(ImgQ &image, int x1, int y1, int x2, int y2);
  
    /// draws a line into an image
    /** @param image destination image 
        @param p1 fist point
        @param p2 second point
    **/
    inline void line(ImgQ &image, const utils::Point &p1, const utils::Point &p2){ line(image,p1.x,p1.y, p2.x,p2.y); }
  
    /// draws a strip of connected lines
    /** @param image destination image
        @param pts list of points
        @param closeLoop if true, then also the first and the last point is connected */
    ICLQt_API void linestrip(ImgQ &image, const std::vector<utils::Point> &pts, bool closeLoop = true);
    
    /// draws a polygon (constructed out of linestrips
    ICLQt_API void polygon(ImgQ &image, const std::vector<utils::Point> &corners);
    
    /// draw a single pixel into an image
    /** @param image destination image 
        @param x xpos of the pixel 
        @param y ypos of the pixel 
    **/
    ICLQt_API void pix(ImgQ &image, int x, int y);
  
    /// draw a single pixel into an image
    /** @param image destination image 
        @param p pos of the pixel
    **/
    inline void pix(ImgQ &image, const utils::Point &p){ pix(image,p.x,p.y); }
    
    /// draws a set of points into an image
    /** @param image destination image
        @param pts vector of points
    **/
    ICLQt_API void pix(ImgQ &image, const vector<utils::Point> &pts);
  
    /// draws a set of point sets into an image
    /** @param image destination image
        @param pts vector of vector of points to draw
    **/
    ICLQt_API void pix(ImgQ &image, const vector<vector<utils::Point> > &pts);
    
    /// renders a filled circle into an image
    /** This function renders a filled circle into a 3 or 1 channel image (only with fill color!)
  			using a QPainter internally.
    	  @param image destination image
        @param x x-pos of the circle center
        @param y x-pos of the circle center
        @param r radius of the circle
    **/
    ICLQt_API void circle(ImgQ &image, int x, int y, int r);
    
    /// renders a text into an image (only available with Qt-Support)
    /** This functin renders a text into a 3 or 1 channel image
        using the a QPainter internally.
        @param image destination image
        @param x xpos of the lower left corner of the text
        @param y ypos of the lower left corner of the text
        @param text text to render
    **/
    ICLQt_API void text(ImgQ &image, int x, int y, const string &text);
  
     /// renders a text into an image (only available with Qt-Support)
    /** This functin renders a text into an 3 or 1 channel image
        using the a QPainter internally.
        @param image destination image
        @param p pos of the lower left corner of the text
        @param sText text to render
    **/
    inline void text(ImgQ &image, const utils::Point &p,const string &sText){ text(image,p.x,p.y,sText); }
  
    /// labels an image in the upper left corner (only available with Qt-Support)
    /** @param image image to label
        @param text text label
        @return labeled source image (= given image)
     **/
    ICLQt_API ImgQ label(const ImgQ &image, const string &text);
  
    /// sets up the current font  (only available with Qt-Support)
    /** @param size new font size 12 by default 
        @param family font family string "Arial" by default
    **/
    ICLQt_API void font(int size, const string &family = "Arial");
    
    /// sets up current fontsize (only available with Qt-Support)
    /** @param size new font size (default is 12) **/
    ICLQt_API void fontsize(int size);
    /** @} **/
    /* }}} */
  
    /** @{ @name timer and benchmarking **/
    /* {{{ open */
  
    /// starts a timer
    ICLQt_API void tic(const std::string &label = "");
    
    /// stops a timer started with tic()
    ICLQt_API void toc();
    /** @} **/
    /* }}} */
                  
  } // namespace qt
}

