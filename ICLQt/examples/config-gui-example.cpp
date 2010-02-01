#include <ICLQuick/Common.h>
#include <ICLUtils/ConfigFile.h>

int main(int n, char **ppc){
  painit(n,ppc,"-config-file|-c(filename=config.xml)");
  QApplication app(n,ppc);
  
  ConfigFile::loadConfig(*pa("-c"));

  ConfigFile::getConfig().listContents();
  GUI gui("vbox");
  gui << "config(embedded)";
  gui.show();
  //  ConfigFileGUI gui(ConfigFile::getConfig());
  //gui.show();
  
  return app.exec();
}
