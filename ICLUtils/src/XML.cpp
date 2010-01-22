#include <ICLUtils/XML.h>
#include <ICLUtils/StringUtils.h>
#include <list>
#include <sstream>
#include <ICLUtils/XMLNode.h>

namespace icl{
  
  void XMLNodeDelOp::delete_func(XMLNode *n){
    delete n;
  }



}
