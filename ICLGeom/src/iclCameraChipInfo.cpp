#include <iclCameraChipInfo.h>

#include <iclStringUtils.h>
#include <sstream>
#include <map>

namespace icl{
  
  CameraChipInfo::CameraChipInfo(){}
  
  CameraChipInfo::CameraChipInfo(const Size32f &size):size(size){}
  
  CameraChipInfo::CameraChipInfo(const std::string &inch) throw (ICLException){
    static std::map<std::string,Size32f> lut;
    if(!lut.size()){
#define FORMAT(I,a,b,c,W,H) lut[I] = Size32f(W,H)
      FORMAT("1/3.6\"",  4:3,  7.056,  5.000,  4.000,  3.000);
      FORMAT("1/3.2\"",  4:3,  7.938,  5.680,  4.536,  3.416);
      FORMAT("1/3\"",    4:3,  8.467,  6.000,  4.800,  3.600);
      FORMAT("1/2.7\"",  4:3,  9.407,  6.721,  5.371,  4.035);
      FORMAT("1/2.5\"",  4:3,  10.160, 7.182,  5.760,  4.290);
      FORMAT("1/2.3\"",  4:3,  11.044, 7.70,   6.16,   4.62);
      FORMAT("1/2\"",    4:3,  12.700, 8.000,  6.400,  4.800);
      FORMAT("1/1.8\"",  4:3,  14.111, 8.933,  7.176,  5.319);
      FORMAT("1/1.7\"",  4:3,  14.941, 9.500,  7.600,  5.700);
      FORMAT("2/3\"",    4:3,  16.933, 11.000, 8.800,  6.600);
      FORMAT("1\"",      4:3,  25.400, 16.000, 12.800, 9.600);
      FORMAT("4/3\"",    4:3,  33.867, 22.500, 18.000, 13.500);
      FORMAT("1.8\"",    3:2,  45.720, 28.400, 23.700, 15.700);
#undef FORMAT
    }
    std::map<std::string,Size32f>::const_iterator it = lut.find(inch);
    if(it != lut.end()){
      this->size = it->second;
    }
    try{
      this->size = parse<Size32f>(inch);
    }catch(...){
      throw ICLException("unable to parse inch string " + inch);
    }
  }

  bool CameraChipInfo::isNull() const{
    return size == Size32f::null;
  }

  std::istream &operator>>(std::istream &is, CameraChipInfo &dst){
    std::string nextToken;
    is >> nextToken;
    dst = CameraChipInfo(nextToken);
    return is;
  }

  std::ostream &operator<<(std::ostream &os,const CameraChipInfo &dst){
    return os << dst.size;
  }
}
