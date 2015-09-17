/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GUI.cpp                                **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/StrTok.h>
#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/ProcessMonitor.h>
#include <ICLUtils/Size.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLCore/CoreFunctions.h>

#include <ICLQt/GUI.h>
#include <ICLQt/GUIWidget.h>
#include <ICLQt/ContainerGUIComponents.h>
#include <ICLQt/GUIDefinition.h>
#include <ICLQt/GUISyntaxErrorException.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Array2D.h>
#include <ICLQt/Widget.h>
#include <ICLUtils/File.h>

#include <QStyleFactory>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QLCDNumber>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QTabBar>
#include <QMainWindow>
#include <QDockWidget>
#include <QTabWidget>
#include <QApplication>
#include <QSplitter>
#include <QScrollArea>

#include <ICLQt/ProxyLayout.h>

#include <ICLQt/ButtonHandle.h>
#include <ICLQt/BoxHandle.h>
#include <ICLQt/TabHandle.h>
#include <ICLQt/BorderHandle.h>
#include <ICLQt/ButtonGroupHandle.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/StateHandle.h>
#include <ICLQt/SliderHandle.h>
#include <ICLQt/FSliderHandle.h>
#include <ICLQt/IntHandle.h>
#include <ICLQt/FloatHandle.h>
#include <ICLQt/StringHandle.h>
#include <ICLQt/ComboHandle.h>
#include <ICLQt/SpinnerHandle.h>
#include <ICLQt/ImageHandle.h>
#include <ICLQt/DrawHandle.h>
#include <ICLQt/DrawHandle3D.h>
#include <ICLQt/DispHandle.h>
#include <ICLQt/FPSHandle.h>
#include <ICLQt/CheckBoxHandle.h>
#include <ICLQt/MultiDrawHandle.h>
#include <ICLQt/SplitterHandle.h>
#include <ICLQt/ColorHandle.h>
#include <ICLQt/PlotHandle.h>
#include <ICLQt/PlotWidget.h>
#include <ICLQt/Quick.h>
#include <QCheckBox>
#include <QtCore/QTimer>

#include <QFileDialog>

#include <ICLQt/CamCfgWidget.h>
#include <ICLUtils/StringUtils.h>
#include <ICLQt/ToggleButton.h>

#include <ICLQt/Widget.h>
#include <ICLQt/DrawWidget.h>
#ifdef ICL_HAVE_OPENGL
#include <ICLQt/DrawWidget3D.h>
#endif
#include <ICLQt/ThreadedUpdatableSlider.h>
#include <ICLQt/ThreadedUpdatableTextView.h>
#include <ICLQt/ColorLabel.h>
#include <ICLUtils/Configurable.h>
#include <ICLCore/Color.h>
#include <QProgressBar>

#include <map>
#include <set>

#ifndef ICL_SYSTEM_WINDOWS
  #include <unistd.h>
#endif

using namespace std;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;


namespace icl{
  namespace qt{
    namespace{
      struct VolatileUpdater : public QTimer{
        std::string prop;
        GUI &gui;
        Configurable &conf;
        LabelHandle *l;
        VolatileUpdater(int msec, const std::string &prop, GUI &gui, Configurable &conf):
          prop(prop),gui(gui),conf(conf),l(0){
          setInterval(msec);
        }
        virtual void timerEvent(QTimerEvent * e){
          if(!l){
            l = &gui.get<LabelHandle>("#i#"+prop);
          }
          (***l).setText(conf.getPropertyValue(prop).c_str());
          (***l).update();
          QApplication::processEvents();
        }
      };
    }

    static const std::string &gen_params(){
      // {{{ open

      static std::string op = ( "general params are: \n"
                                "\t@size=WxH     (W and H are positive integers) set min and max size of that widget\n"
                                "\t@minsize=WxH  (W and H are positive integers) set min. size of that widget\n"
                                "\t@maxsize=WxH  (W and H are positive integers) set max. size of that widget\n"
                                "\t@handle=NAME  if defined, the componets handle is allocated with id NAME\n"
                                "\t              (all size parameters are defined in cells of 15x15 pixles)\n"
                                "\t@label=L      L is the label of this component\n"
                                "\t@out=LIST     LIST is a comma-seperated list of output names\n"
                                "\t@inp=LIST     LIST is a comma-seperated list of output names\n"
                                "\tmargin=MARGIN MARGING is the layout pixel margin for layouting components\n"
                                "\tspacing=SPA   SPA is the layout spacing for layouting components\n"
                                "\ttooltip=TEXT  TEXT is a tooltip text\n" );
      return op;
    }

    // }}}

    /// special gui component for visualizing process information
    struct ProcessMonitorGUIWidget : public GUIWidget{
      QTimer updater;
      ProcessMonitor *pm;
      ProcessMonitor::Info info;

      QLabel *threadCountLabel,*memoryUsageLabel;
      QProgressBar *cpuBar, *cpuBarThis;
      bool rangeSet;
      QCheckBox *disabled;

      ProcessMonitorGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(6,3)){
        rangeSet = false;
        if(def.numParams() > 1) throw GUISyntaxErrorException(def.defString(),"0 or 1 parameters are allowed here");
        float fps = def.numParams() ? parse<float>(def.param(0)) : 10;
        if(fps <= 0 || fps > 10) throw GUISyntaxErrorException(def.defString(),"fps must be in range ]0,10]");

        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for the ProcessMonitor GUI component!");
        }

        updater.setInterval(1000.0f/fps);

        pm = ProcessMonitor::getInstance();
        connect(&updater,SIGNAL(timeout()),this,SLOT(ioSlot()));

        updater.start();

        QLabel *p ;
        addToGrid(p=new QLabel("thread count :",this),0,0,5,1);
        p->setToolTip("current number of threads\n"
                      "of this process. The number of cores\n"
                      "is used to estimate the maximum value\n"
                      "of processor usage percent, which is\n"
                      "#cores x 100%.");

        addToGrid(p=new QLabel("memory used :",this),0,1,5,1);
        p->setToolTip("amount of memory that is\n"
                      "currently used by this process");

        addToGrid(p=new QLabel("overall cpu used :",this),0,2,5,1);
        p->setToolTip("current overall cpu usage\n"
                      "(100% means all cores are\n"
                      "fully used)");

        addToGrid(new QLabel("cpu this process :",this),0,3,5,1);
        p->setToolTip("current cpu usage of this process \n"
                      "(100% means one core is fully used).\n"
                      "Scales up to numCores x 100 %))");

        threadCountLabel = new QLabel("1",this);
        memoryUsageLabel = new QLabel("0 MB",this);

        addToGrid(threadCountLabel,5,0,3,1);
        addToGrid(memoryUsageLabel,5,1,3,1);

        cpuBar = new QProgressBar(this);
        cpuBar->setRange(0,100);
        cpuBar->setValue(50);
        addToGrid(cpuBar,5,2,4,1);


        cpuBarThis = new QProgressBar(this);
        cpuBarThis->setValue(77);
        cpuBarThis->setFormat("%v%");
        addToGrid(cpuBarThis,5,3,4,1);

        disabled= new QCheckBox("disable updates",this);
        disabled->setChecked(false);
        addToGrid(disabled,0,4,8,1);
      }


      static string getSyntax(){
        return string("ps(fps in ]0...10] = 10)[general params]\n")+gen_params();
      }

      virtual void processIO(){
        if(disabled->checkState() == Qt::Checked) return;

        info = pm->getInfo();
        cpuBar->setValue((int)info.allCpuUsage);
        if(!rangeSet){
          rangeSet = true;
          cpuBarThis->setRange(0,info.numCPUs * 100);
        }
        cpuBarThis->setValue((int)info.cpuUsage);

        std::ostringstream s;
        s << info.numThreads << " (" << info.numCPUs << " cores) ";
        threadCountLabel->setText(s.str().c_str());

        memoryUsageLabel->setText((str(info.memoryUsage)+" MB").c_str());
        update();
      }
    };

    // quite complex component for embedded property component 'prop'
    struct ConfigurableGUIWidget : public GUIWidget{

      std::vector<SmartPtr<VolatileUpdater> > timers;
      Configurable *conf;
      GUI gui;
      GUI sub_gui;
      bool deactivateExec;
      std::string processingProperty;
      utils::Mutex execMutex;
      std::map<std::string,std::string> deferredAssignList;

      struct StSt{
        std::string full,half;
        StSt(const std::string &full, const std::string &half):full(full),half(half){}
      };

      void update_all_components(){
        std::vector<std::string> props = conf->getPropertyListWithoutDeactivated();
        for(unsigned int i=0;i<props.size();++i){
          const std::string &p = props[i];
          std::string t = conf->getPropertyType(p);
          if(t == "range" || t == "range:slider"){
            SteppingRange<float> r = parse<SteppingRange<float> >(conf->getPropertyInfo(p));
            if(r.stepping == 1){
              gui.get<SliderHandle>("#r#"+p).setValue( parse<icl32s>(conf->getPropertyValue(p)) );
            }else{
              gui.get<FSliderHandle>("#r#"+p).setValue( parse<icl32f>(conf->getPropertyValue(p)) );
            }
          }else if( t == "range:spinbox"){
            gui.get<SpinnerHandle>("#R#"+p).setValue( parse<icl32s>(conf->getPropertyValue(p)) );
          }else if( t == "menu" || t == "value-list" || t == "valueList"){
            std::string handle = (t == "menu" ? "#m#" : "#v#")+p;
            gui.get<ComboHandle>(handle).setSelectedItem(conf->getPropertyValue(p));
          }else if( t == "info"){
            gui["#i#"+p] = conf->getPropertyValue(p);
          }else if( t == "flag"){
            gui["#f#"+p] = conf->getPropertyValue(p).as<bool>();
          }else if( t == "int"){
            gui["#I#"+p] = parse<int>(conf->getPropertyValue(p));
          }else if( t == "float"){
            gui["#F#"+p] = parse<float>(conf->getPropertyValue(p));
          }else if( t == "string"){
            gui["#S#"+p] = conf->getPropertyValue(p);
          }else if( t == "color"){
            gui["#C#"+p] = parse<Color>(conf->getPropertyValue(p));
          }else if( t == "Point32f"){
            gui["#p#"+p] = conf->getPropertyValue(p);
          }

        }
      }

      std::string get_combo_list(const std::string &pfull){
        std::vector<std::string> l = tok(conf->getPropertyInfo(pfull),",");
        const std::string c = conf->getPropertyValue(pfull);
        for(unsigned int i=0;i<l.size();++i){
          if(l[i] == c) l[i] = '!'+l[i];
        }
        return cat(l,",");
      }

      void add_component(GUI &gui,const StSt &p, std::ostringstream &ostr, GUI &timerGUI){
        std::string t = conf->getPropertyType(p.full);
        std::string tt = conf->getPropertyToolTip(p.full);
        std::string ttt = tt.length() ? "@tooltip="+tt : str("");

        if(t == "range" || t == "range:slider"){
          // todo check stepping ...
          std::string handle="#r#"+p.full;
          SteppingRange<float> r = parse<SteppingRange<float> >(conf->getPropertyInfo(p.full));
          std::string c = conf->getPropertyValue(p.full);
          if(r.stepping >= 1){
            gui << Slider(r.minVal,r.maxVal,parse<int>(c),false,r.stepping).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
          }else{
            //            if(r.stepping){
            //  WARNING_LOG("the prop-GUI compoment is not able to adjust a slider stepping that is not 1");
            //  WARNING_LOG("component was " << p.full);
            //}
            gui << FSlider(r,parse<float>(c)).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
          }
          ostr << '\1' << handle;
        }else if( t == "range:spinbox"){
          std::string handle="#R#"+p.full;
          Range32s r = parse<Range32s>(conf->getPropertyInfo(p.full));
          std::string c = conf->getPropertyValue(p.full);
          gui << Spinner(r.minVal, r.maxVal, parse<int>(c)).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
          ostr << '\1' << handle;
        }else if(t == "menu" || t == "value-list" || t == "valueList"){
          std::string handle = (t == "menu" ? "#m#" : "#v#")+p.full;
          gui << Combo(get_combo_list(p.full)).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
          ostr << '\1' << handle;
        }else if(t == "command"){
          std::string handle = "#c#"+p.full;
          ostr << '\1' << handle;
          gui << Button(p.half).tooltip(tt).handle(handle).minSize(12,2);
        }else if(t == "info"){
          std::string handle = "#i#"+p.full;
          ostr << '\1' << handle;
          gui << Label(conf->getPropertyValue(p.full)).tooltip(tt).handle(handle).minSize(12,2).label(p.half);

          int volatileness = conf->getPropertyVolatileness(p.full);
          if(volatileness){
            timers.push_back(new VolatileUpdater(volatileness,p.full,timerGUI,*conf));
          }
        }else if(t == "flag"){
          std::string handle = "#f#"+p.full;
          ostr << '\1' << handle;
          gui << CheckBox(p.half, conf->getPropertyValue(p.full)).tooltip(tt).handle(handle).minSize(12,2);
        }else if(t == "float"){
          std::string handle = "#F#"+p.full;
          ostr << '\1' << handle;
          Range32f mm = parse<Range32f>(conf->getPropertyInfo(p.full));
          float v = conf->getPropertyValue(p.full);
          gui << Float(mm.minVal, mm.maxVal, v).tooltip(tt).handle(handle).minSize(12,2).label(p.half);

        }else if(t == "int"){
          std::string handle = "#I#"+p.full;
          ostr << '\1' << handle;
          Range32s mm = parse<Range32s>(conf->getPropertyInfo(p.full));
          int v = conf->getPropertyValue(p.full);
          gui << Int(mm.minVal,mm.maxVal,v).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
        }else if(t == "string"){
          std::string handle = "#S#"+p.full;
          ostr << '\1' << handle;
          int max_len = parse<int>(conf->getPropertyInfo(p.full));
          std::string value = conf->getPropertyValue(p.full);
          if(!value.length()) value = " ";
          gui << String(value, max_len).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
        }else if(t =="color"){
          std::string handle = "#C#"+p.full;
          ostr << '\1' << handle;
          Color c = parse<Color>(conf->getPropertyValue(p.full));
          gui << ColorSelect(c[0],c[1],c[2]).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
        }else if(t == "Point32f"){
          std::string handle = "#p#"+p.full;
          ostr << '\1' << handle;
          Point32f pt = conf->getPropertyValue(p.full);
          deferredAssignList[handle] = str(pt);
          gui << String(" ",100).tooltip(tt).handle(handle).minSize(12,2).label(p.half);
        }
        

        else{
          ERROR_LOG("unable to create GUI-component for property \"" << p.full << "\" (unsupported property type: \"" + t+ "\")");
        }
       }

      bool isSpecialGrabberGrabberProperty(Configurable* c, const std::string &prop){
        if(dynamic_cast<io::Grabber*>(conf)){
          const unsigned int propcount = 5;
          const std::string properties[propcount] = {"format", "size", "desired format", "desired size", "desired depth"};
          for (unsigned int i = 0; i < propcount; ++i){
            if(properties[i] == prop) return true;
          }
        }
        return false;
      }

      StSt getStSt(std::map<std::string,std::vector<StSt> > &map, std::string name){
        for(std::map<std::string,std::vector<StSt> >::iterator it=map.begin();it != map.end();++it){
          for(unsigned int i=0;i<it->second.size();++i){
            if((it->second[i]).full == name){
              return it->second[i];
            }
          }
        }

        ERROR_LOG("Could not find " << name << " property in map.");
        return StSt("error", "error");
      }

      ConfigurableGUIWidget(const GUIDefinition &def)
        : GUIWidget(def,1,1,GUIWidget::gridLayout, Size(8,12)),
          deactivateExec(false), processingProperty(""),
          execMutex(Mutex::mutexTypeRecursive)

      {
        static const std::string pointer_prefix = "@pointer@:";
        if(def.param(0).length() > pointer_prefix.length() &&
           def.param(0).substr(0,pointer_prefix.length()) == pointer_prefix){
          conf = Any::ptr<Configurable>(def.param(0).substr(pointer_prefix.length()));
        }else{
          conf = Configurable::get(def.param(0));
        }
        if(!conf) throw GUISyntaxErrorException(def.defString(),"No Configurable with ID "+def.param(0)+" registered");

        std::vector<std::string> props = conf->getPropertyListWithoutDeactivated();
        std::map<std::string,std::vector<StSt> > sections;
        std::map<int,std::string> sections_ordering;

        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for the Configurable GUI component!");
        }

        for(unsigned int i=0;i<props.size();++i){
          const std::string &p = props[i];
          size_t pos = p.find('.');
          string name;
          bool doesnt_exist;
          if(pos == std::string::npos){
            name = "general";
            doesnt_exist = sections.find(name) == sections.end();
            sections["general"].push_back(StSt(p,p));
          }else{
            name = p.substr(0,pos);
            doesnt_exist = sections.find(name) == sections.end();
            sections[name].push_back(StSt(p,p.substr(pos+1))); //
          }
          if(conf->isOrderedFlagSet())
            if(doesnt_exist)sections_ordering[sections.size()] = name;
        }

        std::string tablist;

        int generalIdx = 0;
        int i=0;
        if(!conf->isOrderedFlagSet()) {
          for(std::map<std::string,std::vector<StSt> >::iterator it=sections.begin();it != sections.end();++it){
            if(it->first == "general") {
              generalIdx = i;
            }
            tablist += (tablist.length()?",":"")+it->first;
            ++i;
          }
        } else {
          for(std::map<int,std::string>::iterator it_ordered=sections_ordering.begin();it_ordered != sections_ordering.end();++it_ordered){
            std::string first = it_ordered->second;
            if(first == "general") {
              generalIdx = i;
            }
            tablist += (tablist.length()?",":"")+first;
            ++i;
          }
        }
        gui = HSplit(this).handle("__the_root__");
        bool use_tabs = sections.size() > 1 || (sections.size() == 1 && sections.begin()->first != "general");
        if(use_tabs){
          if(sections.find("general") == sections.end()){
            tablist += ",general";
            generalIdx = tablist.size()-1;
          }
          sub_gui = Tab(tablist,this).handle("__the_tab__");
        } else {
          sub_gui = VBox(this).handle("__the_tab__");
        }

        std::ostringstream ostr;

        // special treatment of grabbers
        if(dynamic_cast<io::Grabber*>(conf)){
          GUI general_box = VBox(this).handle("__the_box__");
          add_component(general_box,getStSt(sections, "format"),ostr,gui);
          add_component(general_box,getStSt(sections, "size"),ostr,gui);
          add_component(general_box,getStSt(sections, "desired format"),ostr,gui);
          add_component(general_box,getStSt(sections, "desired size"),ostr,gui);
          add_component(general_box,getStSt(sections, "desired depth"),ostr,gui);
          gui <<  general_box;
        }

        bool haveGeneral = false;
        if(!conf->isOrderedFlagSet()) {
          for(std::map<std::string,std::vector<StSt> >::iterator it=sections.begin();it != sections.end();++it){
            GUI tab = VScroll();
            for(unsigned int i=0;i<it->second.size();++i){
              if(!isSpecialGrabberGrabberProperty(conf,(it->second[i]).full)){
                add_component(tab,it->second[i],ostr,gui);
              }
            }
            if(it->first == "general"){
              haveGeneral = true;
              tab << ( HBox()
                       << Button("load").handle("#X#load")
                       << Button("save").handle("#X#save")
                     );

              ostr <<  '\1' << "#X#load";
              ostr <<  '\1' << "#X#save";
            }
            sub_gui << tab;
          }
        } else {
            for(std::map<int,std::string>::iterator it_ordered=sections_ordering.begin();it_ordered != sections_ordering.end();++it_ordered){
              std::string first = it_ordered->second;
              std::vector<StSt>& second = sections[it_ordered->second];
              GUI tab = VScroll();
              for(unsigned int i=0;i<second.size();++i){
                if(!isSpecialGrabberGrabberProperty(conf,(second[i]).full)){
                  add_component(tab,second[i],ostr,gui);
                }
              }
              if(first == "general"){
                haveGeneral = true;
                tab << ( HBox()
                         << Button("load").handle("#X#load")
                         << Button("save").handle("#X#save")
                       );

                ostr <<  '\1' << "#X#load";
                ostr <<  '\1' << "#X#save";
              }
              sub_gui << tab;
            }
        }

        if(!haveGeneral){
          GUI tab = VScroll();
            tab << ( HBox()
                     << Button("load").handle("#X#load")
                     << Button("save").handle("#X#save")
                     );

            ostr <<  '\1' << "#X#load";
            ostr <<  '\1' << "#X#save";
            sub_gui << tab;
        }

        gui << sub_gui;
        gui.create();
        
        for(std::map<std::string,std::string>::const_iterator it = deferredAssignList.begin();
            it != deferredAssignList.end(); ++it){
          gui[it->first] = it->second;
        }

        if(use_tabs){
          (**gui.get<TabHandle>("__the_tab__")).setCurrentIndex(generalIdx);
        }

        std::string cblist = ostr.str();
        if(cblist.size() > 1){
          gui.registerCallback(utils::function(this,&icl::qt::ConfigurableGUIWidget::exec),cblist.substr(1),'\1');
        }
        for(unsigned int i=0;i<timers.size();++i){
          timers[i]->start();
        }

        conf->registerCallback(utils::function(this,&icl::qt::ConfigurableGUIWidget::propertyChanged));
      }

      /// Called if a property is changed from somewhere else
      void propertyChanged(const Configurable::Property &p){
        Mutex::Locker l(execMutex);
        const std::string &name = p.name;
        const std::string &type = p.type;
        deactivateExec = true;
        processingProperty = p.name;
        if(type == "range" || type == "range:slider"){
          SteppingRange<float> r = parse<SteppingRange<float> >(conf->getPropertyInfo(name));
          if(r.stepping >= 1){
            int val = parse<icl32s>(conf->getPropertyValue(name));
            int s = (int)(r.stepping);
            val = (val/s)*s;
            gui.get<SliderHandle>("#r#"+name).setValue( val );
          }else{
            gui.get<FSliderHandle>("#r#"+name).setValue( parse<icl32f>(conf->getPropertyValue(name)) );
          }
        }else if (type == "range:spinbox"){
          gui.get<SpinnerHandle>("#R#"+name).setValue( parse<icl32s>(conf->getPropertyValue(name)) );
        }else if( type == "menu" || type == "value-list" || type == "valueList"){
          std::string handle = (type == "menu" ? "#m#" : "#v#")+name;
          gui.get<ComboHandle>(handle).setSelectedItem(conf->getPropertyValue(name));
        }else if( type == "info"){
          gui["#i#"+name] = conf->getPropertyValue(name);
        }else if( type == "flag"){
          gui["#f#"+name] = conf->getPropertyValue(name).as<bool>();
        }else if(type == "int"){
          gui["#I#"+name] = conf->getPropertyValue(name).as<int>();
        }else if(type == "float"){
          gui["#F#"+name] = conf->getPropertyValue(name).as<float>();
        }else if(type == "string"){
          gui["#S#"+name] = conf->getPropertyValue(name).as<std::string>();
        }else if(type == "color"){
          gui["#C#"+name] = conf->getPropertyValue(name).as<Color>();
        }else if(type == "Point32f"){
          gui["#p#"+name] = conf->getPropertyValue(name).as<Point32f>();
        }

        deactivateExec = false;
        processingProperty = "";
      }

      void exec(const std::string &handle){
        Mutex::Locker l(execMutex);
        if(handle.length()<3 || handle[0] != '#') throw ICLException("invalid callback (this should not happen)");
        std::string prop = handle.substr(3);
        if(deactivateExec || processingProperty == prop){
          return;
        } else {
          deactivateExec = true;
        }
        processingProperty = prop;
        switch(handle[1]){
          case 'r':
          case 'R':
          case 'm':
          case 'v':
          case 'f':
          case 'I':
          case 'F':
          case 'S':
          case 'p':
            conf->setPropertyValue(prop,gui[handle].as<Any>());
            break;
          case 'C':
            conf->setPropertyValue(prop,gui[handle].as<Color>());
            break;
          case 'c':
            conf->setPropertyValue(prop,"");
            break;
          case 'X':
            if(prop == "load"){
              try{
                conf->loadProperties(
                      openFileDialog("XML (*.xml);; All files (*)",
                                     "load property file")
                      );
                //update_all_components();
              } catch (utils::ICLException &){
                // cancel
              }
            }else if(prop == "save"){
              try{
                conf->saveProperties(
                      saveFileDialog("XML (*.xml);; All files (*)",
                                     "save properties to file"
                                     ));
              } catch (utils::ICLException &){
                // cancel
              }
            }
            break;
          default:
            ERROR_LOG("invalid callback ID " << handle);
        }
        deactivateExec = false;
        processingProperty = "";
      }

      static string getSyntax(){
        return string("prop(ConfigurableID)[general params]\n")+gen_params();
      }

    };

    struct CamPropertyWidget : public Tab {

      static std::string create_tab_list(){
         const std::vector<io::GrabberDeviceDescription> devs =
                            io::GenericGrabber::getDeviceList("",false);
         std::ostringstream ret;
         for(unsigned int i = 0; i < devs.size(); ++i){
            ret << "[" << devs.at(i).type << "] " << i << ",";
         }
        return ret.str();
      }

      CamPropertyWidget()
        : Tab(create_tab_list())
      {
        minSize(32,24);
        std::vector<io::GrabberDeviceDescription> devs = io::GenericGrabber::getDeviceList("",false);
        for(unsigned int i = 0; i < devs.size(); ++i){
          *this << Prop(devs.at(i).name()).label(devs.at(i).name());
        }
        *this << Create();
      }
    };


    struct CamCfgGUIWidget : public GUIWidget {
      // {{{ open

      CamCfgGUIWidget(const GUIDefinition &def):
        GUIWidget(def,0,2), m_cfg(NULL), m_button(NULL)
      {
        if(def.numParams() != 0  && def.numParams() != 2){
          throw GUISyntaxErrorException(def.defString(),"camcfg can take 0 or 2 parameters");
        }
        if(def.numParams() == 0){
          m_button = new QPushButton("camcfg",this);
          connect(m_button,SIGNAL(clicked()),this,SLOT(ioSlot()));
          addToGrid(m_button);
        } else {
          if(!m_cfg) m_cfg = new CamPropertyWidget();
          addToGrid(m_cfg -> getRootWidget());
        }

        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for the Camera Configuration GUI component!");
        }
      }

      virtual void processIO(){
          if(!m_cfg) m_cfg = new CamPropertyWidget();
          m_cfg->show();
      }

      static string getSyntax(){
        return string("camcfg()[general params]\n")+gen_params();
      }

      CamPropertyWidget *m_cfg;
      QPushButton *m_button;
    };

    struct ScrollGUIWidgetBase : public GUIWidget, public ProxyLayout{
      // {{{ open

      ScrollGUIWidgetBase(const GUIDefinition &def, QBoxLayout::Direction d):
        GUIWidget(def,0,-1,GUIWidget::noLayout){
        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }
        setLayout(new QBoxLayout(d,this));
        m_poScroll = new QScrollArea(this);
        layout()->addWidget(m_poScroll);
        layout()->setContentsMargins(0,0,0,0);
        m_poScroll->setWidget(new QWidget(m_poScroll));
        m_poScroll->setWidgetResizable(true);

        m_poScroll->widget()->setLayout(new QBoxLayout(d,m_poScroll));
        m_poScroll->widget()->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

        m_poScroll->widget()->layout()->setMargin(def.margin());
        m_poScroll->widget()->layout()->setSpacing(def.spacing());

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(d != QBoxLayout::LeftToRight,m_poScroll->widget(),this,m_poScroll));
          getGUI()->unlockData();
        }
      }

      virtual ProxyLayout *getProxyLayout() { return this; }

      virtual void addWidget(GUIWidget *widget){
        widget->setParent(m_poScroll->widget());
        m_poScroll->widget()->layout()->addWidget(widget);
      }

      private:
      QScrollArea *m_poScroll;
    };

    // }}}

    struct HScrollGUIWidget : public ScrollGUIWidgetBase{
      // {{{ open
      HScrollGUIWidget(const GUIDefinition &def):ScrollGUIWidgetBase(def,QBoxLayout::LeftToRight){}
      static string getSyntax(){
        return string("hscroll()[general params]\n")+ gen_params();
      }
    };
    // }}}

    struct VScrollGUIWidget : public ScrollGUIWidgetBase{
      // {{{ open
      VScrollGUIWidget(const GUIDefinition &def):ScrollGUIWidgetBase(def,QBoxLayout::TopToBottom){}
      static string getSyntax(){
        return string("vscroll()[general params]\n")+ gen_params();
      }
    };
    // }}}

    struct HBoxGUIWidget : public GUIWidget{
      // {{{ open
      HBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,0,0,GUIWidget::hboxLayout){
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(true,this,this));//def.parentWidget()));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return string("hbox()[general params]\n")+gen_params();
      }
    };

    // }}}


    struct VBoxGUIWidget : public GUIWidget{
      // {{{ open
      VBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,0,0,GUIWidget::vboxLayout){
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<BoxHandle>(def.handle(),BoxHandle(false,this,this));//def.parentWidget()));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return string("hbox()[general params]\n")+gen_params();
      }
    };

    // }}}

    struct SplitterGUIWidgetBase : public GUIWidget, public ProxyLayout{
      // {{{ open
      SplitterGUIWidgetBase(const GUIDefinition &def, bool horz):GUIWidget(def,0,0,GUIWidget::noLayout){
        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        m_layout = new QGridLayout(this);
        m_splitter = new QSplitter(horz ? Qt::Horizontal:Qt::Vertical , this);
        m_layout->addWidget(m_splitter,0,0);
        m_layout->setContentsMargins(0,0,0,0);//2,2,2,2);
        //setContentsMargins(0,0,0,0);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<SplitterHandle>(def.handle(),SplitterHandle(m_splitter,this));
          getGUI()->unlockData();
        }
      }

      // implements the ProxyLayout interface, what should be done if components are added
      // using the GUI-stream operator <<
      virtual void addWidget(GUIWidget *widget){
        m_splitter->addWidget(widget);
      }

      // as this implements also to proxy layout class, this interface function
      // can directly return itself
      virtual ProxyLayout *getProxyLayout() { return this; }

      //static string getSyntax(){
      //  return string("(COMMA_SEPERATED_TAB_LIST)[general params]\n")+gen_params();
      //}
      QSplitter *m_splitter;
      QGridLayout *m_layout;
    };
    // }}}

    struct HSplitterGUIWidget : public SplitterGUIWidgetBase{
      // {{{ open
      HSplitterGUIWidget(const GUIDefinition &def):SplitterGUIWidgetBase(def,true){}
      static string getSyntax(){
        return string("hsplit()[general params]\n")+gen_params();
      }
    };
    // }}}

    struct VSplitterGUIWidget : public SplitterGUIWidgetBase{
      // {{{ open
      VSplitterGUIWidget(const GUIDefinition &def):SplitterGUIWidgetBase(def,false){}
      static string getSyntax(){
        return string("vsplit()[general params]\n")+gen_params();
      }
    };
    // }}}

    struct TabGUIWidget : public GUIWidget, public ProxyLayout{
      // {{{ open
      TabGUIWidget(const GUIDefinition &def):GUIWidget(def,0,2<<20,GUIWidget::noLayout){
        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        m_layout = new QGridLayout(this);
        m_tabWidget = new QTabWidget(this);
        m_layout->addWidget(m_tabWidget,0,0);

        m_layout->setContentsMargins(0,0,0,0);
        m_nextTabIdx = 0;
        m_tabNames = def.allParams();

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<TabHandle>(def.handle(),TabHandle(m_tabWidget,this));
          getGUI()->unlockData();
        }
      }

      // implements the ProxyLayout interface, what should be done if components are added
      // using the GUI-stream operator <<
      virtual void addWidget(GUIWidget *widget){
        QString tabName;
        if(m_nextTabIdx < (int)m_tabNames.size()){
          tabName = m_tabNames[m_nextTabIdx].c_str();
        }else{
          ERROR_LOG("no tab name defined for " << (m_nextTabIdx) << "th tab");
          tabName = QString("Tab ")+QString::number(m_nextTabIdx);
        }
        m_tabWidget->addTab(widget,tabName);
        m_nextTabIdx++;
      }

      // as this implements also to proxy layout class, this interface function
      // can directly return itself
      virtual ProxyLayout *getProxyLayout() { return this; }

      static string getSyntax(){
        return string("tab(COMMA_SEPERATED_TAB_LIST)[general params]\n")+gen_params();
      }

      std::vector<std::string> m_tabNames;
      QTabWidget *m_tabWidget;
      int m_nextTabIdx;
      QGridLayout *m_layout;
    };
    // }}}

    struct BorderGUIWidget : public GUIWidget{
      // {{{ open

      BorderGUIWidget(const GUIDefinition &def):GUIWidget(def,1){

        if(def.hasToolTip()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        m_poGroupBox = new QGroupBox((def.param(0) + "  ").c_str(),def.parentWidget());
        m_poGroupBox->setFlat(false);
        //m_poGroupBox->setStyleSheet("QGroupBox{ border: 1px solid gray; border-radius: 3px;}");
        m_poGroupBox->setStyle(QStyleFactory::create("fusion"));
        m_poLayout = new QVBoxLayout;
        m_poLayout->setMargin(def.margin());
        m_poLayout->setSpacing(def.spacing());
        m_poGroupBox->setLayout(m_poLayout);
        addToGrid(m_poGroupBox);
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<BorderHandle>(def.handle(),BorderHandle(m_poGroupBox,this));
          getGUI()->unlockData();
        }

      }
      static string getSyntax(){
        return string("border(LABEL)[general params]\n")+
        string("\tLABEL is the border label that is shown\n")+
        gen_params();
      }
      virtual QLayout *getGUIWidgetLayout() { return m_poLayout; }
      private:
      QGroupBox *m_poGroupBox;
      QVBoxLayout *m_poLayout;
    };

    // }}}


    struct ColorGUIWidget : public GUIWidget{
      ColorGUIWidget(const GUIDefinition &def):GUIWidget(def,3,4,GUIWidget::gridLayout,Size(6,2)){
        QPushButton *b = new QPushButton("select",def.parentWidget());
        addToGrid(b);
        connect(b,SIGNAL(clicked()),this,SLOT(ioSlot()));

        m_haveAlpha = (def.numParams() == 4);



        if(m_haveAlpha) m_color = new Color4D(def.intParam(0),def.intParam(1),def.intParam(2),0);
        else m_color = new Color4D();

        Color4D col(def.intParam(0),def.intParam(1),def.intParam(2),m_haveAlpha ?def.intParam(3):0);

        getGUI()->lockData();
        m_color = &getGUI()->allocValue<Color4D>(def.output(0),col);
        getGUI()->unlockData();

        colorLabel = new ColorLabel(*m_color,m_haveAlpha,def.parentWidget());

        if(def.hasToolTip()) colorLabel->setToolTip(def.toolTip().c_str());

        addToGrid(colorLabel,1,0,1,1);

        if(def.handle() != ""){
          getGUI()->lockData();
          handle = &getGUI()->allocValue<ColorHandle>(def.handle(),ColorHandle(colorLabel,this));
          getGUI()->unlockData();
        }else{
          handle = 0;
        }
      }

      virtual void processIO(){
        QColor c = !m_haveAlpha ?
                   QColor((*m_color)[0],(*m_color)[1],(*m_color)[2]) :
                   QColor((*m_color)[0],(*m_color)[1],(*m_color)[2],(*m_color)[3]);

        QColor color = QColorDialog::getColor(c,this,"choose color...",
                                              m_haveAlpha ? QColorDialog::ShowAlphaChannel :
                                              (QColorDialog::ColorDialogOption)0);

        colorLabel->setColor(Color4D(color.red(),color.green(),color.blue(),color.alpha()));
      }

      static string getSyntax(){
        return string("color(R,G,B[,A])[general params] \n")+
        string("\tgiven initial Red, Green and Blue values (Alpha is optional)\n")+
        gen_params();
      }
      ColorLabel *colorLabel;
      ColorHandle *handle;
      Color4D *m_color;
      bool m_haveAlpha;
    };

    class ButtonGUIWidget : public GUIWidget{
    public:
      // {{{ open
      ButtonGUIWidget(const GUIDefinition &def):GUIWidget(def,1,1,GUIWidget::gridLayout,Size(4,1)){
        QPushButton *b = new QPushButton(def.param(0).c_str(),def.parentWidget());

        if(def.hasToolTip()) b->setToolTip(def.toolTip().c_str());

        addToGrid(b);
        connect(b,SIGNAL(pressed()),this,SLOT(ioSlot()));

        if(def.handle() != ""){
          getGUI()->lockData();
          m_poClickedEvent = &getGUI()->allocValue<ButtonHandle>(def.handle(),ButtonHandle(b,this));
          getGUI()->unlockData();
        }else{
          m_poClickedEvent = 0;
        }
      }
      static string getSyntax(){
        return string("button(TEXT)[general params] \n")+
        string("\tTEXT is the button text\n")+
        gen_params();
      }
      virtual void processIO(){
        if(m_poClickedEvent){
          m_poClickedEvent->trigger(false);
        }
      }
    private:
      ButtonHandle *m_poClickedEvent;
    };

    // }}}
    struct ButtonGroupGUIWidget : public GUIWidget{
      // {{{ open

      ButtonGroupGUIWidget(const GUIDefinition &def):
        GUIWidget(def,1,2<<20,GUIWidget::gridLayout,Size(4,def.numParams())), m_uiInitialIndex(0){

        for(unsigned int i=0;i<def.numParams();i++){
          string text = def.param(i);
          if(text.length() && text[0]=='!'){
            m_uiInitialIndex = i;
            text = text.substr(1);
          }
          QRadioButton * b = new QRadioButton(text.c_str(),def.parentWidget());
          if(def.hasToolTip()) b->setToolTip(def.toolTip().c_str());

          m_vecButtons.push_back(b);
          addToGrid(b,0,i);
          connect(b,SIGNAL(clicked()),this,SLOT(ioSlot()));
        }

        getGUI()->lockData();
        m_uiIdx = &getGUI()->allocValue<unsigned int>(def.output(0),m_uiInitialIndex);
        getGUI()->unlockData();

        m_vecButtons[m_uiInitialIndex]->setChecked(true);

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<ButtonGroupHandle>(def.handle(),ButtonGroupHandle(&m_vecButtons,this));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return string("buttongroup(LIST)[general params] \n")+
        string("\tLIST is a comma seperated list of radio button texts to create\n")+
        string("\tthe button with a '!'-prefix is selected (of index 0 by default)\n")+
        gen_params();
      }
      virtual void processIO(){
        *m_uiIdx = 0;
        for(unsigned int i=0;i<m_vecButtons.size();i++){
          if(m_vecButtons[i]->isChecked()){
            *m_uiIdx = i;
            break;
          }
        }
      }
      //    virtual Size getDefaultSize() {
      //  return Size(4,m_vecButtons.size());
      //}
    private:
      unsigned int *m_uiIdx;
      vector<QRadioButton*> m_vecButtons;
      unsigned int m_uiInitialIndex ;
    };

    // }}}
    class ToggleButtonGUIWidget : public GUIWidget{
      // {{{ open
    public:
      ToggleButtonGUIWidget(const GUIDefinition &def):
        GUIWidget(def,2){

        bool initToggled = false;
        if(def.param(1).length() && def.param(1)[0] == '!'){
          initToggled = true;
        }

        getGUI()->lockData();
        bool *stateRef = &getGUI()->allocValue<bool>(def.output(0),initToggled);
        getGUI()->unlockData();

        std::string t1 = def.param(0);
        std::string t2 = def.param(1);
        if(t1.length() && t1[0]=='!') t1 = t1.substr(1);
        if(t2.length() && t2[0]=='!') t2 = t2.substr(1);
        m_poButton = new ToggleButton(t1,t2,def.parentWidget(),stateRef);
        if(def.hasToolTip()) m_poButton->setToolTip(def.toolTip().c_str());

        if(initToggled){
          m_poButton->setChecked(true);
        }

        addToGrid(m_poButton);

        // this must be connected to the toggled function too (not to the clicked() signal) because
        // the clicked()-signal is emitted BEFORE the toggled-signale, which makes the button get
        // out of sync-with it's underlying value :-(
        connect(m_poButton,SIGNAL(toggled(bool)),this,SLOT(ioSlot()));

        if(def.handle() != ""){
          getGUI()->lockData();
          m_poHandle = &getGUI()->allocValue<ButtonHandle>(def.handle(),ButtonHandle(m_poButton,this));
          getGUI()->unlockData();
        }else{
          m_poHandle = 0;
        }
      }
      static string getSyntax(){
        return string("togglebutton(U,T)[general params] \n")+
        string("\tU is the buttons text in untoggled state\n")+
        string("\tT is the buttons text in toggled state\n")+
        string("\tif one of U or T has a '!'-prefix, the button is created with this state\n")+
        gen_params();
      }
      virtual void processIO(){
        if(m_poHandle){
          m_poHandle->trigger(false);
        }
      }
    private:
      ToggleButton *m_poButton;
      ButtonHandle *m_poHandle;
    };

  // }}}


    struct CheckBoxGUIWidget : public GUIWidget{
      // {{{ open
    public:
      CheckBoxGUIWidget(const GUIDefinition &def):
        GUIWidget(def,2){

        bool initChecked = false;
        if(def.param(1)=="on" || def.param(1) == "yes" || def.param(1) == "checked"){
          initChecked = true;
        }

        getGUI()->lockData();
        m_stateRef = &getGUI()->allocValue<bool>(def.output(0),initChecked);
        getGUI()->unlockData();

        std::string t = def.param(0);
        m_poCheckBox = new QCheckBox(def.param(0).c_str(),def.parentWidget());
        m_poCheckBox->setTristate(false);
        if(def.hasToolTip()) m_poCheckBox->setToolTip(def.toolTip().c_str());

        if(initChecked){
          m_poCheckBox->setCheckState(Qt::Checked);
        }else{
          m_poCheckBox->setCheckState(Qt::Unchecked);
        }

        addToGrid(m_poCheckBox);

        // this must be connected to the toggled function too (not to the clicked() signal) because
        // the clicked()-signal is emitted BEFORE the toggled-signale, which makes the button get
        // out of sync-with it's underlying value :-(
        connect(m_poCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ioSlot()));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<CheckBoxHandle>(def.handle(),CheckBoxHandle(m_poCheckBox,this,m_stateRef));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return string("checkbox(TEXT,INIT>)[general params] \n")+
        string("\tTEXT is the check box text\n");
        string("\tINIT defines whether the checkbox is initially checked (checked|unchecked)\n")+
        gen_params();
      }
      virtual void processIO(){
        *m_stateRef = m_poCheckBox->checkState() == Qt::Checked;
        //        DEBUG_LOG("check-box-process-IO::: " << *m_stateRef);
      }
    private:
      bool *m_stateRef;
      QCheckBox *m_poCheckBox;
    };

  // }}}




    struct LabelGUIWidget : public GUIWidget{
      // {{{ open
      LabelGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(4,1)){

        m_poLabel = new CompabilityLabel(def.numParams()==1?def.param(0).c_str():"",def.parentWidget());
        if(def.hasToolTip()) m_poLabel->setToolTip(def.toolTip().c_str());

        addToGrid(m_poLabel);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<LabelHandle>(def.handle(),LabelHandle(m_poLabel,this));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return
        string("label(TEXT="")[general params] \n")+
        string("\tTEXT is the initial text showed by the label")+
        gen_params();
      }
    private:
      CompabilityLabel *m_poLabel;
    };

    // }}}



    struct StateGUIWidget : public GUIWidget{
      // {{{ open
      StateGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(4,1)){
        m_text = new ThreadedUpdatableTextView(def.parentWidget());
        m_text->setReadOnly(true);
        if(def.hasToolTip()) m_text->setToolTip(def.toolTip().c_str());

        addToGrid(m_text);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<StateHandle>(def.handle(),StateHandle(m_text,this,def.numParams()?parse<int>(def.param(0)):1<<30));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return
        string("state(MAX_LINES)[general params] \n")+
        string("\tMAX_LINES is the maximal line count of the state widget, odd lines are removed automatically");
        gen_params();
      }
    private:
      ThreadedUpdatableTextView *m_text;
    };

    // }}}


    struct SliderGUIWidget : public GUIWidget{
      // {{{ open
      
      static bool vertical(const GUIDefinition &def){
        return (def.numParams() >= 4) ? (def.param(3)=="vertical") : false;
      }

      SliderGUIWidget(const GUIDefinition &def):GUIWidget(def,3,6,GUIWidget::gridLayout,vertical(def)?Size(1,4):Size(4,1)){
        /// param_order = min,max,curr,step=1,orientation=("horizontal")|"vertical"
        bool deactivateDisplay = (def.numParams() >= 5) && (def.param(4) == "off");

        m_stepping = def.numParams() == 6 ? def.intParam(5) : 1;
        if(m_stepping < 1){
          ERROR_LOG("a slider gui component with stepping < 1 is not possible (using stepping 1)");
          m_stepping = 1;
        }
        /// min,max,curr,vertical ,"off" for no display
        
        m_piValue = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));

        int iVerticalFlag = vertical(def);
        int iMin = def.intParam(0);
        int iMax = def.intParam(1);
        int iCurr = def.intParam(2);

        if(iVerticalFlag){
          m_poSlider = new ThreadedUpdatableSlider(Qt::Vertical,def.parentWidget());
        }else{
          m_poSlider = new ThreadedUpdatableSlider(Qt::Horizontal,def.parentWidget());
        }
        m_poSlider->setStepping(m_stepping);

        if(def.hasToolTip()) m_poSlider->setToolTip(def.toolTip().c_str());

        addToGrid(m_poSlider);

        m_poSlider->setMinimum(iMin);
        m_poSlider->setMaximum(iMax);
        m_poSlider->setValue(iCurr);
        if(m_stepping != 1){
          m_poSlider->setSingleStep(m_stepping);
          m_poSlider->setTickInterval(m_stepping);
        }
        if(deactivateDisplay){
          m_poLCD = 0;
        }else{
          int nDigits = iclMax(QString::number(iMin).length(),QString::number(iMax).length());
          // what is this ???
          // int iAbsMax = iMax > -iMin ? iMax : -iMin;
          // int iAddOneForSign = iMax < -iMin;
          // m_poLCD = new QLCDNumber(QString::number(iAbsMax).length()+iAddOneForSign,def.parentWidget());
          m_poLCD = new QLCDNumber(nDigits,def.parentWidget());
          m_poLCD->display(iCurr);

          if(iVerticalFlag){
            addToGrid(m_poLCD,0,1,1,4);
          }else{
            addToGrid(m_poLCD,1,0,4,1);
          }
          //connect(m_poSlider,SIGNAL(valueChanged(int)),m_poLCD,SLOT(display(int)));
        }

        connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));

        m_bVerticalFlag = iVerticalFlag ? true : false;

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<SliderHandle>(def.handle(),SliderHandle(m_poSlider,this,m_poLCD));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return
        string("slider(MIN,MAX,CURR,ORIENTATION=horizontal,DISPLAY=on)[general params] \n")+
        string("\tMIN is the minimum value of the slider\n")+
        string("\tMAX is the maximum value of the slider\n")+
        string("\tCURR is the initializing value of the slider\n")+
        string("\tORIENTATION is horizontal or vertical\n")+
        string("\tDISPLAY can be \"on\" (default) or \"off\" \n")+
        gen_params();
      }
      virtual void processIO(){
        //cb();
        //iStep is handled as a value that must '%' the slider to 0
        int value = m_poSlider->value();
        value = (value / m_stepping) * m_stepping;
        *m_piValue = value;
        //        Thread::msleep(100);
        if(m_poLCD) m_poLCD->display(value);
      }
    private:
      ThreadedUpdatableSlider *m_poSlider;
      QLCDNumber *m_poLCD;
      int *m_piValue;
      bool m_bVerticalFlag;
      int m_stepping;

    };

  // }}}
    struct FloatSliderGUIWidget : public GUIWidget{
      // {{{ open

      static bool vertical(const GUIDefinition &def){
        return (def.numParams() >= 4) ? (def.param(3)=="vertical") : false;
      }

      FloatSliderGUIWidget(const GUIDefinition &def):GUIWidget(def,3,5,GUIWidget::gridLayout,vertical(def)?Size(1,4):Size(4,1)){
        bool deactivateDisplay = (def.numParams() == 5) && (def.param(4) == "off");
        //
        // y = mx+b
        // m =dy/dx = max-min/1000
        // b = min
        //float fMin, float fMax, float fCurr vertical|horizontal
        //

        /// param_order = min,max,curr,orientation=("horizontal")|"vertical"

        getGUI()->lockData();
        m_pfValue = &getGUI()->allocValue<float>(def.output(0),def.floatParam(2));
        getGUI()->unlockData();

        int iVerticalFlag = vertical(def);
        m_fMinVal = def.floatParam(0);
        m_fMaxVal = def.floatParam(1);
        float fCurr = def.floatParam(2);
        int nDigits = 6;

        m_fM = (m_fMaxVal-m_fMinVal)/10000.0;
        m_fB = m_fMinVal;

        if(iVerticalFlag){
          m_poSlider = new ThreadedUpdatableSlider(Qt::Vertical,def.parentWidget());
        }else{
          m_poSlider = new ThreadedUpdatableSlider(Qt::Horizontal,def.parentWidget());
        }
        if(def.hasToolTip()) m_poSlider->setToolTip(def.toolTip().c_str());

        addToGrid(m_poSlider);

        m_poSlider->setMinimum(0);
        m_poSlider->setMaximum(10000);
        m_poSlider->setValue(f2i(fCurr));

        if(deactivateDisplay){
          m_poLCD = 0;
        }else{
          m_poLCD = new QLCDNumber(nDigits,def.parentWidget());
          m_poLCD->display(fCurr);

          if(iVerticalFlag){
            addToGrid(m_poLCD,0,1,1,4);
          }else{
            addToGrid(m_poLCD,1,0,4,1);
          }
        }
        connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<FSliderHandle>(def.handle(),FSliderHandle(m_poSlider,&m_fMinVal,&m_fMaxVal,&m_fM,&m_fB,10000,this,m_poLCD));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return
        string("fslider(MIN,MAX,CURR,ORIENTATION=horizontal)[general params] \n")+
        string("\tMIN is the minimum value of the slider\n")+
        string("\tMAX is the maximum value of the slider\n")+
        string("\tCURR is the initializing value of the slider\n")+
        string("\tORIENTATION is horizontal or vertical\n")+
        string("\tDISPLAY can be \"on\" (default) or \"off\" \n")+
        gen_params();
      }
      virtual void processIO(){
        float value = i2f(m_poSlider->value());
        if(m_poLCD){
          m_poLCD->display(value);
        }
        *m_pfValue = value;
      }
    private:
      ThreadedUpdatableSlider *m_poSlider;
      QLCDNumber *m_poLCD;
      float *m_pfValue;
      float m_fM,m_fB;
      float m_fMinVal, m_fMaxVal;
      int f2i(float f){
        return (int)((f-m_fB)/m_fM);
      }
      float i2f(int i){
        return m_fM*i+m_fB;
      }
    };

  // }}}
    struct IntGUIWidget : public GUIWidget{
      // {{{ open
  public:
      IntGUIWidget(const GUIDefinition &def):GUIWidget(def,3){
        m_poLineEdit = new QLineEdit(def.parentWidget());
        m_poLineEdit->setValidator(new QIntValidator(def.intParam(0),def.intParam(1),0));
        m_poLineEdit->setText(QString::number(def.intParam(2)));

        if(def.hasToolTip()) m_poLineEdit->setToolTip(def.toolTip().c_str());

        QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));

        addToGrid(m_poLineEdit);

        getGUI()->lockData();
        m_piOutput = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
        getGUI()->unlockData();

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<IntHandle>(def.handle(),IntHandle(m_poLineEdit,this));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return
        string("int(MIN,MAX,CURR)[general params] \n")+
        string("\tMIN is the minimum allowed input value\n")+
        string("\tMAX is the maximum allowed input value\n")+
        string("\tCURR is the initial value of the textfield\n")+
        gen_params();
      }
      virtual void processIO(){
        bool iOk;
        int iVal = m_poLineEdit->text().toInt(&iOk);
        if(iOk){
          *m_piOutput = iVal;
        }
      }
    private:
      QLineEdit *m_poLineEdit;
      int *m_piOutput;
    };

  // }}}
    struct FloatGUIWidget : public GUIWidget{
      // {{{ open
  public:
      FloatGUIWidget(const GUIDefinition &def):GUIWidget(def,3){
        m_poLineEdit = new QLineEdit(def.parentWidget());
        m_poLineEdit->setValidator(new QDoubleValidator(def.floatParam(0),def.floatParam(1),20,0));
        m_poLineEdit->setText(QString::number(def.floatParam(2)));

        if(def.hasToolTip()) m_poLineEdit->setToolTip(def.toolTip().c_str());

        QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));

        addToGrid(m_poLineEdit);
        getGUI()->lockData();
        m_pfOutput = &getGUI()->allocValue<float>(def.output(0),def.intParam(2));
        getGUI()->unlockData();

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<FloatHandle>(def.handle(),FloatHandle(m_poLineEdit,this));
          getGUI()->unlockData();
        }

      }
      static string getSyntax(){
        return
        string("float(MIN,MAX,CURR)[general params] \n")+
        string("\tMIN is the minimum allowed input value\n")+
        string("\tMAX is the maximum allowed input value\n")+
        string("\tCURR is the initial value of the textfield\n")+
        gen_params();
      }
      virtual void processIO(){
        bool iOk;
        float fVal = m_poLineEdit->text().toFloat(&iOk);
        if(iOk){
          *m_pfOutput = fVal;
        }
      }
    private:
      QLineEdit *m_poLineEdit;
      float *m_pfOutput;
    };

  // }}}
    struct StringGUIWidget : public GUIWidget{
      // {{{ open
      class StringLenValidator : public QValidator{
      public:
        StringLenValidator(int iMaxLen):QValidator(0){
          this->iMaxLen = iMaxLen;
        }
        virtual State validate(QString &sInput,int &iPos)const{
          (void)iPos;
          return sInput.length()>iMaxLen ?  Invalid : Acceptable;
        }
      private:
        int iMaxLen;
      };

      StringGUIWidget(const GUIDefinition &def):GUIWidget(def,1,2){
        m_poLineEdit = new QLineEdit(def.parentWidget());
        m_poLineEdit->setValidator(new StringLenValidator(def.numParams() == 2 ? def.intParam(1) : 100));
        m_poLineEdit->setText(def.numParams()>=1 ? def.param(0).c_str() : "");

        if(def.hasToolTip()) m_poLineEdit->setToolTip(def.toolTip().c_str());

        QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));

        addToGrid(m_poLineEdit);

        getGUI()->lockData();
        m_psOutput = &getGUI()->allocValue<string>(def.output(0),def.param(0));
        getGUI()->unlockData();

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<StringHandle>(def.handle(),StringHandle(m_poLineEdit,m_psOutput,this));
          getGUI()->unlockData();
        }

      }
      static string getSyntax(){
        return
        string("string(TEXT,MAXLEN)[general params] \n")+
        string("\tTEXT is the initial value of the textfield\n")+
        string("\tMAXLEN is max. number of characters that might be written into the textfiled\n")+
        gen_params();
      }
      virtual void processIO(){
        getGUI()->lockData();
        *m_psOutput = m_poLineEdit->text().toLatin1().data();
        getGUI()->unlockData();
      }
    private:
      QLineEdit *m_poLineEdit;
      string *m_psOutput;
    };

  // }}}
    struct DispGUIWidget : public GUIWidget{
      // {{{ open

      static Size dim(const GUIDefinition &def, int facX=1, int facY=1){
        int nW = def.intParam(0);
        int nH = def.intParam(1);
        if(nW < 1) throw GUISyntaxErrorException(def.defString(),"NW must be > 0");
        if(nH < 1) throw GUISyntaxErrorException(def.defString(),"NW must be > 0");
        return Size(nW,nH);
      }

      DispGUIWidget(const GUIDefinition &def):GUIWidget(def,2,2,GUIWidget::gridLayout,dim(def,2,1)){

        Size size = dim(def);
        int nW = size.width;
        int nH = size.height;

        m_poLabelMatrix = new LabelMatrix(nW,nH);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<DispHandle>(def.handle(),DispHandle(m_poLabelMatrix,this));
          getGUI()->unlockData();
        }

        for(int x=0;x<nW;x++){
          for(int y=0;y<nH;y++){
            CompabilityLabel *l = new CompabilityLabel("",def.parentWidget());
            if(def.hasToolTip()) l->setToolTip(def.toolTip().c_str());

            (*m_poLabelMatrix)(x,y) = LabelHandle(l,this);
            addToGrid(l,x,y);
          }
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
      static string getSyntax(){
        return
        string("disp(NW,NH)[general params] \n")+
        string("\tNW is width of the display label matrix (must be > 0)")+
        string("\tNH is height of the display label matrix (must be > 0)")+
        gen_params();
      }

    private:
      LabelMatrix *m_poLabelMatrix;
    };

    // }}}
    struct ImageGUIWidget : public GUIWidget{
      // {{{ open
      ImageGUIWidget(const GUIDefinition &def):GUIWidget(def,0,0,GUIWidget::gridLayout,Size(16,12)){

        m_poWidget = new ICLWidget(def.parentWidget());
        if(def.hasToolTip()) m_poWidget->setInfoText(def.toolTip()); //.c_str());

        addToGrid(m_poWidget);


        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<ImageHandle>(def.handle(),ImageHandle(m_poWidget,this));
          getGUI()->unlockData();
        }

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
      static string getSyntax(){
        return string("image()[general params] \n")+
        gen_params();
      }
    private:
      ICLWidget *m_poWidget;
    };

    // }}}

    struct PlotGUIWidget : public GUIWidget{
      // {{{ open
      PlotGUIWidget(const GUIDefinition &def):GUIWidget(def,0,7,GUIWidget::gridLayout,Size(16,12)){

        m_plot = new PlotWidget(def.parentWidget());

        if(def.hasToolTip()) m_plot->setToolTip(def.toolTip().c_str());


        float minX = def.numParams()>=1 ? def.floatParam(0) : 0;
        float maxX = def.numParams()>=2 ? def.floatParam(1) : 0;
        float minY = def.numParams()>=3 ? def.floatParam(2) : 0;
        float maxY = def.numParams()>=4 ? def.floatParam(3) : 0;

        bool useGL = def.numParams()>=5 ? (def.param(4) == "GL" || def.param(4) == "gl") : false;

        std::string xAxisLabel = def.numParams()>=6 ? def.param(5) : str("");
        std::string yAxisLabel = def.numParams()>=7 ? def.param(6) : str("");

        if(xAxisLabel.length() && xAxisLabel != "-"){
          m_plot->setPropertyValue("labels.x-axis",xAxisLabel);
          m_plot->setPropertyValue("borders.bottom",55);
        }
        if(yAxisLabel.length() && yAxisLabel != "-"){
          m_plot->setPropertyValue("labels.y-axis",yAxisLabel);
          m_plot->setPropertyValue("borders.left",55);
        }


        m_plot->setDataViewPort(Range32f(minX,maxX), Range32f(minY,maxY));

        if(useGL){
          QGLWidget *gl = new QGLWidget(def.parentWidget());
          QLayout *layout = new QVBoxLayout(gl);
          layout->setSpacing(0);
          layout->setContentsMargins(0,0,0,0);
          gl->setLayout(layout);
          gl->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
          m_plot->setParent(gl);
          layout->addWidget(m_plot);
          addToGrid(gl);
        }else{
          addToGrid(m_plot);
        }

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<PlotHandle>(def.handle(),PlotHandle(m_plot,this));
          getGUI()->unlockData();
        }

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
      static string getSyntax(){
        return
        string("plot(X_VIEWPORT_MIN=0,X_VIEWPORT_MAX=0,Y_VIEWPORT_MIN=0,"
               "Y_VIEWPORT_MAX=0,GL=noGL,X_AXIS_LABEL=\"\",Y_AXIS_LABEL=\"\")[general params]\n")+
        string("\tX/Y_VIEWPORT_MIN/MAX are optionally given. The parameters define the data viewport in\n")+
        string("\tcreated PlotWidget. If min and max X are zero, the PlotWidget will automatically estimate\n")+
        string("\tthe X-data viewport. The same is true for the Y-data viewport. Please note, that\n")+
        string("\tthe data viewport can also later be set\n")+
        string("\tGL if the 5th parameter is set to GL, the widget will be embedded into an QGLWidget to enhance performance\n")+
        string("\tX_AXIS_LABEL and Y_AXIS_LABEL can be given optionally use \"-\" as placeholder for no label\n");

        gen_params();
      }
    private:
      PlotWidget *m_plot;
    };

    // }}}

    struct DrawGUIWidget : public GUIWidget{
      // {{{ open
      DrawGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(16,12)){
        m_poWidget = new ICLDrawWidget(def.parentWidget());
        if(def.hasToolTip()) m_poWidget->setInfoText(def.toolTip());
        //.c_str());if(def.hasToolTip()) m_poWidget->setToolTip(def.toolTip().c_str());

        if(def.numParams() == 1) {
          m_poWidget->setViewPort(parse<Size>(def.param(0)));
        }
        addToGrid(m_poWidget);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<DrawHandle>(def.handle(),DrawHandle(m_poWidget,this));
          getGUI()->unlockData();
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
      static string getSyntax(){
        return string("draw()[general params] \n")+
        gen_params();
      }
    private:
      ICLDrawWidget *m_poWidget;
    };

    // }}}


    struct MultiDrawGUIWidget : public GUIWidget{
      // {{{ open
      MultiDrawGUIWidget(const GUIDefinition &def):GUIWidget(def,1,2<<20,GUIWidget::vboxLayout){
        m_bAll = false;
        m_bDeep = true;

        bool x[2]= {false,false};
        bool err = false;
        for(unsigned int i=0;i<def.numParams();++i){
          if(def.param(i) == "!one") {
            m_bAll = false;
            if(x[0]) err = true;
            x[0] = true;
          }
          if(def.param(i) == "!all"){
            m_bAll = true;
            if(x[0]) err = true;
            x[0] = true;
          }
          if(def.param(i) == "!deepcopy"){
            m_bDeep = true;
            if(x[1]) err = true;
            x[1] = true;
          }
          if(def.param(i) == "!shallowcopy"){
            m_bDeep = false;
            if(x[1]) err = true;
            x[1] = true;
          }
        }
        if(err){
          throw GUISyntaxErrorException(def.defString(),"any two parameters are doubled or contradictory");
        }

        m_poTabBar = new QTabBar(def.parentWidget());
        m_poDrawWidget = new ICLDrawWidget(def.parentWidget());

        if(def.hasToolTip()) m_poDrawWidget->setToolTip(def.toolTip().c_str());


        m_poTabBar->setMaximumHeight(25);
        getGUIWidgetLayout()->addWidget(m_poTabBar);
        getGUIWidgetLayout()->addWidget(m_poDrawWidget);

        for(unsigned int i=0;i<def.numParams();++i){
          std::string s = def.param(i);
          if(s.length() && s[0] == '!') continue;
          m_poTabBar->addTab(s.c_str());
        }

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<MultiDrawHandle>(def.handle(),MultiDrawHandle(m_poDrawWidget,m_poTabBar,&m_vecImageBuffer,m_bAll, m_bDeep,this));
          getGUI()->unlockData();
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
      static string getSyntax(){
        return string("multidraw(TAB-LIST)[general params] \n")+
        gen_params();
      }
      private:

      QTabBar *m_poTabBar;
      ICLDrawWidget *m_poDrawWidget;
      std::vector<ImgBase *> m_vecImageBuffer;

      bool m_bAll;
      bool m_bDeep;
    };

    // }}}

  #ifdef ICL_HAVE_OPENGL
    struct DrawGUIWidget3D : public GUIWidget{
      // {{{ open
      DrawGUIWidget3D(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(16,12)){
        m_poWidget3D = new ICLDrawWidget3D(def.parentWidget());
        if(def.numParams() == 1) {
          m_poWidget3D->setViewPort(parse<Size>(def.param(0)));
        }

        if(def.hasToolTip()) m_poWidget3D->setInfoText(def.toolTip());
                //if(def.hasToolTip()) m_poWidget3D->setToolTip(def.toolTip().c_str());



        addToGrid(m_poWidget3D);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<DrawHandle3D>(def.handle(),DrawHandle3D(m_poWidget3D,this));
          getGUI()->unlockData();
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
      static string getSyntax(){
        return string("draw()[general params] \n")+
        gen_params();
      }
    private:
      ICLDrawWidget3D *m_poWidget3D;
    };
  #endif

    // }}}
    struct ComboGUIWidget : public GUIWidget{
      // {{{ open
      ComboGUIWidget(const GUIDefinition &def):GUIWidget(def,1,2<<20,GUIWidget::gridLayout,Size(4,1)){
        if(def.numParams() < 1) throw GUISyntaxErrorException(def.defString(),"at least 1 param needed here!");

        m_poCombo = new QComboBox(def.parentWidget());

        if(def.hasToolTip()) m_poCombo->setToolTip(def.toolTip().c_str());

        addToGrid(m_poCombo);

        unsigned int selectedIndex = 0;
        string sFirst = def.param(0);
        for(unsigned int i=0;i<def.numParams();i++){
          const std::string &s = def.param(i);
          if(s.length() && s[0]=='!'){
            sFirst = s.substr(1);
            selectedIndex = i;
            m_poCombo->addItem(s.substr(1).c_str());
          }else{
            m_poCombo->addItem(s.c_str());
          }
        }

        getGUI()->lockData();
        m_psCurrentText = &getGUI()->allocValue<string>(def.output(0),sFirst);
        getGUI()->unlockData();

        m_poCombo->setCurrentIndex(selectedIndex);

        connect(m_poCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(ioSlot()));

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<ComboHandle>(def.handle(),ComboHandle(m_poCombo,this));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return string("combo(entry1,entry2,entry3)[general params] \n")+
        string("\tentryX is the the X-th entry of the combo box\n")+
        string("\tif any entry has a '!'-prefix, this entry will be selected initially\n")+
        gen_params();
      }
      virtual void processIO(){
        getGUI()->lockData();
        *m_psCurrentText = m_poCombo->currentText().toLatin1().data();
        getGUI()->unlockData();
      }

    private:
      string *m_psCurrentText;
      QComboBox *m_poCombo;
    };

    // }}}
    struct SpinnerGUIWidget : public GUIWidget{
      // {{{ open
  public:
      SpinnerGUIWidget(const GUIDefinition &def):GUIWidget(def,3,3,GUIWidget::gridLayout,Size(4,1)){
        m_poSpinBox = new QSpinBox(def.parentWidget());
        m_poSpinBox->setRange(def.intParam(0),def.intParam(1));
        m_poSpinBox->setValue(def.intParam(2));

        if(def.hasToolTip()) m_poSpinBox->setToolTip(def.toolTip().c_str());


        QObject::connect(m_poSpinBox,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));

        addToGrid(m_poSpinBox);

        getGUI()->lockData();
        m_piOutput = &getGUI()->allocValue<int>(def.output(0),def.intParam(2));
        getGUI()->unlockData();

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<SpinnerHandle>(def.handle(),SpinnerHandle(m_poSpinBox,this));
          getGUI()->unlockData();
        }

      }
      static string getSyntax(){
        return
        string("spinner(MIN,MAX,CURR)[general params] \n")+
        string("\tMIN is the minimum possible value\n")+
        string("\tMAX is the maximum possible value\n")+
        string("\tCURR is the initial value of the spinbox\n")+
        gen_params();
      }
      virtual void processIO(){
        *m_piOutput = m_poSpinBox->value();
      }
    private:
      QSpinBox *m_poSpinBox;
      int *m_piOutput;
    };

  // }}}

    struct FPSGUIWidget : public GUIWidget{
      // {{{ open
      FPSGUIWidget(const GUIDefinition &def):GUIWidget(def,0,1,GUIWidget::gridLayout,Size(5,2)){
        int np = def.numParams();
        int fpsEstimatorTimeWindow = np ? def.intParam(0) : 10;

        m_poLabel = new CompabilityLabel("fps...",def.parentWidget());
        if(def.hasToolTip()) m_poLabel->setToolTip(def.toolTip().c_str());


        addToGrid(m_poLabel);

        if(def.handle() != ""){
          getGUI()->lockData();
          getGUI()->allocValue<FPSHandle>(def.handle(),FPSHandle(fpsEstimatorTimeWindow,m_poLabel,this));
          getGUI()->unlockData();
        }
      }
      static string getSyntax(){
        return string("fps(TIME_WINDOW=10)[general params] \n"
                      "  TIME_WINDOW is the number of timesteps, that are used as \n"
                      "  Low-Pass-Filter for estimated fps counts\n")
                      +gen_params();
      }
      virtual Size getDefaultSize() {
        return Size(4,1);
      }
    private:
      CompabilityLabel *m_poLabel;
    };


    /// template for creating arbitrary GUIWidget's
    template<class T>
    GUIWidget *create_widget_template(const GUIDefinition &def){
      // {{{ open
      T *t = 0;
      try{
        t = new  T(def);
      }catch(GUISyntaxErrorException &ex){
        printf("%s\nsyntax is: %s\n",ex.what(),T::getSyntax().c_str());
        return 0;
      }
      return t;
    }

    // }}}

    static std::map<std::string,GUI::CreatorFunction> &get_registered_widget_types(){
      static std::map<std::string,GUI::CreatorFunction> m;
      return m;
    }

    void GUI::register_widget_type(const std::string &tag, GUI::CreatorFunction f){
      get_registered_widget_types()[tag] = f;
    }


    struct DefaultWidgetTypeRegister{
      DefaultWidgetTypeRegister(){
        GUI::register_widget_type("hbox",create_widget_template<HBoxGUIWidget>);
        GUI::register_widget_type("vbox",create_widget_template<VBoxGUIWidget>);
        GUI::register_widget_type("hscroll",create_widget_template<HScrollGUIWidget>);
        GUI::register_widget_type("vscroll",create_widget_template<VScrollGUIWidget>);
        GUI::register_widget_type("button",create_widget_template<ButtonGUIWidget>);
        GUI::register_widget_type("border",create_widget_template<BorderGUIWidget>);
        GUI::register_widget_type("buttongroup",create_widget_template<ButtonGroupGUIWidget>);
        GUI::register_widget_type("togglebutton",create_widget_template<ToggleButtonGUIWidget>);
        GUI::register_widget_type("checkbox",create_widget_template<CheckBoxGUIWidget>);
        GUI::register_widget_type("label",create_widget_template<LabelGUIWidget>);
        GUI::register_widget_type("slider",create_widget_template<SliderGUIWidget>);
        GUI::register_widget_type("fslider",create_widget_template<FloatSliderGUIWidget>);
        GUI::register_widget_type("int",create_widget_template<IntGUIWidget>);
        GUI::register_widget_type("float",create_widget_template<FloatGUIWidget>);
        GUI::register_widget_type("string",create_widget_template<StringGUIWidget>);
        GUI::register_widget_type("disp",create_widget_template<DispGUIWidget>);
        GUI::register_widget_type("image",create_widget_template<ImageGUIWidget>);
        GUI::register_widget_type("state",create_widget_template<StateGUIWidget>);
        GUI::register_widget_type("draw",create_widget_template<DrawGUIWidget>);
  #ifdef ICL_HAVE_OPENGL
        GUI::register_widget_type("draw3D",create_widget_template<DrawGUIWidget3D>);
  #endif
        GUI::register_widget_type("combo",create_widget_template<ComboGUIWidget>);
        GUI::register_widget_type("spinner",create_widget_template<SpinnerGUIWidget>);
        GUI::register_widget_type("fps",create_widget_template<FPSGUIWidget>);
        GUI::register_widget_type("multidraw",create_widget_template<MultiDrawGUIWidget>);
        GUI::register_widget_type("tab",create_widget_template<TabGUIWidget>);
        GUI::register_widget_type("hsplit",create_widget_template<HSplitterGUIWidget>);
        GUI::register_widget_type("vsplit",create_widget_template<VSplitterGUIWidget>);
        GUI::register_widget_type("camcfg",create_widget_template<CamCfgGUIWidget>);
        GUI::register_widget_type("prop",create_widget_template<ConfigurableGUIWidget>);
        GUI::register_widget_type("color",create_widget_template<ColorGUIWidget>);
        GUI::register_widget_type("ps",create_widget_template<ProcessMonitorGUIWidget>);
        GUI::register_widget_type("plot",create_widget_template<PlotGUIWidget>);
      }
    } defaultWidgetTypeRegisterer;

    /// NEW CREATOR MAP ENTRIES HERE !!!
    /*
        This function is called by the GUI::create function,
        to create arbitrary widgets. To accelerate the widget creation process
        it build a CreatorFuncMap which uses the GUIDefinitinos type-string as
        identifier to estimate which creation function must be called.         */
    GUIWidget *create_widget(const GUIDefinition &def){
      // {{{ open
      typedef std::map<std::string,GUI::CreatorFunction> tmap;
      tmap &m = get_registered_widget_types();
      tmap::iterator it = m.find(def.type());

      if(it == m.end()){
        throw ICLException("unable to create GUI component with type '" + def.type() + "' (unknown type)");
      }
      return it->second(def);
    }

    // }}}

    string extract_label(string s){
      // {{{ open

      string::size_type p = s.find('[');
      if(p == string::npos) return "";
      s = s.substr(p+1);
      if(!s.length()) return "";
      if(s[s.length()-1] != ']') return "";
      StrTok t(s.substr(0,s.length()-1),"@");
      while(t.hasMoreTokens()){
        const string &s2 = t.nextToken();
        if(!s2.find("label",0)){
          if(s2.length() < 7) return "";
          return s2.substr(6);
        }
      }
      return "";
    }

    // }}}
    string extract_minsize(string s){
      // {{{ open

      string::size_type p = s.find('[');
      if(p == string::npos) return "";
      s = s.substr(p+1);
      if(!s.length()) return "";
      if(s[s.length()-1] != ']') return "";
      StrTok t(s.substr(0,s.length()-1),"@");
      while(t.hasMoreTokens()){
        const string &s2 = t.nextToken();
        if(!s2.find("minsize",0)){
          if(s2.length() < 9) return "";
          return s2.substr(8);
        }
      }
      return "";
    }

    // }}}
    string extract_maxsize(string s){
      // {{{ open

      string::size_type p = s.find('[');
      if(p == string::npos) return "";
      s = s.substr(p+1);
      if(!s.length()) return "";
      if(s[s.length()-1] != ']') return "";
      StrTok t(s.substr(0,s.length()-1),"@");
      while(t.hasMoreTokens()){
        const string &s2 = t.nextToken();
        if(!s2.find("maxsize",0)){
          if(s2.length() < 9) return "";
          return s2.substr(8);
        }
      }
      return "";
    }

    // }}}
    string extract_size(string s){
      // {{{ open

      string::size_type p = s.find('[');
      if(p == string::npos) return "";
      s = s.substr(p+1);
      if(!s.length()) return "";
      if(s[s.length()-1] != ']') return "";
      StrTok t(s.substr(0,s.length()-1),"@");
      while(t.hasMoreTokens()){
        const string &s2 = t.nextToken();
        if(!s2.find("size",0)){
          if(s2.length() < 6) return "";
          return s2.substr(5);
        }
      }
      return "";
    }

    // }}}

    string remove_label(const string &s, const std::string &label){
      // {{{ open
      string toRemove = string("@label=")+label;
      unsigned int p = s.find(toRemove);
      return s.substr(0,p) + s.substr(p+toRemove.length());
    }

    // }}}


    GUI::GUI(QWidget *parent):
      m_sDefinition("vbox"),m_poWidget(0),m_bCreated(false),m_poParent(parent){
    }
    GUI::GUI(const std::string &definition,QWidget *parent):
      // {{{ open
      m_sDefinition(definition),m_poWidget(0),m_bCreated(false),m_poParent(parent){
    }

    GUI::GUI(const GUIComponent &component, QWidget *parent):
      m_sDefinition(component.toString()), m_poWidget(0),m_bCreated(false),m_poParent(parent){
    }


    // }}}
    GUI::GUI(const GUI &g,QWidget *parent):
      // {{{ open
      m_sDefinition(g.createDefinition()),
      m_children(g.m_children),
      m_poWidget(NULL),m_bCreated(false),
      m_poParent(parent){
    }
    // }}}


    GUI &GUI::operator=(const GUI &other){
      m_sDefinition = other.createDefinition();
      m_children = other.m_children;
      m_poWidget = NULL;
      m_bCreated = false;
      m_poParent = other.m_poParent;
      m_oDataStore = other.m_oDataStore;
      return *this;
    }

    GUI::~GUI(){
      // this leads to seg-faults
      //delete m_poWidget;
    }

    /// adds a new GUI component
    GUI &GUI::operator<<(const GUIComponent &component){
      // TODO: this is just a slow fallback right now
      // as a first attempt, we could bypass the parsing
      // of the serialized string version ...
      return (*this) << component.toString();
    }

    GUI &GUI::operator<<(const std::string &definition){
      // {{{ open
      if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
      if(definition.length() > 100000) {
        throw GUISyntaxErrorException("-- long text --","definition string was too large! (>100000 characters)");
      }
      if(!definition.length() || definition == "dummy"){
        return *this;
      }

      if(definition.size() && definition[0] == '!'){
        if(definition == "!show"){
          show();
          return *this;
        }else if(definition == "!create"){
          create();
          return *this;
        }
        throw GUISyntaxErrorException(definition,"wrong !xxx command found in GUI::operator<< : allowed are \"!show\" and \"!create\")");
      }

      /**
          //#ifndef WIN32
          usleep(1000*100);
          #else
        Sleep(100);
          //#endif
      **/

      string label = extract_label(definition);
      string minsize = extract_minsize(definition);
      string maxsize = extract_maxsize(definition);
      string size = extract_size(definition);

      Size S11(1,1);
      if(minsize.length()) minsize = string("@minsize=")+str(parse<Size>(minsize)+S11);
      if(maxsize.length()) maxsize = string("@maxsize=")+str(parse<Size>(maxsize)+S11);
      if(size.length()) size = string("@size=")+str(parse<Size>(size)+S11);

      if(label.length()){
        string rest = remove_label(definition,label);
        //      return ( (*this) << ( GUI(string("border("+label+")["+minsize+maxsize+size+"]")) << rest ) );
        GUI border(string("border(")+label+")["+minsize+maxsize+size+"]");
        border << rest;
        *this << border;
        return *this;
      }else{
        m_children.push_back(new GUI(definition));
        return *this;
      }
    }

    void GUI::to_string_recursive(const GUI *gui, std::ostream &str, int level){
      GUIDefinition def(gui->m_sDefinition, const_cast<GUI*>(gui));
      str << std::string(level*2,' ') << "<" << def.type();
      int nParams = def.numParams();
      std::ostringstream sparams;
      for(int i=0;i<nParams;++i){
        sparams << def.param(0);
        if(i<nParams-1) sparams << ",";
      }
      if(nParams){
        str << " args=\"" << sparams.str() << "\"";
      }
      if(def.label().length()) str << " label=\"" <<  def.label() << "\"";
      if(def.handle().length()) str << " handle=\"" << def.handle() << "\"";
      if(def.size() != Size::null) str << " size=\"" << def.size() << "\"";
      if(def.minSize() != Size::null) str << " minsize=\"" << def.minSize() << "\"";
      if(def.maxSize() != Size::null) str << " maxsize=\"" << def.maxSize() << "\"";
      if(def.margin() != 2) str << " margin=\"" << def.margin() << "\"";
      if(def.spacing() != 2) str << " spacing=\"" << def.spacing() << "\""; 
      if(def.hasToolTip()) str << " tooltip=\"" << def.toolTip() << "\"";
      if(gui->m_children.size()){
        str << ">" << std::endl;
        for(size_t i=0;i<gui->m_children.size();++i){
          to_string_recursive(gui->m_children[i], str, level+1);
        }
        str << std::string(level*2,' ') << "</" << def.type() << ">" << std::endl;
      }else{
        str << "/>" << std::endl;
      }
    }

    std::string GUI::createXMLDescription() const{
      std::ostringstream str;
      to_string_recursive(this,str,0);
      return str.str();
    }

    // }}}
    GUI &GUI::operator<<(const GUI &g){
      // {{{ open

      if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }

      if(g.isDummy()){
        return *this;
      }

      std::string def = g.createDefinition();
      string label = extract_label(def);
      string minsize = extract_minsize(def);
      string maxsize = extract_maxsize(def);
      string size = extract_size(def);

      Size S11(1,1);
      if(minsize.length()) minsize = string("@minsize=")+str(parse<Size>(minsize)+S11);
      if(maxsize.length()) maxsize = string("@maxsize=")+str(parse<Size>(maxsize)+S11);
      if(size.length()) size = string("@size=")+str(parse<Size>(size)+S11);


      if(label.length()){
        GUI gNew(g);
        def = gNew.createDefinition();
        if(def.length() > 1000000) {
          throw GUISyntaxErrorException("-- long text --","definition string was too large! (>100000 characters)");
        }
        gNew.m_sDefinition = remove_label(def,label);
        return ( *this << (  GUI(string("border(")+label+")["+minsize+maxsize+size+"]") << gNew ) );
      }else{
        m_children.push_back(new GUI(g));
        return *this;
      }


      //    m_children.push_back(new GUI(g));
      return *this;
    }

    // }}}

    void GUI::create(QLayout *parentLayout,ProxyLayout *proxy,QWidget *parentWidget, DataStore *ds){
      // {{{ open
      if(ds) m_oDataStore = *ds;
      try{
        if(isDummy()){
          throw ICLException("cannot create a \"dummy\"-GUI. (Dummy GUI's are GUI-instances\n"
                             "that are created from an empty string or from the string \"dummy\") ");
        }
        GUIDefinition def(createDefinition(),this,parentLayout,proxy,parentWidget);

        m_poWidget = create_widget(def);

        if(!parentWidget){
          //        std::cout << "setting window title:" << QApplication::applicationName().toLatin1().data() << std::endl;
          //m_poWidget->setWindowTitle(File(QApplication::arguments().at(0).toLatin1().data()).getBaseName().c_str());
          m_poWidget->setWindowTitle(File(QApplication::applicationName().toLatin1().data()).getBaseName().c_str());
		  //SHOW(m_poWidget->testAttribute(Qt::WA_QuitOnClose))
        }

        if(!m_poWidget){
          ERROR_LOG("Widget could not be created ( aborting to avoid errors ) \n");
          exit(0);
        }
        QLayout *layout = m_poWidget->getGUIWidgetLayout();
        ProxyLayout *proxy = m_poWidget->getProxyLayout();

        if(!layout && !proxy && m_children.size()){
          ERROR_LOG("GUI widget has noGUI layout, "<< m_children.size() <<" child components can't be added!");
          return;
        }
        for(unsigned int i=0;i<m_children.size();i++){
          m_children[i]->create(layout,proxy,m_poWidget,&m_oDataStore);
        }
        m_bCreated = true;
      }catch(GUISyntaxErrorException &ex){
        ERROR_LOG(ex.what());
        exit(0);
      }

    }

    // }}}

    bool GUI::isVisible() const{
      if(!m_bCreated) return false;
      return m_poWidget->isVisible();
    }

    void GUI::switchVisibility(){
      if(isVisible()) hide();
      else show();
    }

    void GUI::hide(){
      if(m_bCreated){
        m_poWidget->setVisible(false);
      }else{
        ERROR_LOG("unable to hide GUI that has not been created yet, call create() or show() first!");
      }
    }

    void GUI::create(){
      if(!m_bCreated){
        if(m_poParent){
          create(m_poParent->layout(),0,m_poParent,0);
        }else{
          create(0,0,0,0);
        }
      }
    }


    bool GUI::hasBeenCreated() const{
      return m_poWidget;
    }


    void GUI::show(){
      // {{{ open
      create();
      m_poWidget->setVisible(true);
    }

    // }}}

    void GUI::waitForCreation(){
      while(!m_bCreated){
  #ifndef WIN32
                usleep(1000*100);
  #else
                Sleep(100);
  #endif
      }
    }

    void GUI::registerCallback(const Callback &cb, const std::string &handleNamesList, char delim){
      std::string delims; delims+=delim;

      StrTok tok(handleNamesList,delims);
      while(tok.hasMoreTokens()){
        get<GUIHandleBase>(tok.nextToken(),false).registerCallback(cb);
      }
    }

    void GUI::registerCallback(const ComplexCallback &cb, const std::string &handleNamesList, char delim){
      std::string delims; delims+=delim;

      StrTok tok(handleNamesList,delims);
      while(tok.hasMoreTokens()){
        get<GUIHandleBase>(tok.nextToken(),false).registerCallback(cb);
      }
    }

    void GUI::removeCallbacks(const std::string &handleNamesList, char delim){
      std::string delims; delims+=delim;

      StrTok tok(handleNamesList,delims);
      while(tok.hasMoreTokens()){
        get<GUIHandleBase>(tok.nextToken(),false).removeCallbacks();
      }
    }

  } // namespace qt
}

//  LocalWords:  if
