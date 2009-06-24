#include <iclXML.h>
#include <iclStringUtils.h>
#include <list>
#include <sstream>
#include <iclXMLNode.h>

namespace icl{
  
  void XMLNodeDelOp::delete_func(XMLNode *n){
    delete n;
  }



}
