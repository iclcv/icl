#ifndef STR_TOK_H
#define STR_TOK_H

#include <vector>
#include <string>

namespace icl{
  
  /// String Tokenizer Utility class  \ingroup UTILS
  class StrTok{
    public:
    
    /// Constructor
    /**
    @param s string to be tokenized
    @param sDelims delimiter string: each character of this string 
    is used as delimiter of s
    */
    StrTok(std::string s, std::string sDelims);
    
    /// Returns count of remaining tokens
    /**
    @return count of remaining tokens. For each nextToken()-call
    this count will be decreased by one untill it is 0.
    */
    bool hasMoreTokens() const;
    

    /// Returns the next token
    const std::string &nextToken();
    

    /// returns the number of tokesn
    unsigned int nTokens() const;

    /// returns a vector
    const std::vector<std::string> &allTokens() const;
    
    private:
    std::vector<std::string> m_oTokens;
    unsigned int m_uiPos;
  };

}


#endif
