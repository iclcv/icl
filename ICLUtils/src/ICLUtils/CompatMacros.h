/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/CompatMacros.h                   **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

#include <ICLUtils/ICLConfig.h>

#ifdef WIN32
#define ICL_SYSTEMCALL_RM "del "
#else
#define ICL_SYSTEMCALL_RM "rm -rf "
#endif

#ifdef WIN32
  #define NOMINMAX
  #define _USE_MATH_DEFINES
  #define M_PI 3.14159265358979323846
  // TODOWW: test with _MSC_VER == 1700
  #if (defined _MSC_VER && _MSC_VER < 1800)
    #include <cmath>
    inline double round(double a)
    {
      return floor(a + 0.5f);
    }
    inline double log2(double a)  
    {
      return log(a) / 0.69314718055994530943;  
    }
    inline float pow(int a, int b)  
    {
      return pow((float)a, b);  
    }
    inline double pow(float a, double b)
    {
      return pow((double)a, b);
    }
    inline int rint(double a)  
    {
      // this is not really what it should do
      return (int)round(a);  
    }
    inline float log(int a)  
    {
      return log((float)a);  
    }
    inline float exp(int a)  
    {
      return exp((float)a);  
    }
    inline float sqrt(int a)
    {
      return sqrt((float)a);
    }
  #endif
  // in windows use this instead of #warning
  #define WARNING(msg) message(__FILE__ "(" STRINGSIZE(__LINE__) ") : warning: " #msg)
#endif


#ifdef ICL_SYSTEM_WINDOWS
#	define IPP_DECL __stdcall
#else
#	define IPP_DECL
#endif


/// this macros are important for creating dll's

#ifdef WIN32

#ifdef ICLUtils_EXPORTS
#define ICLUtils_API   __declspec(dllexport)
#else
#define ICLUtils_API   __declspec(dllimport)
#endif

#ifdef ICLMath_EXPORTS
#define ICLMath_IMP
#define ICLMath_API   __declspec(dllexport)
#else
#define ICLMath_IMP   __declspec(dllimport)
#define ICLMath_API   __declspec(dllimport)
#endif

#ifdef ICLCore_EXPORTS
#define ICLCore_API   __declspec(dllexport)
#else
#define ICLCore_API   __declspec(dllimport)
#endif

#ifdef ICLFilter_EXPORTS
#define ICLFilter_API   __declspec(dllexport)
#else
#define ICLFilter_API   __declspec(dllimport)
#endif

#ifdef ICLIO_EXPORTS
#define ICLIO_API   __declspec(dllexport)
#else
#define ICLIO_API   __declspec(dllimport)
#endif

#ifdef ICLIO_EXPORTS
#define ICLIO_API   __declspec(dllexport)
#else
#define ICLIO_API   __declspec(dllimport)
#endif

#ifdef ICLCV_EXPORTS
#define ICLCV_API   __declspec(dllexport)
#else
#define ICLCV_API   __declspec(dllimport)
#endif

#ifdef ICLQt_EXPORTS
#define ICLQt_API   __declspec(dllexport)
#else
#define ICLQt_API   __declspec(dllimport)
#endif

#ifdef ICLGeom_EXPORTS
#define ICLGeom_API   __declspec(dllexport)
#else
#define ICLGeom_API   __declspec(dllimport)
#endif

#ifdef ICLMarkers_EXPORTS
#define ICLMarkers_API   __declspec(dllexport)
#else
#define ICLMarkers_API   __declspec(dllimport)
#endif

#else
#define ICLUtils_API
#define ICLMath_IMP
#define ICLMath_API
#define ICLCore_API
#define ICLFilter_API
#define ICLIO_API
#define ICLCV_API
#define ICLQt_API
#define ICLGeom_API
#define ICLMarkers_API
#endif

#ifdef ICL_HAVE_OPENGL
  #ifdef ICL_SYSTEM_APPLE
    #include <OpenGL/glew.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
  #elif ICL_SYSTEM_WINDOWS
    #include <Windows.h>
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
  #else
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
  #endif
#endif
