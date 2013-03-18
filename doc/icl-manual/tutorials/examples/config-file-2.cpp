#include <ICLUtils/ConfigFile.h>
#include <ICLQt/Common.h>

// for the Vec type
#include <ICLGeom/GeomDefs.h>

int main(){
  ConfigFile cfg;
  cfg["config.general.a vector"] = Vec(1,2,3,0);
  cfg.setPrefix("config.general.params.");
  cfg["threshold"] = 7;
  cfg["value"] = 6.45;
  cfg["filename"] = str("./notHallo.txt");
  cfg.setPrefix("");
  cfg["config.special.hint"] = 'a';
  cfg["config.special.nohint"] = 'b';
  SHOW(cfg);
}
