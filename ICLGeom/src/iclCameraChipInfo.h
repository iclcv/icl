#ifndef ICL_CAMERA_CHIP_INFO_H
#define ICL_CAMERA_CHIP_INFO_H

#include <iclPoint32f.h>
#include <iclSize32f.h>
#include <iclException.h>
#include <string>
#include <iostream>

namespace icl{

  /// Utility structure for camera chip description
  /** The camera chip description must be given to Camera structure,
      if it shall be used to compute ViewRays. A CameraChipDescription
      instance contains currently only a size, which internally
      is always the chip size in mm  **/ 
  struct CameraChipInfo{
    Size32f size; // internal chip size

    /// create a null instance
    /** size is set to 0.0, and camera view ray computation throws an
        exception */
    CameraChipInfo();
    
    /// create an instance with given size [in mm]
    CameraChipInfo(const Size32f &size);
    
    /// create an instance from given chip size description string
    /** most common cameras use default chip sizes like 1/3"  1/2"
        or even 1 1/3" such strings can be passed to this function. If
        the given string cannot be parsed an ICLException is thrown.
        This is the list of currently supported fixed chip sizes
<pre>
Type 	Aspect Ratio 	Dia. (mm) 	Diagonal 	Width 	Height
1/3.6" 	       4:3 	7.056 	        5.000 	        4.000 	3.000
1/3.2" 	       4:3 	7.938 	        5.680 	        4.536 	3.416
1/3" 	       4:3 	8.467 	        6.000 	        4.800 	3.600
1/2.7" 	       4:3 	9.407 	        6.721 	        5.371 	4.035
1/2.5" 	       4:3 	10.160 	        7.182 	        5.760 	4.290
1/2.3" 	       4:3 	11.044 	        7.70 	        6.16 	4.62
1/2" 	       4:3 	12.700 	        8.000 	        6.400 	4.800
1/1.8" 	       4:3 	14.111 	        8.933 	        7.176 	5.319
1/1.7" 	       4:3 	14.941 	        9.500 	        7.600 	5.700
2/3" 	       4:3 	16.933 	        11.000 	        8.800 	6.600
1" 	       4:3 	25.400 	        16.000 	        12.800 	9.600
4/3" 	       4:3 	33.867 	        22.500 	        18.000 	13.500
1.8"    (*)    3:2 	45.720 	        28.400 	        23.700 	15.700
</pre>
        Where:
        - type is the actual chip size in inch
        - Aspect ratio is the ratio width/height
        - Dia.(mm) is the tube diameter in mm (this is simply the type, converted 
          to mm
        - Diagonal is the actual chip diameter in mm
        - Width and Height are the chip bounds, that are used
        
        <b>Note</b>, given inch string cannot be parsed as one of the pre
        defined chip sizes, it is parse as icl::Size (WxH) <b>in mm</b>. This
        facilitates parsing of CameraChipInfo instances significantly
    */
    CameraChipInfo(const std::string &inchTagOrMM) throw (ICLException);

    /// a null CameraChipInfo instance has size(0,0)
    bool isNull() const;

    /// 1 INCH = 25.4mm
    static const float INCH = 25.4;
    
    /// inch to mm
    static inline float inchToMM(float inch){ return 25.4*INCH; }

    /// mm to inch
    static inline float mmToInch(float mm){ return mm/INCH; }
  };

  /// istrea m operator (using  CameraChipInfo(std::string constructor))
  std::istream &operator>>(std::istream &is, CameraChipInfo &dst);

  /// ostream operator
  std::ostream &operator<<(std::ostream &os,const CameraChipInfo &dst);
  
  
}

#endif
