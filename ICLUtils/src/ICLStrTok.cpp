#include "ICLStrTok.h"

using namespace std;
using namespace icl;

StrTok::StrTok(string s, string sDelims){
  size_t iPos;
  size_t iLastPos = 0;
  
  if(!(s.length())){
    this->m_uiPos = 0;   
    return;
  }
  iPos = s.find_first_of( sDelims, iLastPos );

  // we don't want empty tokens
  if (iPos != iLastPos){
    m_oTokens.push_back(s.substr(iLastPos,iPos-iLastPos));
  }

  while(iPos != string::npos){
    iLastPos = iPos;
    iPos = s.find_first_of( sDelims, iLastPos+1 );
    m_oTokens.push_back(s.substr(iLastPos+1,iPos-iLastPos-1));
  }
  this->m_uiPos = 0;
}

bool StrTok::hasMoreTokens() const{
  return m_uiPos<m_oTokens.size();
}

const string &StrTok::nextToken(){
  return m_oTokens[m_uiPos++];    
}

unsigned int StrTok::nTokens() const{
  return static_cast<unsigned int>(m_oTokens.size());
}

const vector<string> &StrTok::allTokens()const{
  return m_oTokens;
}

