#include <ICLUtils/ConfigFile.h>
#include <ICLQt/Common.h>

// for the Vec type
#include <ICLGeom/GeomDefs.h>

int main(){
  ConfigFile cfg("config-file.xml");

  Vec v = cfg["config.general.a vector"];
  cfg.setPrefix("config.general.params.");
  int threshold = cfg["threshold"];
  double value = cfg["value"];
  std::string filename = cfg["filename"];

  SHOW(v);
  SHOW(threshold);
  SHOW(value);
  SHOW(filename);

}
