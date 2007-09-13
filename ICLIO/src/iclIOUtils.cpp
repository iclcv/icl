#include <iclIOUtils.h>

using namespace std;

namespace icl{
  namespace ioutils{
    
    string toLower(string s){
      // {{{ open
      for(unsigned int i=0;i<s.length();i++){
        s[i]=tolower(s[i]);
      }
      return s;
    };
    
    string time2str(Time::value_type x){
      char acBuf[30];
      sprintf(acBuf, "%lld",x);
      return acBuf;
    }
    string i2str(int i){
      char acBuf[12];
      sprintf(acBuf, "%d",i);
      return acBuf;
    }
    
    string skipWhitespaces(const string &s){
      return s.substr(s.find_first_not_of(' '));
    }
    int ti(const std::string &value){
      return atoi(value.c_str());
    }
    long tl(const std::string &value){
      return atoi(value.c_str());
    }
    vector<int> vec3(int a, int b, int c){
      vector<int> v(3);
      v[0] = a;
      v[1] = b;
      v[2] = c;
      return v;
    }
    
    bool endsWith(const string &s,const string &postfix){
      return s.length() >= postfix.length() && s.substr(s.length()-postfix.length()) == postfix;
    }
    
    bool startsWith(const string &s, const string &praefix){
      return s.find(praefix) == 0;
    }

    void analyseHashes (const string &sFileName, unsigned int& nHashes, string::size_type& iPostfixPos) {
      nHashes = 0; iPostfixPos = string::npos;
      
      // search for first '.'
      string::size_type iTmpPos = sFileName.rfind ('.');
      if (iTmpPos == string::npos) 
        throw ICLException ("cannot detect file type");
      
      // search for second '.' if the postfix is .gz so far
      const string& sType = sFileName.substr (iTmpPos);
      if (sType == ".gz" && iTmpPos > 0) { // search further for file type
        iPostfixPos = sFileName.rfind ('.', iTmpPos-1);
      }
      if (iPostfixPos == string::npos) iPostfixPos = iTmpPos;
      
      // count number of hashes directly before the postfix
      for (string::const_reverse_iterator start (sFileName.begin() + iPostfixPos),
           it = start, end = sFileName.rend(); it != end; ++it) {
        if (*it != '#') {
          // first pos without hash, count hashes
          nHashes = it - start;
          break;
        }
      }
    }


  }
}

