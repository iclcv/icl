/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCC/Color.h                                  **
** Module : ICLCC                                                  **
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

#ifndef ICL_COLOR_H
#define ICL_COLOR_H

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/FixedVector.h>
#include <string>

namespace icl{

  /// Default color type of the ICL
  typedef FixedColVector<icl8u,3> Color;
  
  /// Special color type for float valued colors
  typedef FixedColVector<icl32f,3> Color32f;


  /// Special color type for e.g. rgba color information
  typedef FixedColVector<icl8u,4> Color4D;

  /// Special color type for e.g. rgba color information (float)
  typedef FixedColVector<icl32f,4> Color4D32f;
  
  // Create a color by given name (see GeneralColor Constructor)
  const Color &iclCreateColor(std::string name);
  
  /// Creates a (by default 20 percent) darker color 
  inline Color darker(const Color &c, double factor=0.8){
    return c*factor;
  }
  
  /// Creates a (by default 20 percent) lighter color 
  inline Color lighter(const Color &c,double factor=0.8){
    return c/factor;
  }

  /// Parses a color string representation into a color structur
  /** If an error occurs, a warning is shown and black color is returned 
      first checks for some default color names:
      - black
      - white
      - red
      - green
      - blue
      - cyan
      - magenta
      - yellow
      - gray50
      - gray100
      - gray150
      - gray200
  */
  Color color_from_string(const std::string &name);

}
#endif
