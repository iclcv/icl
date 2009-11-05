#ifndef ICL_QUICK_DOCUMENTATION_H
#define ICL_QUICK_DOCUMENTATION_H

/** 
    \mainpage ICLQuick - an ICLPackage for rapid prototyping 
    
    \section sec1 Overview
    In contrast to all other ICLPackages the ICLQuick package
    focuses mainly on the programmers convenience. It should
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
    The following demo application demonstrates how powerful
    The ICLQuick functions are, if no real-time performance is
    required:
    \code
    
    
    int main(){
      ImgQ b;
      for(int i=0;i<10;i++){
        ImgQ a;
        for(int j=0;j<10;j++){
          // right-side concatenation of a scaled pwc-image to a
          a = (a,scale(pwc(),0.1)); 
        }
        // bottom side concatenation of a the "10xa-row" to the current b
        b=(b%a);
      }
      // visualize b using xv
      show(b);    
      return 0;
    }
    \endcode
    
    \section sec3 Essential functions / features
    
    - <b>zeros, ones, load, create, pwc and ieee</b>\n
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
    
 **/



#endif
