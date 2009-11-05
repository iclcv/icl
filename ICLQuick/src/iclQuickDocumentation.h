#ifndef ICL_QUICK_DOCUMENTATION_H
#define ICL_QUICK_DOCUMENTATION_H

/** 
    \mainpage ICLQuick - an ICLPackage for rapid prototyping 
    
    \section sec1 The ICLQuick Package (Overview)
    In contrast to all other ICL-packages, the ICLQuick package
    focuses only on the programmers convenience. It should
    be used for rapid prototyping or for test applications.\n
    Consider a default test or benchmark application: The 
    programmer has to do several things that are identical for
    most of this kind of application:
    - including all necessary headers
    - making use of the namespaces "icl" and "std"
    - acquiring source images / creating a set of source images
    - running the tests / benchmarks ( this is the most variable 
      point)
    - visualizing the result image or images 
    
    The ICLQuick package provides the most common functions 
    and operators to create the desired result application 
    in much less time, that is even much better readable!
    
    \section sec2 Example
    
    <table border=0><tr><td>
    The following demo application demonstrates how powerful
    The ICLQuick functions are, if no real-time performance is
    required:
    \code
    \#include <iclQuick.h>
    // no using namespace etc, ICLQuick automatically 
    // uses namespace icl and std
    int main(){
      ImgQ b;
      for(int i=0;i<10;i++){
        ImgQ a;
        for(int j=0;j<10;j++){
           // right-side concatenation of a scaled snapshot from dc camera
           a = (a,scale(grab("dc","0"),0.1)); 
        }
        // bottom side concatenation of a the "10xa-row" to the current b
        b=(b%a);
      }
      // visualize b using icl-xv
      show(b);    
    }
    \endcode
    </td><td>
    \image html quick-example-1.png "Image concantenation using ICLQuick"
    </td></tr></table>
    \section sec3 Essential functions / features
    
    - <b>zeros, ones, load, create, grab, pwc and ieee</b>\n
      creating images from different sources 
    - <b>filter,scaled, cc, levels, copy, flipx, ... </b>\n
      filtering functions, creating a new image from a given source image
    - <b>save, show and print</b>\n
      output functions
    - <b>arithmetical operators +,-,*,/</b>\n
      defined for two images as well as for images and constants
    - <b>arithmetical functions sqr, sqrt, exp, ln, and abs </b>\n
    - <b>logical operators || and && </b>\n
    - <b>concatenation operators "," and "%" </b>\n
      the ","-operator concatenates images horizontally,
      the "%"-operator concatenates images vertically.
    - <b>The roi-access function </b>\n
      for copying into an images ROI just use the roi function\n
      "roi(ImgA) = ImgB;" or "roi(ImgA) = 255;"  
    
    \section ARITH Pixel-wise Arithmetical and Logical Operations
 
 <table border=0><tr><td>   
    Another essential set of ICLQuick-functions are the arithmetical and logical 
    functions and operators. Instead for creating UnaryOp instances from the ICLFilter
    package, in non-realtime applications, images can simply be added or multiplied
    pixel-wise using the common operators '+','*',etc\n

    Internally, most arithmetical and logical operations use a static
    memory managing system to avoid most memory allocation calls at run-time.
    
    
\code
#include <iclCommon.h>

GUI gui;

void init(){
  gui << "image[@handle=image@minsize=16x12]"
      << "slider(0,255,127)"
         "[@out=t@maxsize=100x2@label=threshold]";
  
  gui.show();
}

void run(){
  static ImgQ last;
  ImgQ curr = cvt(grab(FROM_PROGARG("-input")));
  
  gui["image"] = thresh(abs(last-curr),gui["t"].as<int>());
  gui["image"].update();
  last = curr;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
}

\endcode
    
    </td><td>
    \image html quick-example-2.png "Arithmetical operations for creation of difference images"
    </td></tr></table>
 **/



#endif
