#include <ICLUtils/Range.h>
#include <ICLUtils/StringUtils.h>
namespace icl{

  template<class T> 
  std::ostream &operator<<(std::ostream &s, const Range <T> &range){
    s << '[';
    icl_to_stream(s,range.minVal);
    s << ',';
    icl_to_stream(s,range.maxVal);
    return s << ']';
  }

  template<class T> 
  std::istream &operator>>(std::istream &s, Range <T> &range){
    char c;
    s >> c;
    icl_from_stream(s,range.minVal);
    s >> c;
    icl_from_stream(s,range.maxVal);
    return s >> c;
  }

  
  
#define ICL_INSTANTIATE_DEPTH(D)                                        \
  template std::ostream &operator<<(std::ostream&,const Range<icl##D>&); \
  template std::istream &operator>>(std::istream&,Range<icl##D>&);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
  
}
