#ifndef ICL_CORE_H
#define ICL_CORE_H

#include <ICLCore/Img.h>
#include <ICLCore/CornerDetectorCSS.h>
#include <ICLCore/ImgBorder.h>
#include <ICLCore/Line32f.h>
#include <ICLCore/Line.h>
#include <ICLCore/LineSampler.h>
#include <ICLCore/Mathematics.h>
#include <ICLCore/Random.h>
#include <ICLCore/SampledLine.h>

/** 
    \defgroup TYPES Common Data Type Definitions
    \defgroup GENERAL General Utility and Support Functions
    \defgroup IMAGE Image Classes and Support Functions
    \defgroup STRUTILS Utiltiy functions for std::string conversions
    \defgroup MATH Mathematical Utiltiy functions

    \mainpage Image Component Library (ICL) 

    ICL is a C++ image processing library, developed in the 
    <a href="www.ni.www.techfak.uni-bielefeld.de"> Neuroinformatics Group</a> at the 
    <a href="www.uni-bielefeld.de">University of Bielefeld in Germany</a>. 
    ICL provides a large set of features for image acquisition, image processing and
    image visualization. During the design and development process, the following main goals took center stage:
    - Optimal Performace (ensured by internal use of Intel IPP, see \ref IPP_MKL)
    - Simple and easy to use C++-interface (see \ref EXAMPLE)
    - Platform-Independence (currently linux and MacOS-X are supported)
    - No compulsory software dependencies (see \ref EXTERNAL_DEPS)

    ICL provides all necessary building blocks for the development of complex computer-vision applications.

    \section EXAMPLE Simple Viewer Application for Arbitrary Camera Sources

    <TABLE border=0><TR><TD>
    \code
    #include <ICLQuick/Common.h>

    GUI gui;

    void init(){
      gui << "image[@handle=image@minsize=16x12]";
      gui.show();
    }

    void run(){
      static GenericGrabber g(FROM_PROGARG("-i"));
      gui["image"] = g.grab();
      gui["image"].update();
    }

    int main(int n, char **args){
      return ICLApplication(n,args,"-input|-i(2)",
                            init,run).exec();
    }
    \endcode

    </TD><TD>
    \image html viewer.jpg
    </TD></TR></TABLE>

    - Save this file as <tt>viewer.cpp</tt>
    - Setup your <tt>PKG_CONFIG_PATH</tt> variable
    - Compile with <tt>CXXFLAGS=`pkg-config --libs --cflags icl` make viewer</tt>
    - Plug in a camera
    - Run your application e.g. with <tt>./viewer -input unicap '*'</tt>

    An example that demonstrates accessing a grabbed images pixels is provided below (see \ref COMPLEX_EXAMPLE)

    \section PACKAGES Packages

    <TABLE border=0><TR><TD>
    ICL consists of currently 11 packages that are listed in the main menu at the left.

    - <b>ICLUtils</b> Contains general purpose functions and classes that are currently not part of the C++-STL (e.g. threads or matrices).
    - <b>ICLCore</b> basically provides class definitions for ICL's image classes Img and ImgBase and related global functions.
    - <b>ICLCC</b> provides functions and classes for color conversion.
    - <b>ICLIO</b> extends the range of functions by input and output classes. Camera grabbers different camera 
      types (e.g. IEEE-1394 or Video-4-Linux) can be found     here as well a video file grabber or a file writer class.
    - <b>ICLBlob</b> contains classes for blob detection and tracking and for connected component analysis.
    - <b>ICLFilter</b> provides classes for most common image filters like linear filters and morphological operators.
    - <b>ICLQuick</b> provides almost 100 functions and functors for rapid prototyping
    - <b>ICLGeom</b> contains classes for 3D-modelling and camera calibration. 
    - <b>ICLQt*</b> contains a Qt-4 based GUI-API that facilitates creation of simple and complex GUI 
      applications significantly. And of course a powerful image     visualisation widget called ICLWidget is provided.
    - <b>ICLAlgorithms</b> contains high level classes like a hough-transformation-based line detector or generic self organizing map (SOM) implementation. 
    - <b>ICLOpenCV*</b> offers functions for shallow and deep copies from ICL-images types into OpenCV's images types and v.v.
    
    (*) The packages ICLQt and ICLOpenCV depend compulsorily on the corresponding external software dependencies Qt4 and OpenCV. 
    Consequently these packages are not available if these dependencies are missing.
    
    </TD><TD>
    \image html icl-components.png "ICL's component collaboration diagram"
    </TD></TR></TABLE>

    \section THE_IMAGE The Image Classes
    

    <TABLE border=0><TR><TD>

    We use inheritance and class templates for ICL's image representation:
    The ImgBase class defines an abstract interface, that manages all image information except image pixel data. These abstract image features are:

    - size (in pixels)
    - channel count  (see <b>channel concept</b>)
    - type of pixels (see <b>data types</b>)
    - color format (see <b>color formats</b>)
    - raw image data access
    - Region of Interest (see <b>Region of Interests</b> \ref ROI (ROI)) 
    - a time stamp 

    The ImgBase interface is implemented by the template class Img<T> which implements all abstract ImgBase-functions and aggregates a 
    vector of planar image channel data pointers. Internally, these channel data pointers use reference counting to allow shallow image copies. \n
    <b>Img's copy-constructor and assignment operator use shallow copy on default!</b>

    The Img<T> template also adds functions for type-safe data access:

    - access to channel data pointers (using <tt>getData(channel)</tt> or <tt>begin(channel)</tt>)
    - extraction of single image channels (using <tt>operator [](int)</tt>) 
    - extraction of single image pixels (using <tt>operator()(x,y,channel-index)</tt> for single values or <tt>operator()(x,y)</tt> to obtain a pixel vector)
    - iterator based access to data of a given channel (using <tt>begin(channel)</tt> and <tt>end(channel)</tt>)
    - iterator based access to the ROI-pixels of a given channel (using <tt>beginROI(channel)</tt> and <tt>endROI(channel)</tt>)

    </TD><TD>
    \image html image-sketch.png "A sketch of ICL's image type Img<T>"
    </TD></TR></TABLE>
 
    \section SEC_DATA_ORIGN Data Origin
    As most common image formats image processing use the upper left image corner as data origen, ICL follows this convention as well.
    Howerver, many image operation like filtering or thresholding works without regarding the image contents at all. 
    Nonetheless, we suggest to use this standard, as it is particularly important for I/O-routines or image visualization and - not at least - whenever
    discussing about ICL images.

    \section Channel-Concept
    The Img treats images as a stack of image slices -- <b>channels</b>.  Channels
    can be shared by multiple Img instances, which is especially important for fast
    shallow images copies. Actually, it is possible to freely compose existing
    channels (within several "parent images") to another new image.  

    Attention: The newly <i>composed</i> image shares its channel data with
    the original images, such that modifications will effect all images equally.
    In order to get an independent image a deep-copy as well as a so called detach method
    are provided. The latter replaces the "shared" image channel(s) with new 
    independent ones. Shared channel data are stored using the boost-like 
    shared pointer class SmartPtr, which uses reference counting for autonomous garbage collection in order
    to realease <i>unused</i> image channels.


    \section DATA_TYPES Data-Types
    The Img template is not implemented completely inline to reduce compilation expense. Therefore, the Img template 
    is instantiated for the following types T

    - <b>icl8u</b> 8bit unsigned char
    - <b>icl16s</b> 16bit signed integer (short)
    - <b>icl32s</b> 32bit signed integer (int)
    - <b>icl32f</b> 32bit single precision float (float)
    - <b>icl64f</b> 64bit double precision float (double)

    Derived from this types, Img-classes are predefined as follows
    - Img<icl8u> : public ImgBase <b>typedef'd to Img8u</b>
    - Img<icl16s> : public ImgBase <b>typedef'd to Img16s</b>
    - Img<icl32s> : public ImgBase <b>typedef'd to Img32s</b>
    - Img<icl32f> : public ImgBase <b>typedef'd to Img32f</b>
    - Img<icl64f> : public ImgBase <b>typedef'd to Img64f</b>


    Each of these data types has several advantages/disadvantages. The greatest
    disadvantage of the integer types, is their bounded range (e.g. 0-255 for icl8u),
    which has the effect, that all information has to be scaled to this
    range, and all image processing functions must take care that
    no range-overflow occurs during calculation. Furthermore
    the limited range may cause loss of information - particular in 
    complex systems.
    However integer types can often be processed significantly faster. In particular the use of 8-bit unsigned integer images
    relieves the the memory interface due to it's lower memory usage.

    <b>A nice rule of thumb is: If processing speed matters, use Img8u images whenever it's possible and avoid Img64f because 
    double processing is much slower on (still common) 32 bit machines (as long as you do not really need double precision)</b> 

    \section ROI Region of Interest (ROI)
    Each image can be set up with a rectangular region of interest. Nearly all algorithms 
    work only on the pixels within the ROI. If a function does not support
    ROI handling it will be noticed in the documentation. There are several ways
    to realize ROI handling in functions. The most common way is to use the
    ImgIterator with can be accessed using the STL-style functions beginROI(channel) and
    endROI(channel). 

    \section Color Formats
    An ImgBase image provides information about the (color) format, that
    is associated with the image data represented by the images channels. Color
    is written in brackets, as not all available formats imply color-information.
    The most known color space is probably the RGB color space. 
    If an ImgBase image has the format <i>formatRGB</i>, than this implies the 
    following:
    - the image has exactly 3 channels
    - the first channel contains RED-Data in range [0,255]
    - the second channel contains GREEN-Data in range [0,255]
    - the third channel contains BLUE-Data in range [0,255]

    All additional implemented functions and classes regard this information. 
    The currently available Img formats are member of the enum Format.
    A special format: formatMatrix can be used for arbitrary purpose.


    \section CONST_SEC Const-Concept
    ICL Images use the const concept of C++ to ensure pixel data of
    const Images (of type const ImgBase or more precisely const Img<T>)
    is not changed, i.e. it is only accessible for reading. Unfortunately
    this leads to a conflict with the "shallow-copy" concept of ICL images.
    \code
    void func(const Img8u &image){
      // given image is const -> data must not be changed
      Img8u x = image;
      // x is a shallow copy of image (data is shared)
      x.clear();
      // this affects also the data of image (which shall not
      // be permitted
    }
    \endcode
    To avoid this conflict, we tried to forbid creating un-const shallow
    copies of const images by implementing no default copy constructor:
    \code
    Img<T>(const Img<T> &other) {... }
    \endcode
    but an un-const version of this:
    \code
    Img<T>(Img<T> &other) {... }
    \endcode
    Here we face some GCC related problem, because gcc is not able
    for an implicit cast of an Img<T> to an Img<T>& in constructor
    calls:
    \code
    template<class T> class Img<T>{ ... };

    Img8u create_image(){ 
      return Img8u(); 
    }

    int main(){
      Img8u a = create_image();
    }
    \endcode
    Here, the compiler gives error: "Can't find constructor Img<T>(Img<T>)".
    In fact, this constructor can not exist: it must have the following
    syntax: Img<T>(Img<T>&)

    Probably further gcc versions will fix this problem!

    <b>Until then, we accept the const leak at constructor and assignment
    operator and reimplemented them as ..(const Img<T> &other) </b>


    \section IPP_MKL IPP/MKL-optimization
    The Intel Integrated Performance Primitives (Intel IPP) and the Intel Math Kernel Library (Intel MKL) are assembler libraries that provide a C-interface 
    to a large set of highly optimized and hardware accelerated functions for image processing, and 
    other numerical problems for all processors providing MMX and SSE instruction sets,
    i.e. most common Intel and AMD processors.
    As far as we know, Intel IPP and Intel MKL can be used freely for non-commercial use,<b> but not for research</b>. 
    Fortunately, IPP/MKL support is purely optional.
    Therefore you can simply develop your application with an ICL-build without IPP/MKL-optimization and re-link it against an optimized ICL-build lateron.

    \subsection IPP Intel IPP
    If Intel IPP is available, it is highly integrated into ICL:

    - a large number of image processing functions are IPP-accelerated
    - iclXX data types are typedef'ed to ippXX data types rather than to default types from the \<stdint.h\> header 
    - icl::Size extends the IppSize struct which enables the programmer to pass an icl::Size instance directly to an ipp-function call
    - the same is true for icl::Point and icl::Rect
    - all ipp-headers will we available, so IPP-functions can be used directly

    We tuned the Img-class to facilitate the use of IPP functions.

    \subsection MKL Intel MKL

    In contrast to the matrix package for small matrices, which is optimized for matrices up to dimensions of 6x6, Intel MKL is optimized for larger matrices. 
    As under certain conditions, MKL is more then 100 times faster, we decided to add MKL support as well. However, MKL is currently only 
    used in the implementation of some DynMatrix multiplication functions in the ICLUtils package.

    \section ICLCore Modules
    If you like to explore the ICLCore documentation by your own, take a 
    look a the following sub modules:\n
    -# \ref TYPES 
    -# \ref GENERAL
    -# \ref IMAGE 
    -# \ref STRUTILS
    -# \ref MATH 


    \section EXTERNAL_DEPS Optional 3rd Party Dependencies

    As mentioned above, ICL has a set of external 3rd party software dependencies. All of these dependencies are purely optional for compiling
    the ICL libraries, however some special functionalities depend compulsorily on certain 3rd party libraries. In the following, 
    dependent functions, classes and packages are listed:

    <TABLE border=1>
    <TR><TD>External Package</TD><TD>Version</TD>                  <TD>proprietary</TD><TD>Resource Location</TD><TD>Ubuntu Package</TD><TD>Dependend ICL Components</TD></TR>
    <TR><TD>Intel IPP</TD>     <TD>6.0 (maybe others)</TD>         <TD>yes</TD> 
    <TD><a href="http://software.intel.com/en-us/intel-ipp/">Intel Webpage</a></TD>    <TD>not available</TD>
    <TD><ul>
    <li> Most basic image pocessing functions are optionally accellerated using an Intel IPP backend
    <li> ICLFilter:CannoOp class (Canny edge detector) [no fallback]
    </ul></TD></TR>
    <TR><TD>Intel MKL</TD>     <TD>10.2 (maybe others)</TD>        <TD>yes</TD> 
    <TD><a href="http://software.intel.com/en-us/intel-mkl/">Intel Webpage</a></TD>    <TD>not available</TD>
    <TD><ul>
    <li> ICLUtils:big_matrix_mult_t (efficient matrix multiplication for large matrices) [fallback: IPP and C++]
    </ul></TD></TR>
    <TR><TD>libdc1394</TD>     <TD>2.x (1.x is not supported)</TD> <TD>no</TD>  
    <TD><a href="http://sourceforge.net/projects/libdc1394">Sourceforge</a></TD>       <TD>libdc1394-2x-dev</TD>
    <TD><ul>
    <li> ICLIO:DCGrabber (FireWire Camera Support) [fallback: ICLIO:UnicapGrabber]
    <li> "dc" plugin for ICLIO:GenericGrabber [fallback: "unicap"-plugin]
    </ul></TD></TR>
    <TR><TD>OpenCV</TD>        <TD>1.x and 2.x</TD>                <TD>no</TD>  
    <TD><a href="http://sourceforge.net/projects/opencvlibrary/">Sourceforge</a></TD>  <TD>libopencv-dev</TD>
    <TD><ul>
    <li> ICLOpenCV:image converters [no fallback]
    </ul></TD></TR>
    <TR><TD>OpenGL</TD>        <TD>any common</TD>                 <TD>no</TD>  
    <TD>Graphics card vendor</TD>                                                      <TD>libgl-dev</TD>
    <TD><ul>
    <li> Qt dependency depends also on libGL (But GL-headers are needed as well)
    <li> The whole ICLQt-package and all GUI-applications [no fallback]
    </ul></TD></TR>
    <TR><TD>Qt</TD>            <TD>4.4 (or higher)</TD>            <TD>no</TD>
    <TD><a href="http://qt.nokia.com/downloads">Nokia-Qt Webpage</a></TD>              <TD>libqt4-dev and qt4-dev-tools</TD>
    <TD><ul>
    <li> The whole ICLQt-package and all GUI-applications [no fallback]
    </ul></TD></TR>
    <TR><TD>Unicap</TD>        <TD>we use 0.2.19 maybe others</TD> <TD>no</TD>
    <TD><a href="http://sourceforge.net/projects/unicap/">Sourceforge</a></TD>         <TD>formerly: unicap</TD>
    <TD><ul> 
    <li> ICLIO:UnicapGrabber (firewire and video 4 linux Support) [firewire fallback: ICLIO:DCGrabber]
    <li> "unicap" plugin for ICLIO:GenericGrabber [firewire fallback: "dc"-plugin]
    </ul></TD></TR>
    <TR><TD>libMagick++</TD>   <TD>any common</TD>                 <TD>no</TD>
    <TD><a href="http://www.graphicsmagick.org/Magick++/">Sourceforge</a></TD>         <TD>libgraphicsmagick++1-dev and graphicsmagick-libmagick-dev-compat</TD>
    <TD><ul>
    <li> Support for most file formats [fallback: C++ for .ppm, .pnm, .pgm, .icl, and .csv (and .gz-versions of all), libjepg for .jpg]
    <li> No fallback support for other very common formats like .png, .giv, .bmp, ...
    </ul></TD></TR>
    <TR><TD>libjpeg</TD>       <TD>any common</TD>                 <TD>no</TD>
    <TD><a href="http://sourceforge.net/projects/libjpeg/">Sourceforge</a></TD>        <TD>libjpeg-dev</TD>
    <TD><ul>
    <li> Support for .jpg (or .jpeg) files [fallback: libMagick++ (which maybe needs libjepg as well)]
    </ul></TD></TR>
    <TR><TD>libz</TD>          <TD>any common</TD>                 <TD>no</TD>
    <TD>Operating System Basics</TD>                                                   <TD>libz-dev</TD>
    <TD><ul>
    <li> Zip-Compression/Decompression support for class ICLIO:File. 
    <li> Zip-Compression/Decompression support for built-in image formats .pnm, .pgm, .ppm, .csv and .icl
    </ul></TD></TR>
    <TR><TD>videodev.h</TD>    <TD>from 2.4.x or 2.6.x kernel</TD> <TD>no</TD>  
    <TD>Linux kernel header</a></TD>                                                   <TD>linux-headers-2.x.x.x-generic</TD>
    <TD><ul>
    <li> ICLIO:PWCGrabber (Native Support for Phillips 740 Webcams) [fallback: maybe unicap]
    </ul></TD></TR>
    <TR><TD>SVS</TD>           <TD>4.2 (maybe others)</TD>         <TD>yes</TD> 
    <TD><a href="http://www.videredesign.com/templates/svs.htm">Videre Design</a></TD> <TD>not available</TD>
    <TD><ul>
    <li> ICLAlgorithms:SVS stereo vision support class
    </ul></TD></TR>
    <TR><TD>libXine</TD>       <TD>any common</TD>                 <TD>no</TD>  
    <TD><a href="http://sourceforge.net/projects/xine/">Sourceforge</a></TD>           <TD>libxine-dev</TD>
    <TD><ul>
    <li> ICLIO:VideoGrabber class (video image source support) [no fallback]
    <li> "video" plugin for ICLIO:GenericGrabber [no fallback]
    </ul></TD></TR>
    <TR><TD>XCF-Framework</TD> <TD>any common</TD>                 <TD>no</TD>  
    <TD><a href="http://sourceforge.net/projects/xcf/files/">Sourceforge</a></TD>       <TD>not available</TD>
    <TD><ul>
    <li> Sending and receiving images via TCP network interface [no fallback]
    <li> ICLIO:XCFServerGrabber (RMI-based image acquisition)
    <li> ICLIO:XCFPublisherGrabber (Subscriber for published images)
    <li> ICLIO:XCFMemoryGrabber (image acquisition for ActiveMemory instance)
    <li> ICLIO:GenericGrabber plugins "xcfp", "xcfs", and "xcfm"
    <li> ICLIO:icl-xcf-publisher application
    <li> ICLIO:MemoryListener utility class
    <li> Global utility functions from ICLIO:XCFUtils header
    </ul></TD></TR>    
    </TABLE> 


    \section COMPLEX_EXAMPLE A More Complex Example for Interactive Image Thresholding

    <TABLE border=0><TR><TD>
    \code
    #include <ICLQuick/Common.h>

    GUI gui;

    void init(){
      gui << "image[@handle=vis@minsize=16x12]"
          << "slider(0,255,127)[@out=thresh@label=threshold@maxsize=100x2]";
      gui.show();
    }

    void run(){
      gui_ImageHandle(vis);
      gui_int(thresh);
  
      static GenericGrabber g(FROM_PROGARG("-input"));
  
      // we need a working copy of acquired images for inplace thresholding
      static Img8u image;
      g.grab()->convert(&image);
  
      // Of course, we could use an instance ThresholdOp here,
      // but here we demonstrate pixel-access as well
      for(int c=0;c<image.getChannels();++c){
        for(int y=0;y<image.getHeight();++y){
          for(int x=0;x<image.getWidth();++x){
            image(x,y,c) = 255 * (image(x,y,c) > thresh);
          }
        }
      }
      vis = image;
      vis.update();
    }

    int main(int n, char **args){
      return ICLApplication(n,args,"-input(2)",init,run).exec();
    }
    \endcode

    </TD><TD>
    \image html thresh.jpg
    </TD></TR></TABLE>

    **/ #endif
