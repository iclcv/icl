/** \mainpage ICL Documentation for All ICL Packages 

\section COMBINED_DOC ICL's Combined Documentation

ICL's documentation is split into the sub-packages. Each subpackage provides an own documentation that can be entered by clicking the corrensponding link
in the pages main menu at the left. Each sub-package documentation results from an independent doxygen project. Therefore the packages are not linked
with each others. This <em>combined documentation</em> contains all classes, functions and headers of ICL, which allows doxygen to interlink all elements
completely. However this combined documentation is unsuitable for exploration of certain ICL packages.

\section PACKAGES ICL Packages

  <TABLE border=0><TR><TD>
    ICL consists of currently 11 packages that are listed in the main menu at the left.

    - <b>ICLUtils</b> Contains general purpose functions and classes that are currently not part of the C++-STL (e.g. threads or matrices).
    - <b>ICLCore</b> basically provides class definitions for ICL's image classes Img and ImgBase and related global functions.
    - <b>ICLCC</b> provides functions and classes for color conversion.
    - <b>ICLIO</b> extends the range of functions by input and output classes. Camera grabbers different camera 
      types (e.g. IEEE-1394 or Video-4-Linux) can be found here as well a video file grabber or a file writer class.
    - <b>ICLBlob</b> contains classes for blob detection and tracking and for connected component analysis.
    - <b>ICLFilter</b> provides classes for most common image filters like linear filters and morphological operators.
    - <b>ICLQuick</b> provides almost 100 functions and functors for rapid prototyping
    - <b>ICLGeom</b> contains classes for 3D-modelling and camera calibration. 
    - <b>ICLQt*</b> contains a Qt-4 based GUI-API that facilitates creation of simple and complex GUI 
      applications significantly. And of course a powerful image visualisation widget called ICLWidget is provided.
    - <b>ICLAlgorithms</b> contains high level classes like a hough-transformation-based line detector or generic self organizing map (SOM) implementation. 
    - <b>ICLOpenCV*</b> offers functions for shallow and deep copies from ICL-images types into OpenCV's images types and v.v.
    
    (*) The packages ICLQt and ICLOpenCV depend compulsorily on the corresponding external software dependencies Qt4 and OpenCV. 
    Consequently these packages are not available if these dependencies are missing.
    
    </TD><TD>
    \image html icl-components.png "ICL's component collaboration diagram"
    <A href="block-scheme.html">(browse)</A>
    </TD></TR></TABLE>

*/
