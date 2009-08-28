#ifndef ICL_CONFIG_FILE_GUI_H
#define ICL_CONFIG_FILE_GUI_H

#include <iclGUI.h>
#include <iclConfigFile.h>
#include <iclException.h>

#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <list>

namespace icl{

  /// GUI  support class to change GUI entries at runtime
  /** Consider a complex application with a lot of parameters. At some point, using
      program arguments gets to difficult, so you decide to use an xml configuration 
      file which can be integrated very conveniently using the ConfigFile class
      provided by ICL's Qt package.\n
      During the evaluation process of your application, you may have to find an
      optimal parameter set, which implies many passes of
      - adapt parameter
      - run application
      - apply some steps that are necessary to estimate the quality of the new
        parameter value
      - stop your application etc...
      
      If it gets to complicated, you'll have to create a more complex GUI that allows
      to define all your parameters at runtime. But contingently, you'll be able to 
      find good values for your parameters, so you'll no longer need most of the
      new GUI components. This makes you simplify your GUI again and so on ....

      The new ConfigGUI enables you to change the entries of a ConfigFile dynamically
      at runtime. Furthermore, you can save your current adaption progress to another
      -- or even the same -- xml configuration file.
      
      In addition, ConfigFile float- and int-entries can be set up with a range-property to 
      enable the ConfigFileGUI to create a slider for this entry (@see ConfigFile for more details)
  */
  class ConfigFileGUI : public QObject, public GUI, public GUI::Callback{
    Q_OBJECT;
    ConfigFile *m_config;
    bool m_own;
    QTreeWidget *m_tree;

    public:
    /// Create a new config GUI that wraps a given configuration file (or the static one at default)
    ConfigFileGUI(const ConfigFile &config=ConfigFile::getConfig(), QWidget *parent=0) throw(ICLException);
    
    /// destructor
    ~ConfigFileGUI();

    /// load another config (references are not updated)
    void loadConfig(const ConfigFile &config);

    /// returns the root widget of the ConfigFileGUI
    QWidget *getWidget();
    
    /// for GUI::Callback
    virtual void exec();

    /// for GUI::Callback
    virtual void exec(const std::string &handle);
    
    private slots:
    void itemDoubleClicked(QTreeWidgetItem *item, int column);
    void saveAs();
    void load();
    void updateTree();
    
    private:
    struct NamedGUI{
      GUI gui;
      std::string id;
      std::string type;
      QTreeWidgetItem *item;
    };
    std::list<NamedGUI> m_guis;
  };
  
}

#endif
