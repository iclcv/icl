#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

using namespace icl;

int main(int n, char **ppc){
  painit(n,ppc,"[m]-config|-c(filename)");

  ConfigFile f(pa("-c"));
  std::cout << "config file content:" << std::endl;
  f.listContents();


  f.clear();
  f.setPrefix("config.");

  f["section-1.subsection-1.val1"] = 5;
  f["section-2.subsection-1.val1"] = 5.4;
  f["section-1.subsection-2.val1"] = 544.f;
  f["section-2.subsection-1.val2"] = 'c';
  f["section-3.subsection-1.val3"] = str("22test");
  
  f.listContents();
  
  std::cout << f << std::endl;
}
