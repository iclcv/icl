#include <iclXMLDocument.h>


int main(int n, char **ppc){
  if(n!=2){
    std::cout << "usage:\n\t" << (*ppc) << " <XML-DOCUMENT-NAME>" << std::cout;
    exit(-1);
  }

  icl::XMLDocument doc = icl::XMLDocument::load(ppc[1]);
  
  
  std::cout << "this is the parsed document:\n" << doc << "\n";
}
