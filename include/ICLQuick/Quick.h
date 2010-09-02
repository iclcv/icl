/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQuick/Quick.h                               **
** Module : ICLQuick                                               **
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

#ifndef ICL_QUICK_H
#define ICL_QUICK_H

#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>

using namespace icl;

/// just used by default
using namespace std;

/// all ICLQuick functions are placed here
namespace icl{

#define ICL_QUICK_DEPTH depth32f
#define ICL_QUICK_TYPE icl32f
  
    
  /// typedef for the quick images type (this time Img<icl32f>)
  typedef Img<ICL_QUICK_TYPE> ImgQ;

  /** @{ @name creator functions **/
  /* {{{ open */
  /// create an empty (black) ImgQ (affinity for floats)
  /** see \ref AFFINITY
      @param width image width
      @param height image height
      @param channels image channel count
  **/
  template<class T>
  Img<T> zeros(int width, int height, int channels=1);
  
  /***/
  inline ImgQ zeros(int width, int height, int channels=1){ 
    return zeros<ICL_QUICK_TYPE>(width,height,channels); 
  }
  
  
  /// create an ImgQ that is pre-initialized with ones (affinity for floats)
  /** see \ref AFFINITY
      @param width image width
      @param height image height
      @param channels image channel count      
  **/
  template<class T>
  Img<T> ones(int width, int height, int channels=1);

  /** \cond affinity version */
  inline ImgQ ones(int width, int height, int channels=1){ return ones<ICL_QUICK_TYPE>(width,height,channels);  }
  /** \endcond */
  
  /// load an image file read file (affinity for floats) 
  /** see \ref AFFINITY
      @param filename filename/pattern to read 
  **/
  template<class T>
  Img<T> load(const std::string &filename);

   /** \cond affinity version */
  inline ImgQ load(const std::string &filename) { return load<ICL_QUICK_TYPE>(filename); }
  /** \endcond */
  
  /// loads an image file and returns image in given format (affinity for floats)
  /** see \ref AFFINITY
      @param filename filename to read (*.jpg, *.ppm, *.pgm)
      @param fmt image format to convert the result to
  **/
  template<class T>
  Img<T> load(const std::string &filename, format fmt);

  /** \cond affinity version */
  inline ImgQ load(const std::string &filename, format fmt) { return load<ICL_QUICK_TYPE>(filename,fmt); }
  /** \endcond */
  
  /// create a test image (converted to destination format) (affinity for floats)
  /** see \ref AFFINITY
      @param name identifier for the image:
                  names are: parrot, lena, cameraman, mandril, 
                  windows, flowers, women, house and tree 
      @param fmt image format to convert the result to
  **/
  template<class T>
  Img<T> create(const std::string &name, format fmt=formatRGB);

  /** \cond affinity version */
  inline ImgQ create(const std::string &name, format fmt=formatRGB) { return create<ICL_QUICK_TYPE>(name,fmt); }
  /** \endcond */
  
  /// grabs a new image from given device (affinity for floats) 
  /** see \ref AFFINITY
      @param dev device driver type (see Generic Grabber for more details)
      @param devSpec device specifier
      @param size output image size (grabbers size if Size::null)
      @param fmt output format
      @param releaseGrabber if set to true, the 
             correspondig grabbers are deleted 
             immediately */
  template<class T>
  Img<T> grab(const std::string &dev, const std::string &devSpec, 
              const Size &size=Size::null, format fmt=formatRGB,
              bool releaseGrabber=false);

  /** \cond affinity version */
  inline ImgQ grab(const std::string &dev, const std::string &devSpec, 
                   const Size &size=Size::null, format fmt=formatRGB,
                   bool releaseGrabber=false){
    return grab<ICL_QUICK_TYPE>(dev,devSpec,size,fmt,releaseGrabber);
  }
  /** \endcond */

  /// read an image for pwc webcam with given size, and format (affinity for float)
  /** see \ref AFFINITY
      if releaseGrabber is set to 1, the internal used PWCGrabber is released after this call 
      @param device device for this grabbin call (0,1,2 or 3)
      @param size size of the returned image
      @param fmt format of the returned image
      @param releaseGrabber indicates whether the internal grabber object should be released
                            after this pwc call
  **/
  template<class T>
  inline Img<T> pwc(int device=0, const Size &size=Size(640,480), format fmt=formatRGB, bool releaseGrabber=false){
    return grab<T>("pwc",str(device),size,fmt,releaseGrabber);
  }
  
  /** \cond affinity version */
  inline ImgQ pwc(int device=0, const Size &size=Size(640,480), format fmt=formatRGB, bool releaseGrabber=false){
    return grab<ICL_QUICK_TYPE>("pwc",str(device),size,fmt,releaseGrabber);
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
  Img8u cvt8u(const ImgQ &image);

  /// Converts the image into depth16
  /** @param image source image 
      @return converted image 
  **/
  Img16s cvt16s(const ImgQ &image);

  /// Converts the image into depth32s
  /** @param image source image 
      @return converted image 
  **/
  Img32s cvt32s(const ImgQ &image);

  /// Converts the image into depth32f
  /** @param image source image 
      @return converted image 
  **/
  Img32f cvt32f(const ImgQ &image);

  /// Converts the image into depth64f
  /** @param image source image 
      @return converted image 
  **/
  Img64f cvt64f(const ImgQ &image);

  /// Converts a given Img8u into an ImgQ
  /** @param image source image 
      @return converted image 
  **/
  ImgQ cvt(const Img8u &image);

  /// Converts a given Img16s into an ImgQ
  /** @param image source image 
      @return converted image 
  **/
  ImgQ cvt(const Img16s &image);

  /// Converts a given Img32s into an ImgQ
  /** @param image source image 
      @return converted image 
  **/
  ImgQ cvt(const Img32s &image);

  /// Converts a given Img32f into an ImgQ
  /** @param image source image 
      @return converted image 
  **/
  ImgQ cvt(const Img32f &image);

  /// Converts a given Img64f into an ImgQ
  /** @param image source image 
      @return converted image 
  **/
  ImgQ cvt(const Img64f &image);

  /// Converts a given ImgBase into an ImgQ
  /** @param image source image 
      @return conveted image 
  **/
  ImgQ cvt(const ImgBase *image);


  /// Converts a given ImgBase into an ImgQ
  /** @param image source image 
      @return conveted image 
  **/
  ImgQ cvt(const ImgBase &image);
  /** @} **/
  /* }}} */

  /** @{ @name filtering function **/
  /* {{{ open */


  /// applies a filter operation on the source image
  /** @param image source image
      @param filter filter name, possible: sobelx, sobely, gauss, 
                    laplacs, median, dilation,erosion, opening and
                    closing 
  **/
  ImgQ filter(const ImgQ &image, const std::string &filter);
  
  /// applies a color conversion
  /** @param image source image*
      @param fmt destination image format
  **/
  ImgQ cc(const ImgQ& image, format fmt);
  
  /// converts a given image to formatRGB
  /** @param image source image **/
  ImgQ rgb(const ImgQ &image);
  
  /// converts a given image to formatHLS
  /** @param image source image **/
  ImgQ hls(const ImgQ &image);

  /// converts a given image to formatLAB
  /** @param image source image **/
  ImgQ lab(const ImgQ &image);

  /// converts a given image to formatGray
  /** @param image source image **/
  ImgQ gray(const ImgQ &image);
  
  /// scales an image by a given factor
  /** @param image source image
      @param factor scale factor 
  **/
  ImgQ scale(const ImgQ& image, float factor);

  /// scales an image to the given size
  /** @param image source image
      @param width destination image width
      @param height destination image height
  **/
  ImgQ scale(const ImgQ& image, int width, int height);
  
  /// picks a specific image channel
  /** @param image source image
      @param channel channel index to pick
  **/
  ImgQ channel(const ImgQ &image,int channel);

  /// reduces an images quantisation levels
  /** Internally the image is coverted to Img8u and back to
      apply this operation.
      @param image source image
      @param levels gray level count for each channel of the 
                    destination image
  **/
  ImgQ levels(const ImgQ &image, icl8u levels);
  
  /// performs an image binarisation for each channel with given threshold
  /** @param image source image
      @param threshold threshold to compare each pixel with
  **/
  ImgQ thresh(const ImgQ &image, float threshold);
 
  /// deep copy for an image
  /** @param image source image **/
  ImgQ copy(const ImgQ &image);
  
  /// deep copy of an images roi
  /** @param image source image **/
  ImgQ copyroi(const ImgQ &image);
  
  /// normalize an images range to [0,255]
  /** @param image source image **/
  ImgQ norm(const ImgQ &image);
  
  /// horizontal flip of an image
  /** @param image source image **/
  ImgQ flipx(const ImgQ& image);

  /// vertical flip of an image
  /** @param image source image **/
  ImgQ flipy(const ImgQ& image);

  /** @} **/
  /* }}} */
  
  /** @{ @name output functions **/
  /* {{{ open */
  
  /// write an image to HD 
  /** @param image source image
      @param filename filename to write the image to.
  **/
  void save(const ImgQ &image, const std::string &filename);
  
  /// shows an image using TestImages::show
  /** The image is wrote to disk into a temporary file. Then it
      is shown using a specifi show command, which can be set, using
      the the showSetup function, which is also available in this 
      package.
      @param image image to show
      @see showSetput(const string&, const stirng&, int)
  **/
  void show(const ImgQ &image);
  
  
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
  void showSetup(const string &showCommand="xv %s", const string &rmCommand="rm -rf %s", int msecBeforeDelete=500);
  
  /// print the images parameters to std::out
  /** @param image image to print to std::out **/
  void print(const ImgQ &image);

  /** @} **/
  /* }}} */
  
  /** @{ @name ImgQ arithmetical operators **/
  /* {{{ open */

  /// adds two images pixel-wise
  /** @param a first source image 
      @param b second source image 
  **/
  ImgQ operator+(const ImgQ &a, const ImgQ &b);
  
  /// subtracts two images pixel-wise
  /** @param a first source image 
      @param b second source image 
  **/
  ImgQ operator-(const ImgQ &a, const ImgQ &b);
  
  /// multiplies two images pixel-wise
  /** @param a first source image 
      @param b second source image 
  **/
  ImgQ operator*(const ImgQ &a, const ImgQ &b);

  /// divides two images pixel-wise
  /** @param a first source image 
      @param b second source image 
  **/
  ImgQ operator/(const ImgQ &a, const ImgQ &b);

  /// adds a constant to each pixel value
  /** @param image source image 
      @param val const addition value
  **/
  ImgQ operator+(const ImgQ &image, float val);

  /// subtracts a constant to each pixel value
  /** @param image source image 
      @param val const subtraction value
  **/
  ImgQ operator-(const ImgQ &image, float val);

  /// multiplies each pixel value with a constant
  /** @param image source image 
      @param val const multiplication value
  **/
  ImgQ operator*(const ImgQ &image, float val);

  /// divides each pixel value by a constant
    /** @param image source image 
      @param val const division value
  **/
  ImgQ operator/(const ImgQ &image, float val);

  /// adds a constant to each pixel value
  /** @param image source image 
      @param val const addition value
  **/
  ImgQ operator+(float val, const ImgQ &image);

  /// subtracts each pixel value from a constant
  /** @param image source image 
      @param val const left value for subtraction
  **/
  ImgQ operator-(float val, const ImgQ &image);
  
  /// multiplies each pixel value with a constant
    /** @param image source image 
      @param val const multiplication value
  **/
  ImgQ operator*(float val, const ImgQ &image);
  
  /// divides a constant by each pixel value
  /** @param image source image 
      @param val nominator for the division operation
  **/
  ImgQ operator/(float val, const ImgQ &image);

  /// returns image*(-1)
  /** @param image source image **/
  ImgQ operator-(const ImgQ &image);

  /** @} **/
  /* }}} */
  
  /** @{ @name ImgQ arithmetical functions **/
  /* {{{ open */

  /// calls exp( each pixel )
  /** @param image source image **/
  ImgQ exp(const ImgQ &image);

  /// calls ln( each pixel )  
  /** @param image source image **/
  ImgQ ln(const ImgQ &image);

  /// calls ( each pixel )²
  /** @param image source image **/
  ImgQ sqr(const ImgQ &image);
  
  /// calls sqrt( each pixel)
  /** @param image source image **/
  ImgQ sqrt(const ImgQ &image);
  
  /// calls abs ( each pixel)
  /** @param image source image **/
  ImgQ abs(const ImgQ &image);
  
  /** @} **/
  /* }}} */
  
  /** @{ @name ImgQ logical operators **/
  /* {{{ open */

  
  /// pixel-wise logical or
  /** @param a first source image 
      @param b second source image
  **/
  ImgQ operator||(const ImgQ &a, const ImgQ &b);

  /// pixel-wise logical and
  /** @param a first source image 
      @param b second source image
  **/
  ImgQ operator&&(const ImgQ &a, const ImgQ &b);

  /// pixels-wise binary or (each value is converted to T brefore binary or)
  template<class T>
  ImgQ binOR(const ImgQ &a, const ImgQ &b);
  
  /// pixels-wise binary or (each value is converted to T brefore binary or)
  template<class T>
  ImgQ binXOR(const ImgQ &a, const ImgQ &b);

  /// pixels-wise binary or (each value is converted to T brefore binary or)
  template<class T>
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
  ImgQ operator,(const ImgQ &a, const ImgQ &b);
  
  /// vertical image combination (as ,-operator)
  /** @param a upper image
      @param b lower image
  **/
  ImgQ operator%(const ImgQ &a, const ImgQ &b);

  /// channel concatenation of images
  /** @param a first image (channels f1,f2,f3,...)
      @param b second image (channels s2, s2,s3,...)
      @return image with channels (f1,f2,... s1, s2, ...)
  **/
  ImgQ operator|(const ImgQ &a, const ImgQ &b);

  /** @} **/
  /* }}} */
  
  /** @{ @name ImgQ roi copy **/
  /* {{{ open */


  /// internal stuct, used for deep image copies
  /** @see icl::roi(Img<icl32f> &)*/
  struct ImgROI{
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
  ImgROI roi(ImgQ &r);
  
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
  ImgROI data(ImgQ &r);

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
  void color(float r, float g=-1, float b=-1, float alpha=255);

  /// sets the current fill color to given r,g,b,alpha value
  /** @param r red value 
      @param g green value (if < 0, g is set to r) 
      @param b blue value (if < 0, b is set to r) 
      @param alpha alpha value 255 = no transparency, 0 = full transparency
  **/
  void fill(float r, float g=-1, float b=-1, float alpha=255);

  /// returns the current color state
  /** @param color destintaion array for the current draw color 
      @param fill destinaion array for the current fill color 
  **/
  void colorinfo(float color[4], float fill[4]);
  
  /// draws a 6x6-cross into an image
  /** @param image destination image 
      @param x x-pos of the cross
      @param y y-pos of the cross
  **/
  void cross(ImgQ &image, int x, int y);

  /// draws a 6x6-cross into an image
  /** @param image destination image 
      @param p position 
  **/
  inline void cross(ImgQ &image, const Point &p) { cross(image,p.x,p.y); }

  /// draws a rect into an image
  /** @param image destination image
      @param x x-pos of the rect
      @param y x-pos of the rect
      @param w width of the rect
      @param h height of the rect
      @param rounding rounded corners (in pixels)
  **/
  void rect(ImgQ &image, int x, int y, int w, int h, int rounding=0);

  /// draws a rect into an image
  /** @param image destination image
      @param r rect to draw
      @param rounding rounded corners (in pixels)
  **/
  inline void rect(ImgQ &image, const Rect &r, int rounding=0){ rect(image,r.x,r.y,r.width,r.height,rounding); }
  
  /// draws a triangle into an image
  /** given 3 points (x1,y1),(x2,y2),(x3,y3) */
  void triangle(ImgQ &image, int x1,int y1, int x2, int y2, int x3, int y3);

  /// draws a triangle into an image
  inline void triangle(ImgQ &image, const Point &a, const Point &b, const Point &c){
    triangle(image,a.x,a.y,b.x,b.y,c.x,c.y);
  }
  
  /// draws a line into an image
  /** @param image destination image 
      @param x1 fist point x coord 
      @param y1 fist point y coord 
      @param x2 second point x coord 
      @param y2 second point y coord      
  **/
  void line(ImgQ &image, int x1, int y1, int x2, int y2); 

  /// draws a line into an image
  /** @param image destination image 
      @param p1 fist point
      @param p2 second point
  **/
  inline void line(ImgQ &image, const Point &p1, const Point &p2){ line(image,p1.x,p1.y, p2.x,p2.y); }

  /// draws a strip of connected lines
  /** @param image destination image
      @param pts list of points
      @param closeLoop if true, then also the first and the last point is connected */
  void linestrip(ImgQ &image, const std::vector<Point> &pts, bool closeLoop=true);
  
  /// draw a single pixel into an image
  /** @param image destination image 
      @param x xpos of the pixel 
      @param y ypos of the pixel 
  **/
  void pix(ImgQ &image, int x, int y);

  /// draw a single pixel into an image
  /** @param image destination image 
      @param p pos of the pixel
  **/
  inline void pix(ImgQ &image, const Point &p){ pix(image,p.x,p.y); }
  
  /// draws a set of points into an image
  /** @param image destination image
      @param pts vector of points
  **/
  void pix(ImgQ &image, const vector<Point> &pts);

  /// draws a set of point sets into an image
  /** @param image destination image
      @param pts vector of vector of points to draw
  **/
  void pix(ImgQ &image, const vector<vector<Point> > &pts);
  
  /// renders a filled circle into an image
  /** This function renders a filled circle into a 3 or 1 channel image (only with fill color!)
			using a QPainter internally.
  	  @param image destination image
      @param x x-pos of the circle center
      @param y x-pos of the circle center
      @param r radius of the circle
  **/
  void circle(ImgQ &image, int x, int y, int r);
  
  /// renders a text into an image (only available with Qt-Support)
  /** This functin renders a text into a 3 or 1 channel image
      using the a QPainter internally.
      @param image destination image
      @param x xpos of the lower left corner of the text
      @param y ypos of the lower left corner of the text
      @param text text to render
  **/
  void text(ImgQ &image, int x, int y,const string &text);

   /// renders a text into an image (only available with Qt-Support)
  /** This functin renders a text into an 3 or 1 channel image
      using the a QPainter internally.
      @param image destination image
      @param p pos of the lower left corner of the text
      @param sText text to render
  **/
  inline void text(ImgQ &image, const Point &p,const string &sText){ text(image,p.x,p.y,sText); }

  /// labels an image in the upper left corner (only available with Qt-Support)
  /** @param image image to label
      @param text text label
      @return labeled source image (= given image)
   **/
  ImgQ label(const ImgQ &image, const string &text);

  /// sets up the current font  (only available with Qt-Support)
  /** @param size new font size 12 by default 
      @param family font family string "Arial" by default
  **/
  void font(int size, const string &family="Arial");
  
  /// sets up current fontsize (only available with Qt-Support)
  /** @param size new font size (default is 12) **/
  void fontsize(int size);
  /** @} **/
  /* }}} */

  /** @{ @name timer and benchmarking **/
  /* {{{ open */

  /// starts a timer
  void tic();
  
  /// stops a timer started with tic()
  void toc();
  /** @} **/
  /* }}} */
                
}


#endif
