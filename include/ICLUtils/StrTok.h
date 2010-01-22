#ifndef STR_TOK_H
#define STR_TOK_H

#include <vector>
#include <string>

namespace icl{
  
  /// String Tokenizer Utility class  \ingroup UTILS
  class StrTok{
    public:
    
    /// Constructor
    /** @param s string to be tokenized
        @param delims delimiter string: meaning depends on singleCharDelims
        @param singleCharDelims if this is true (default), each character of delims
                                is treated a single possible delimiter. Otherwise,
                                the delims string is used as a delimiter sequence
        @param escapeChar if this char is not '\0', delimiter occurences are skipped,
                          if this char is found directly before the delimiter. (Often
                          the '\\' char is used here)
    */
    StrTok(const std::string &s,const std::string &delims, bool singleCharDelims=true, char escapeChar='\0');
    
    /// Returns whether more tokens are available using nextToken()
    /** <b>note:</b> nextToken is not safe; it must be checked with hasMoreTokens
    */
    bool hasMoreTokens() const;
    
    /// Returns the next token (unsafe -> check with hasMoreTokens before)
    const std::string &nextToken();
    
    /// returns the internal token count
    unsigned int nTokens() const;

    /// returns a vector
    const std::vector<std::string> &allTokens() const;
    
    /// resets internal position indicator
    void reset() { m_uiPos = 0; }
    
    /// reverse iterator type
    typedef std::vector<std::string>::reverse_iterator reverse_iterator;
    
    /// constant reverse iterator type
    typedef std::vector<std::string>::const_reverse_iterator const_reverse_iterator;

    /// iterator type
    typedef std::vector<std::string>::iterator iterator;

    /// constant iterator type
    typedef std::vector<std::string>::const_iterator const_iterator;
    
    /// returns begin-iterator
    inline iterator begin() { return m_oTokens.begin(); }

    /// returns const begin-iterator
    inline const_iterator begin() const { return m_oTokens.begin(); }

    /// returns reverse begin-iterator
    inline reverse_iterator rbegin() { return m_oTokens.rbegin(); }

    /// returns const reverse begin-iterator
    inline const_reverse_iterator rbegin() const { return m_oTokens.rbegin(); }

    /// returns end-iterator
    inline iterator end() { return m_oTokens.end(); }

    /// returns const end-iterator
    inline const_iterator end() const { return m_oTokens.end(); }

    /// returns reverse end-iterator
    inline reverse_iterator rend() { return m_oTokens.rend(); }

    /// returns const reverse end-iterator
    inline const_reverse_iterator rend() const { return m_oTokens.rend(); }

    
    private:
    /// internal data storage
    std::vector<std::string> m_oTokens;
    
    /// current position indicator
    unsigned int m_uiPos;
  };

}


#endif
