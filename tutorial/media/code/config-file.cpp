#include <iclConfigFile.h>
#include <iclFixedMatrix.h>

using namespace icl;

int main(){
  ConfigFile f("config.xml");
  
  f.setPrefix("config.");

  int thresh = f["global.threshold"];
  float radius = f["global.radius"];

  /// arbitrary types can be passed as strings
  FixedMatrix<float,3,3> T = parse<FixedMatrix<float,3,3> >(f["local.T"]);
  
  std::cout << "radius is: " << radius << std::endl;
  std::cout << "thresh is: " << thresh << std::endl;
  std::cout << "matrix is: " << std::endl << T << std::endl;
  
  /// also we can create configuration files that simply
  ConfigFile f2;
  f2["config.foo.a"] = radius;
  f2["config.foo.b"] = std::string("hello");

  //f2.save(f["local.filename"]);
}
