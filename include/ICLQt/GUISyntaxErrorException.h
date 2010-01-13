#include <ICLUtils/Exception.h>

#include <string>

namespace icl{ 
  /// Internally used and caught exception class for the GUI API \ingroup UNCOMMON
  class GUISyntaxErrorException : public ICLException {
    public:
    GUISyntaxErrorException(const std::string &guidef, const std::string &problem) throw():
    ICLException(std::string("Syntax Error while parsing:\n\"")+guidef+"\"\n("+problem+")\n") {}
    virtual ~GUISyntaxErrorException() throw() {}
  };
}
