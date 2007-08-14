#include <iclException.h>

#include <string>

namespace icl{ 
  class GUISyntaxErrorException : public ICLException {
    public:
    GUISyntaxErrorException(const std::string &guidef, const std::string &problem) throw():
    ICLException(std::string("Syntax Error while parsing:\n\"")+guidef+"\"\n("+problem+")\n") {}
    virtual ~GUISyntaxErrorException() throw() {}
  };
}
