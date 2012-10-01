#include <ICLUtils/ConfigFile.h>
#include <ICLQt/Common.h>

// for the Vec type
#include <ICLGeom/GeomDefs.h>

int main(){
  ConfigFile cfg;
  cfg["config.general.a vector"] = Vec(1,2,3,0);
  cfg["config.general.params.threshold"] = 7;
  cfg["config.general.params.value"] = 6.45;
  cfg["config.general.params.filename"] = str("./notHallo.txt");
  cfg["config.special.hint"] = 'a';
  cfg["config.special.nohint"] = 'b';
  SHOW(cfg);
}
