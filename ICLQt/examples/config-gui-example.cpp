#include <ICLQuick/Common.h>
#include <ICLUtils/ConfigFile.h>

int main(int n, char **ppc){
  pa_init(n,ppc,"-c(1)");
  QApplication app(n,ppc);
  
  ConfigFile::loadConfig(pa_subarg<std::string>("-c",0,"config.xml"));

  ConfigFile::getConfig().listContents();
  GUI gui("vbox");
  gui << "config(embedded)";
  gui.show();
  //  ConfigFileGUI gui(ConfigFile::getConfig());
  //gui.show();
  
  return app.exec();
}
