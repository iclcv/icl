/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/QuickDocumentation.h                   **
** Module : ICLCV                                                  **
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

/**
    H1 ICLQuick - an ICLPackage for rapid prototyping

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
    #include <ICLQt/Quick.h>
    // no using namespace etc, ICLQuick automatically
    // uses namespace icl and std
    int main(){
      ImgQ b;
      for(int i=0;i<10;i++){
        ImgQ a;
        for(int j=0;j<10;j++){
           // horizontal concatenation using the comma operator
           a = (a,scale(grab("dc","0"),0.1));
        }
        // vertical concatenation of a the "10xa-row" to the current b
        b=(b%a);
      }
      // visualize b using icl-xv
      show(b);
    }
    \endcode
    </td><td>
    \image html quick-example-1.png "Image concantenation using ICLQuick"
    </td></tr></table>

    \section AFFINITY Template Functions with Affinity (to icl32f)

    Most ICLQuick functions are implemented twice -- as template and
    as non-template with ImgQ parameters/return type. By this means,
    the template functions behave as if a default template parameter
    was given (which is not possible in the current C++-standard).

    Implementation example:
    \code
    #include <iostream>
    #include <typeinfo>

    template<class T> std::string name(){ return "??"; }
    template<> std::string name<int>(){ return "int"; }
    template<> std::string name<float>(){ return "float"; }

    template<class T>
    T foo(){
       std::cout << "foo<T> T=" << name<T>() << std::endl;
    }

    int foo(){
      std::cout << "XX" ;
      foo<int>();
    }

    int main(){
      foo();
      foo<float>();
      foo<int>();
    }
    \endcode

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
#include <ICLQt/Common.h>

GUI gui;
GenericGrabber grabber;
ImgQ last;  // last image

void init(){
  gui << Image().handle("image")
      << Slider(0,255,127).out("thresh")
      << Show();
  grabber.init(pa("-i"));
}

void run(){
  // use cvt to create an Img32 (aka ImgQ)
  ImgQ curr = cvt(grabber.grab());
  // nested use of operators
  gui["image"] = thresh(abs(last-curr),gui["thresh"]);
  last = curr;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input|-i(2)",init,run).exec();
}

\endcode

    </td><td>
    \image html quick-example-2.png "Arithmetical operations for creation of difference images"
    </td></tr></table>
 **/



