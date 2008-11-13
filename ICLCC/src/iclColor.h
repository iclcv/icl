#ifndef ICL_COLOR_H
#define ICL_COLOR_H

#include <iclCore.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iclStringUtils.h>
#include <iclFixedVector.h>
namespace icl{

  /// Default color type of the ICL
  typedef FixedColVector<icl8u,3> Color;
  
  /// Special color type for float valued colors
  typedef FixedColVector<icl32f,3> Color32f;


  /// Special color type for e.g. rgba color information
  typedef FixedColVector<icl8u,4> Color4D;

  /// Special color type for e.g. rgba color information (float)
  typedef FixedColVector<icl32f,3> Color4D32f;
  
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
  */
  Color translateColor(const std::string &s);

  /// Creates a (r,g,b) string representation for a color structure
  std::string translateColor(const Color &color);

}
#endif
