#include <iclCommon.h>
#include <iclConfigFile.h>

int main(int n, char **ppc){
  pa_init(n,ppc,"-c(1)");
  QApplication app(n,ppc);
  
  ConfigFile::loadConfig(pa_subarg<std::string>("-c",0,"config.xml"));
  GUI gui("vbox");
  gui << "config(embedded)";
  gui.show();
  //  ConfigFileGUI gui(ConfigFile::getConfig());
  //gui.show();
  
  return app.exec();
}
