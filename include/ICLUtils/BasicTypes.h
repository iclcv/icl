/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/BasicTypes.h                          **
** Module : ICLUtils                                               **
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

#ifndef ICL_BASIC_TYPES_H
#define ICL_BASIC_TYPES_H

#ifdef HAVE_IPP
#include <ipp.h>
#endif

#include <stdint.h>


namespace icl {
  
#ifdef HAVE_IPP
  /// 64Bit floating point type for the ICL \ingroup TYPES
  typedef Ipp64f icl64f;

  /// 32Bit floating point type for the ICL \ingroup TYPES 
  typedef Ipp32f icl32f;

  /// 32bit signed integer type for the ICL \ingroup TYPES
  typedef Ipp32s icl32s;

  /// 16bit signed integer type for the ICL (range [-32767, 32768 ]) \ingroup TYPES
  typedef Ipp16s icl16s;
  
  /// 8Bit unsigned integer type for the ICL \ingroup TYPES
  typedef Ipp8u icl8u;

#else
  /// 64Bit floating point type for the ICL \ingroup TYPES
  typedef double icl64f;

  /// 32Bit floating point type for the ICL \ingroup TYPES
  typedef float icl32f;

  /// 32bit signed integer type for the ICL \ingroup TYPES
  typedef int32_t icl32s;
  
  /// 16bit signed integer type for the ICL (range [-32767, 32768 ]) \ingroup TYPES
  typedef int16_t icl16s;

  /// 8Bit unsigned integer type for the ICL \ingroup TYPES
  typedef uint8_t icl8u;

#endif

  /// 32bit unsigned integer type for the ICL \ingroup TYPES
  typedef uint32_t icl32u;

  /// 16bit unsigned integer type for the ICL \ingroup TYPES
  typedef uint16_t icl16u;
}

#endif // include guard
