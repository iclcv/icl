/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Size.h                           **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
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
#include <string>
#include <iostream>

#ifdef ICL_HAVE_IPP
#include <ipp.h>
#endif

namespace icl {
  namespace utils{
  #ifndef ICL_HAVE_IPP
    /// fallback implementation for the IppiSize struct, defined in the ippi lib
    struct IppiSize {
      // width
      int width;

      // height
      int height;
    };

  #else
  #endif

    /** \cond */
    class Size32f;
    /** \endcond */

    /// Size class of the ICL
    class ICLUtils_API Size : public IppiSize{
      public:
      /// null is w=0, h=0
      static const Size null;

      /// Quater QVGA res. 160x120
      static const Size QQVGA;

      /// Color Graphics Adapter res. 320x200
      static const Size CGA;

      /// Qauter VGA res. 320x240
      static const Size QVGA;

      /// Half VGA res. 480x320
      static const Size HVGA;

      /// Enhanced Graphics Adapter res. 640x350
      static const Size EGA;

      /// Video Graphics Array res. 640x480
      static const Size VGA;

      /// Wide VGA res. 800x480
      static const Size WVGA;

      /// Super VGA res. 800x600
      static const Size SVGA;

      /// Quater HD res. 960x540
      static const Size QHD;

      /// Double VGA res. 960x640
      static const Size DVGA;

      /// Extended Graphics Array res. 1024x768 (also known as EVGA)
      static const Size XGA;

      /// XGA Plus res. 1152 x 864
      static const Size XGAP;

      /// Double Super VGA res. 1200x800
      static const Size DSVGA;

      /// Half Definition res. 1280x720
      static const Size HD720;

      /// Wide XGA res. 1280x800
      static const Size WXGA;

      /// Wide XGA Plus res. 1440x900
      static const Size WXGAP;

      /// Quad VGA res. 1280x960
      static const Size SXVGA;

      /// Super XGA res. 1280x1024
      static const Size SXGA;

      /// Wide Super XGA res. 1600x900
      static const Size WSXGA;

      /// Super XGA Plus res. 1400x1050
      static const Size SXGAP;

      /// Wide Super XGA Plus res. 1600x1050
      static const Size WSXGAP;

      /// Ultra XGA res. 1600x1200
      static const Size UXGA;

      /// High definition res. 1920x1080
      static const Size HD1080;

      /// Wide UXGA res. 1920x1080
      static const Size WUXGA;

      /// Ultra Definietion res. 3840x2160
      static const Size UD;


      // video formats

      /// Common Intermediate Format res. 352x288
      static const Size CIF;

      /// Source Imput Format res. 360x240
      static const Size SIF;

      /// Semi Quater CIF res. 128x96
      static const Size SQCIF;

      /// Quater CIF res. 176x144
      static const Size QCIF;

      /// Phase alternating Line res. 768x576 (many other formats are known as PAL)
      static const Size PAL;

      /// National Television System Commitee res. 640x480 (many other formats are known as NTSC)
      static const Size NTSC;

	    /// default constructor
	    inline Size(){ this->width = 0; this->height = 0; }

      /// deep copy of another Size
      inline Size(const Size &s){ this->width = s.width;this->height = s.height; }

      /// creates a specified size
      inline Size(int width,int height){ this->width = width; this->height = height; }

      /// creates a size from given string (e.g. VGA, CIF, or 1024x768)
      explicit Size(const std::string &name);

      /// creates a size from given float precision size (values are rounded)
      Size(const Size32f &other);

      /// checks wether the object instance is null, i.e. all elements are zero
      bool isNull() const { return (*this)==null; }

      /// checks if two sizes are equal
      bool operator==(const Size &s) const {return width==s.width && height==s.height;}

      /// checks if two size are not equal
      bool operator!=(const Size &s) const {return width!=s.width || height!=s.height;}

      /// add a size to another size
      Size operator+(const Size &s) const {return  Size(width+s.width,height+s.height);}

      /// substracts a size from another size
      Size operator-(const Size &s) const {return  Size(width-s.width,height-s.height);}

      /// scales the size by a scalar value
      Size operator*(double d) const { return Size((int)(d*width),(int)(d*height));}

      /// scales the size by a scalar value
      Size operator/(double d) const { return (*this)*(1.0/d); }

      /// adds another size inplace
      Size& operator+=(const Size &s){width+=s.width; height+=s.height; return *this;}

      /// substracst another size inplace
      Size& operator-=(const Size &s){width-=s.width; height-=s.height; return *this;}

      /// scales the size parameters inplace by a scalar
      Size& operator*=(double d) {width=(int)((float)width*d); height=(int)((float)height*d); return *this;};

      /// scales the size parameters inplace by a scalar
      Size& operator/=(double d) { return (*this)*=(1.0/d); }

      /// reutrns width*height
      int getDim() const {return width*height;}
    };

    /// ostream operator WIDTHxHEIGHT
    ICLUtils_API std::ostream &operator<<(std::ostream &os, const Size &s);

    /// istream operator parses a size from a string
    /** also called in Size::Size(const std::string&)**/
    ICLUtils_API std::istream &operator>>(std::istream &is, Size &s);

  } // namespace utils
}// namespace icl

