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

#ifdef ICL_SYSTEM_WINDOWS
  #define NOMINMAX
  #define _USE_MATH_DEFINES
  //#define __msxml_h__ // icl has its own xml classes
  //#include <Windows.h>
#endif


#ifdef SYSTEM_WINDOWS
#	define IPP_DECL __stdcall
#	ifndef _USE_MATH_DEFINES
#	define _USE_MATH_DEFINES
#	endif
#	define rint  ::System::Math::Round
#	define round ::System::Math::Round
#else
#	define IPP_DECL
#endif


/// this macros are important for creating dll's

#ifdef ICL_SYSTEM_WINDOWS

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
#define ICL_IO_API   __declspec(dllexport)
#else
#define ICL_IO_API   __declspec(dllimport)
#endif

#ifdef ICLIO_EXPORTS
#define ICL_IO_API   __declspec(dllexport)
#else
#define ICL_IO_API   __declspec(dllimport)
#endif

#ifdef ICLCV_EXPORTS
#define ICL_CV_API   __declspec(dllexport)
#else
#define ICL_CV_API   __declspec(dllimport)
#endif

#ifdef ICLQt_EXPORTS
#define ICL_QT_API   __declspec(dllexport)
#else
#define ICL_QT_API   __declspec(dllimport)
#endif

#ifdef ICLGeom_EXPORTS
#define ICL_Geom_API   __declspec(dllexport)
#else
#define ICL_Geom_API   __declspec(dllimport)
#endif

#ifdef ICLMarkers_EXPORTS
#define ICLMarkers_API   __declspec(dllexport)
#else
#define ICLMarkers_API   __declspec(dllimport)
#endif

#else
#define ICLUtils_API
#define ICLMath_IMP
#define ICLMath_IMP
#define ICLCore_API
#define ICLFilter_API
#define ICL_IO_API
#define ICL_CV_API
#define ICL_QT_API
#define ICL_Geom_API
#define ICLMarkers_API
#endif

