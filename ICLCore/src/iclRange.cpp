#include <iclRange.h>
#include <iclStringUtils.h>

namespace icl{

  template<class T>
  std::string translateRange(const Range<T> &t){
    return std::string("[") + str(t.minVal) + "," + str(t.maxVal) + "]"; 
  }
  
  template<class T>
  Range<T> translateRange(const std::string &range){
    if(range.length() < 5){
      ERROR_LOG("invalid range string" << range); 
      return Range<T>();
    }
    std::string range2 = range.substr(1,range.size()-2);
    std::string minVal = range2.substr(0,range2.find(','));
    std::string maxVal = range2.substr(minVal.length()+1);
    return Range<T>(parse<T>(minVal),parse<T>(maxVal));
  }
  
#define ICL_INSTANTIATE_DEPTH(D)                                        \
  template std::string translateRange<icl##D>(const Range<icl##D>&);    \
  template Range<icl##D> translateRange<icl##D>(const std::string &);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
  template std::string translateRange<unsigned int>(const Range<unsigned int>&);
  template Range<unsigned int> translateRange<unsigned int>(const std::string &);
  
}
