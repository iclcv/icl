#include "iclStringUtils.h"
#include "iclStrTok.h"

using std::string;
using std::vector;

namespace icl{
  
  vector<string> tok(const string &s, const string &delims){
    return StrTok(s,delims).allTokens();
  }


  vector<string> &tok(const string &s, const string &sDelims,vector<string> &dst){
    size_t iPos;
    size_t iLastPos = 0;
    
    unsigned int iDstPos = 0;
    
    if(!(s.length())){
      dst.resize(0);
      return dst;
    }
    iPos = s.find_first_of( sDelims, iLastPos );
    
    // we don't want empty tokens
    if (iPos != iLastPos){
      if(dst.size() > iDstPos){
        dst[iDstPos++] = s.substr(iLastPos,iPos-iLastPos);
      }else{
        dst.push_back(s.substr(iLastPos,iPos-iLastPos));
      }
    }
    
    while(iPos != string::npos){
      iLastPos = iPos;
      iPos = s.find_first_of( sDelims, iLastPos+1 );
      if(dst.size() > iDstPos){
        dst[iDstPos] = s.substr(iLastPos+1,iPos-iLastPos-1);
      }else{
        dst.push_back(s.substr(iLastPos+1,iPos-iLastPos-1));
      }
      iDstPos++;
    }
    dst.resize(iDstPos);
    return dst;
  }
}
