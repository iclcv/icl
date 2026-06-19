// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <icl/utils/StrTok.h>
#include <icl/utils/SteppingRange.h>
#include <icl/utils/ProcessMonitor.h>
#include <icl/utils/Size.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/prop/Constraints.h>
#include <icl/io/source/ImageSource.h>
#include <icl/io/detail/SourceBackend.h>
#include <icl/io/source/DeviceDescription.h>

#include <icl/qt/GUI.h>
#include <icl/qt/GUIWidget.h>
#include <icl/qt/ContainerGUIComponents.h>
#include <icl/qt/GUISyntaxErrorException.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Array2D.h>
#include <icl/qt/Widget.h>
#include <icl/utils/File.h>

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

#include <icl/qt/ProxyLayout.h>

#include <icl/qt/ButtonHandle.h>
#include <icl/qt/BoxHandle.h>
#include <icl/qt/TabHandle.h>
#include <icl/qt/BorderHandle.h>
#include <icl/qt/ButtonGroupHandle.h>
#include <icl/qt/LabelHandle.h>
#include <icl/qt/StateHandle.h>
#include <icl/qt/SliderHandle.h>
#include <icl/qt/FSliderHandle.h>
#include <icl/qt/IntHandle.h>
#include <icl/qt/FloatHandle.h>
#include <icl/qt/StringHandle.h>
#include <icl/qt/ComboHandle.h>
#include <icl/qt/SpinnerHandle.h>
#include <icl/qt/ImageHandle.h>
#include <icl/qt/DrawHandle.h>
#include <icl/qt/DrawHandle3D.h>
#include <icl/qt/DispHandle.h>
#include <icl/qt/FPSHandle.h>
#include <icl/qt/CheckBoxHandle.h>
#include <icl/qt/MultiDrawHandle.h>
#include <icl/qt/SplitterHandle.h>
#include <icl/qt/ColorHandle.h>
#include <icl/qt/PlotHandle.h>
#include <icl/qt/PlotWidget.h>
#include <icl/qt/Quick2.h>
#include <QCheckBox>
#include <QtCore/QTimer>
#include <QtCore/QMetaObject>

#include <QFileDialog>

#include <icl/qt/CamCfgWidget.h>
#include <icl/utils/StringUtils.h>
#include <icl/qt/ToggleButtonWidget.h>

#include <icl/qt/Widget.h>
#include <icl/qt/DrawWidget.h>
#ifdef ICL_HAVE_OPENGL
#include <icl/qt/DrawWidget3D.h>
#endif
#include <icl/qt/ThreadedUpdatableSlider.h>
#include <icl/qt/ThreadedUpdatableTextView.h>
#include <icl/qt/ColorLabel.h>
#include <icl/qt/ui.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/cc/Color.h>
#include <QProgressBar>

#include <map>
#include <set>

#ifndef ICL_SYSTEM_WINDOWS
  #include <unistd.h>
#include <mutex>
#endif
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;


namespace icl{
  namespace qt{
    // VolatileUpdater / VolatileImageUpdater retired — Info and ImageView
    // properties update through the unified callback push channel
    // (enqueuePropertyUpdate → flushPendingPropertyUpdates → applyPropertyToWidget).
    // Rapid writes coalesce to one widget flush per GUI event-loop
    // tick, so even per-frame updates from a grab path are bounded.

    static const std::string &gen_params(){

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


    /// special gui component for visualizing process information
    struct ProcessMonitorGUIWidget : public GUIWidget{
      QTimer updater;
      ProcessMonitor *pm;
      ProcessMonitor::Info info;

      QLabel *threadCountLabel,*memoryUsageLabel;
      QProgressBar *cpuBar, *cpuBarThis;
      bool rangeSet;
      QCheckBox *disabled;

      ProcessMonitorGUIWidget(const Ps &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(6,3)){
        rangeSet = false;
        float fps = c.updateFPS;
        if(fps <= 0 || fps > 10) throw ICLException("Ps: updateFPS must be in range ]0,10]");

        if(!c.options().tooltip.empty()){
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


      static std::string getSyntax(){
        return std::string("ps(fps in ]0...10] = 10)[general params]\n")+gen_params();
      }

      virtual void processIO(){
        if(disabled->checkState() == Qt::Checked) return;

        info = pm->getInfo();
        cpuBar->setValue(static_cast<int>(info.allCpuUsage));
        if(!rangeSet){
          rangeSet = true;
          cpuBarThis->setRange(0,info.numCPUs * 100);
        }
        cpuBarThis->setValue(static_cast<int>(info.cpuUsage));

        std::ostringstream s;
        s << info.numThreads << " (" << info.numCPUs << " cores) ";
        threadCountLabel->setText(s.str().c_str());

        memoryUsageLabel->setText((str(info.memoryUsage)+" MB").c_str());
        update();
      }
    };

    // quite complex component for embedded property component 'prop'
    struct ConfigurableGUIWidget : public GUIWidget{

      Configurable *conf;
      GUI gui;
      GUI sub_gui;
      bool deactivateExec;
      std::string processingProperty;
      std::map<std::string,std::string, std::less<>> deferredAssignList;

      // Coalescing queue for async widget updates.  Writer threads
      // (anyone firing a Configurable callback) add property names
      // here and schedule a single GUI-thread flush via
      // QMetaObject::invokeMethod(Qt::QueuedConnection).  The flush
      // reads the latest value for each pending property from the
      // Configurable and updates the matching widget — all on the
      // GUI thread, so no cross-thread widget mutation and no lock
      // inversion against writer-side mutexes (m_grabMutex, etc.).
      std::mutex pendingMutex;
      std::set<std::string> pendingPropertyUpdates;
      bool updateScheduled = false;

      // Parallel coalescing for child-set rebuilds.  Fires from
      // Configurable::onChildSetChanged when add/removeChildConfigurable
      // happens at runtime (codec swap on ImageCompressor, backend
      // swap on ImageSource, ...).  Rebuild tears the widget tree
      // down and walks the property list again — always on the GUI
      // thread, at most once per event-loop tick.
      bool rebuildScheduled = false;

      // Callback tokens for unregistering in the dtor.  The Configurable
      // may outlive this widget (e.g. a ImageSource wired to a Prop
      // that gets torn down when the GUI closes) — without unregistering,
      // the captured `this` dangles and the next fire segfaults.
      Configurable::CallbackToken propChangeToken = 0;
      Configurable::CallbackToken childSetToken   = 0;

      struct StSt{
        std::string full,half;
        StSt(const std::string &full, const std::string &half):full(full),half(half){}
      };

      // Dispatch off the property's structured constraint
      // (prop::Range<T>, prop::Menu<T>, prop::Flag, ...).  Kept in sync
      // with add_component / propertyChanged — all three sites share
      // the same handle-prefix scheme ("#r#" for slider, "#R#" for
      // spinbox, ...).
      void update_all_components(){
        namespace up = utils::prop;
        namespace cp = core::prop;
        for(const std::string &p : conf->getPropertyListWithoutDeactivated()){
          auto h = conf->prop(p);
          const std::any &c = h.constraint;
          if(auto *r = std::any_cast<up::Range<float>>(&c)){
            if(r->ui == up::UI::Spinbox){
              gui["#F#"+p] = h.as<float>();
            }else{
              gui["#r#"+p] = h.as<float>();       // FSliderHandle::operator=(float)
            }
          }else if(auto *r = std::any_cast<up::Range<int>>(&c)){
            if(r->ui == up::UI::Spinbox){
              gui["#R#"+p] = h.as<int>();          // SpinnerHandle::operator=(int)
            }else{
              gui["#r#"+p] = h.as<int>();          // SliderHandle::operator=(int)
            }
          }else if(std::any_cast<up::Menu<std::string>>(&c) ||
                   std::any_cast<up::Menu<int>>(&c) ||
                   std::any_cast<up::Menu<float>>(&c)){
            gui["#m#"+p] = h.as<std::string>();    // ComboHandle::operator=(string)
          }else if(std::any_cast<up::Flag>(&c)){
            gui["#f#"+p] = h.as<bool>();
          }else if(std::any_cast<up::Info>(&c)){
            gui["#i#"+p] = h.as<std::string>();
          }else if(std::any_cast<up::Text>(&c)){
            gui["#S#"+p] = h.as<std::string>();
          }else if(std::any_cast<cp::Color>(&c)){
            gui["#C#"+p] = h.as<Color>();
          }
          // up::Command — fire-and-forget, no value to restore.
          // cp::ImageView — refreshed by VolatileImageUpdater timer.
          // Empty constraint — unknown/malformed legacy property; skip.
        }
      }

      // Build the comma-separated Combo entry list from a Menu's choices,
      // marking the current value with a leading '!'.  Choices of types
      // other than std::string are stringified.
      template<class T>
      static std::string build_combo_list(const std::vector<T> &choices,
                                          const std::string &current){
        std::vector<std::string> l;
        l.reserve(choices.size());
        for(const T &x : choices) l.push_back(utils::str(x));
        for(std::string &s : l) if(s == current) s = '!' + s;
        return utils::cat(l,",");
      }

      // Dispatch off the property's structured constraint.  Shares the
      // handle-prefix scheme with update_all_components /
      // propertyChanged.
      void add_component(GUI &gui,const StSt &p, std::ostringstream &ostr, GUI &timerGUI){
        namespace up = utils::prop;
        namespace cp = core::prop;
        auto h  = conf->prop(p.full);
        const std::any    &c  = h.constraint;
        const std::string &tt = h.tooltip;

        if(auto *r = std::any_cast<up::Range<float>>(&c)){
          float v = h.as<float>();
          if(r->ui == up::UI::Spinbox){
            // Rare: float-valued spinbox — no QSpinBox equivalent, so
            // use the Float widget (QLineEdit + double validator) which
            // preserves the "free-form entry" feel.
            std::string handle = "#F#"+p.full;
            ostr << '\1' << handle;
            gui << Float(r->min, r->max, v, {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
          }else{
            std::string handle = "#r#"+p.full;
            ostr << '\1' << handle;
            gui << FSlider(r->min, r->max, v, {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
          }
        }else if(auto *r = std::any_cast<up::Range<int>>(&c)){
          int v = h.as<int>();
          if(r->ui == up::UI::Spinbox){
            std::string handle = "#R#"+p.full;
            ostr << '\1' << handle;
            gui << Spinner(r->min, r->max, v, {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
          }else{
            std::string handle = "#r#"+p.full;
            ostr << '\1' << handle;
            int step = (r->step == 0) ? 1 : r->step;
            gui << Slider(r->min, r->max, v, {.vertical=false, .step=step, .handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
          }
        }else if(auto *m = std::any_cast<up::Menu<std::string>>(&c)){
          std::string handle = "#m#"+p.full;
          ostr << '\1' << handle;
          gui << Combo(build_combo_list(m->choices, h.as<std::string>()), {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
        }else if(auto *m = std::any_cast<up::Menu<int>>(&c)){
          std::string handle = "#m#"+p.full;
          ostr << '\1' << handle;
          gui << Combo(build_combo_list(m->choices, h.as<std::string>()), {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
        }else if(auto *m = std::any_cast<up::Menu<float>>(&c)){
          std::string handle = "#m#"+p.full;
          ostr << '\1' << handle;
          gui << Combo(build_combo_list(m->choices, h.as<std::string>()), {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
        }else if(std::any_cast<up::Command>(&c)){
          std::string handle = "#c#"+p.full;
          ostr << '\1' << handle;
          gui << Button(p.half, {.handle=handle, .tooltip=tt, .minSize={12, 2}});
        }else if(std::any_cast<up::Info>(&c)){
          std::string handle = "#i#"+p.full;
          ostr << '\1' << handle;
          gui << Label(h.as<std::string>(), {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
          // Refreshed through the callback push channel on every
          // write; coalesced to one widget flush per GUI tick.
        }else if(std::any_cast<up::Flag>(&c)){
          std::string handle = "#f#"+p.full;
          ostr << '\1' << handle;
          gui << CheckBox(p.half, {.checked=h.as<bool>(), .handle=handle, .tooltip=tt, .minSize={12, 2}});
        }else if(auto *t = std::any_cast<up::Text>(&c)){
          std::string handle = "#S#"+p.full;
          ostr << '\1' << handle;
          int max_len = t->maxLength ? t->maxLength : 100;
          std::string value = h.as<std::string>();
          if(!value.length()) value = " ";
          gui << String(value, {.maxLen=max_len, .handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
        }else if(std::any_cast<cp::Color>(&c)){
          std::string handle = "#C#"+p.full;
          ostr << '\1' << handle;
          Color col = h.as<Color>();
          gui << ColorSelect(col[0], col[1], col[2], {.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 2}});
        }else if(std::any_cast<cp::ImageView>(&c)){
          // Read-only image preview.  Op writes the live core::Image
          // into typed_value via setPropertyValueTyped (or the
          // prop(name).value = img proxy); our property-change
          // callback posts a GUI-thread update that pushes the
          // Image into the ImageHandle.  Intentionally NOT added to
          // the callback-exec list — ImageHandle is output-only and
          // rejects GUI::ComplexCallback registrations.
          std::string handle = "#img#"+p.full;
          gui << Display({.handle=handle, .label=p.half, .tooltip=tt, .minSize={12, 8}});
        }else{
          ERROR_LOG("unable to create GUI-component for property \"" << p.full
                    << "\" (no constraint recognised; legacy type=\""
                    << h.type() << "\")");
        }
      }

      bool isSpecialGrabberGrabberProperty(Configurable* c, const std::string &prop){
        if(dynamic_cast<io::SourceBackend*>(conf)){
          const unsigned int propcount = 5;
          const std::string properties[propcount] = {"format", "size", "desired format", "desired size", "desired depth"};
          for (unsigned int i = 0; i < propcount; ++i){
            if(properties[i] == prop) return true;
          }
        }
        return false;
      }

      StSt getStSt(std::map<std::string,std::vector<StSt>, std::less<>> &map, std::string name){
        for(const auto& [section, entries] : map){
          for(unsigned int i=0;i<entries.size();++i){
            if(entries[i].full == name){
              return entries[i];
            }
          }
        }

        ERROR_LOG("Could not find " << name << " property in map.");
        return StSt("error", "error");
      }

      ConfigurableGUIWidget(const Prop &c, const CreateContext &ctx)
        : GUIWidget(c,ctx,GUIWidget::gridLayout, Size(8,12)),
          deactivateExec(false), processingProperty("")
      {
        // The component carries the Configurable* directly (no pointer-in-string
        // smuggling), or a registered string ID.
        conf = c.cfg ? const_cast<Configurable*>(c.cfg) : Configurable::get(c.cfgID);
        if(!conf) throw ICLException("No Configurable with ID "+c.cfgID+" registered");

        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for the Configurable GUI component!");
        }

        buildFromConf();

        // Property-change callback: any thread may fire this.  Coalesce
        // updates in `pendingPropertyUpdates`; schedule at most one
        // GUI-thread flush at a time via QMetaObject::invokeMethod
        // with Qt::QueuedConnection.
        propChangeToken = conf->registerCallback([this](const Configurable::Property &p){
          enqueuePropertyUpdate(p.name);
        });

        // Child-set callback: fires when addChildConfigurable /
        // removeChildConfigurable runs after the Configurable is
        // already wired to this widget.  Trigger a full rebuild so
        // the newly added / removed child's properties appear /
        // disappear live.  Coalesced to one rebuild per event-loop
        // tick.
        childSetToken = conf->onChildSetChanged([this]{ enqueueRebuild(); });
      }

      ~ConfigurableGUIWidget() override {
        // Unregister both callbacks so the Configurable (which may
        // outlive this widget) won't fire into a dangling `this`.
        if(conf){
          conf->removeCallback(propChangeToken);
          conf->removeChildSetCallback(childSetToken);
        }
      }

      // Build the widget tree from conf's current property list.
      // Called from the constructor and on rebuild after a child-set
      // change.  Kept as a member so we can walk the exact property
      // list `conf` exposes *now*, not a snapshot.
      void buildFromConf(){
        std::vector<std::string> props = conf->getPropertyListWithoutDeactivated();
        std::map<std::string,std::vector<StSt>, std::less<>> sections;
        std::map<int,std::string> sections_ordering;

        for(unsigned int i=0;i<props.size();++i){
          const std::string &p = props[i];
          size_t pos = p.find('.');
          std::string name;
          bool doesnt_exist;
          if(pos == std::string::npos){
            name = "general";
            doesnt_exist = !sections.contains(name);
            sections["general"].push_back(StSt(p,p));
          }else{
            name = p.substr(0,pos);
            doesnt_exist = !sections.contains(name);
            sections[name].push_back(StSt(p,p.substr(pos+1))); //
          }
          if(conf->isOrderedFlagSet())
            if(doesnt_exist)sections_ordering[sections.size()] = name;
        }

        std::string tablist;

        int generalIdx = 0;
        int i=0;
        if(!conf->isOrderedFlagSet()) {
          for(const auto& [section, entries] : sections){
            if(section == "general") {
              generalIdx = i;
            }
            tablist += (tablist.length()?",":"")+section;
            ++i;
          }
        } else {
          for(const auto& [order, section] : sections_ordering){
            if(section == "general") {
              generalIdx = i;
            }
            tablist += (tablist.length()?",":"")+section;
            ++i;
          }
        }
        gui = HSplit(this, {.handle="__the_root__"});
        bool use_tabs = sections.size() > 1 || (sections.size() == 1 && sections.begin()->first != "general");
        if(use_tabs){
          if(!sections.contains("general")){
            tablist += ",general";
            generalIdx = tablist.size()-1;
          }
          sub_gui = Tab(tablist, this, {.handle="__the_tab__"});
        } else {
          sub_gui = VBox(this, {.handle="__the_tab__"});
        }

        std::ostringstream ostr;

        // special treatment of grabbers
        if(dynamic_cast<io::SourceBackend*>(conf)){
          GUI general_box = VBox(this, {.handle="__the_box__"});
          add_component(general_box,getStSt(sections, "format"),ostr,gui);
          add_component(general_box,getStSt(sections, "size"),ostr,gui);
          add_component(general_box,getStSt(sections, "desired format"),ostr,gui);
          add_component(general_box,getStSt(sections, "desired size"),ostr,gui);
          add_component(general_box,getStSt(sections, "desired depth"),ostr,gui);
          gui <<  general_box;
        }

        bool haveGeneral = false;
        if(!conf->isOrderedFlagSet()) {
          for(const auto& [section, entries] : sections){
            GUI tab = VScroll();
            for(unsigned int i=0;i<entries.size();++i){
              if(!isSpecialGrabberGrabberProperty(conf,entries[i].full)){
                add_component(tab,entries[i],ostr,gui);
              }
            }
            if(section == "general"){
              haveGeneral = true;
              tab << ( HBox()
                       << Button("load", {.handle="#X#load"})
                       << Button("save", {.handle="#X#save"})
                     );

              ostr <<  '\1' << "#X#load";
              ostr <<  '\1' << "#X#save";
            }
            sub_gui << tab;
          }
        } else {
            for(const auto& [order, section] : sections_ordering){
              std::vector<StSt>& entries = sections[section];
              GUI tab = VScroll();
              for(unsigned int i=0;i<entries.size();++i){
                if(!isSpecialGrabberGrabberProperty(conf,entries[i].full)){
                  add_component(tab,entries[i],ostr,gui);
                }
              }
              if(section == "general"){
                haveGeneral = true;
                tab << ( HBox()
                         << Button("load", {.handle="#X#load"})
                         << Button("save", {.handle="#X#save"})
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
                     << Button("load", {.handle="#X#load"})
                     << Button("save", {.handle="#X#save"})
                     );

            ostr <<  '\1' << "#X#load";
            ostr <<  '\1' << "#X#save";
            sub_gui << tab;
        }

        gui << sub_gui;
        gui.create();

        for(const auto& [handle, value] : deferredAssignList){
          gui[handle] = value;
        }

        if(use_tabs){
          (**gui.get<TabHandle>("__the_tab__")).setCurrentIndex(generalIdx);
        }

        std::string cblist = ostr.str();
        if(cblist.size() > 1){
          gui.registerCallback([this](const std::string &handle) { exec(handle); },cblist.substr(1),'\1');
        }
      }

      /// Tear the widget tree down so buildFromConf() can rebuild it.
      /// Runs on the GUI thread.  Keeps the ConfigurableGUIWidget itself
      /// intact so external handles pointing to *this* stay valid.
      void clearWidgets(){
        // Drop all Qt children of this GUIWidget — everything built by
        // buildFromConf() is parented under here through `gui = HSplit(this)`.
        QLayout *l = this->layout();
        if(l){
          while(QLayoutItem *item = l->takeAt(0)){
            if(QWidget *w = item->widget()){
              w->setParent(nullptr);
              w->deleteLater();
            }
            delete item;
          }
        }
        // Reset GUI state so re-creation doesn't duplicate handles.
        gui = GUI();
        sub_gui = GUI();
        // Pending property updates may reference properties that no
        // longer exist — drop them; fresh subscriptions will be set
        // up by the rebuild.
        std::scoped_lock lk(pendingMutex);
        pendingPropertyUpdates.clear();
        updateScheduled = false;
      }

      /// Runs on the GUI thread (scheduled via invokeMethod).  Fully
      /// rebuilds the widget tree from conf's current property list.
      void rebuild(){
        clearWidgets();
        buildFromConf();
      }

      /// Coalesced scheduler for rebuild().  Safe to call from any
      /// thread — add/removeChildConfigurable may fire from inside a
      /// Configurable property-change callback on a worker thread.
      void enqueueRebuild(){
        bool needSchedule = false;
        {
          std::scoped_lock lk(pendingMutex);
          if(!rebuildScheduled){
            rebuildScheduled = true;
            needSchedule = true;
          }
        }
        if(needSchedule){
          QMetaObject::invokeMethod(this, [this]{
            {
              std::scoped_lock lk(pendingMutex);
              rebuildScheduled = false;
            }
            rebuild();
          }, Qt::QueuedConnection);
        }
      }

      /// Add `name` to the pending-update set and schedule a single
      /// GUI-thread flush if not already scheduled.  Thread-safe: may
      /// be called from any thread firing a Configurable callback.
      void enqueuePropertyUpdate(const std::string &name){
        bool needSchedule = false;
        {
          std::scoped_lock lk(pendingMutex);
          pendingPropertyUpdates.insert(name);
          if(!updateScheduled){
            updateScheduled = true;
            needSchedule = true;
          }
        }
        if(needSchedule){
          QMetaObject::invokeMethod(this, [this]{ flushPendingPropertyUpdates(); },
                                    Qt::QueuedConnection);
        }
      }

      /// Runs on the GUI thread (scheduled via invokeMethod).  Reads the
      /// *current* value of each pending property and pushes it into the
      /// matching widget — multiple rapid writes coalesce to one widget
      /// update per property per GUI tick.
      void flushPendingPropertyUpdates(){
        std::set<std::string> batch;
        {
          std::scoped_lock lk(pendingMutex);
          batch.swap(pendingPropertyUpdates);
          updateScheduled = false;
        }
        for(const std::string &name : batch){
          try{
            applyPropertyToWidget(conf->prop(name));
          }catch(const ICLException &){
            // Property vanished between the enqueue and the flush
            // (e.g. child configurable removed) — skip silently.
          }
        }
      }

      /// Updates the matching widget for a single property.  Runs on
      /// GUI thread.  Dispatch mirrors update_all_components /
      /// add_component's handle-prefix scheme.
      void applyPropertyToWidget(Configurable::Handle h){
        namespace up = utils::prop;
        namespace cp = core::prop;
        const std::string &name = h.name;
        const std::any    &c    = h.constraint;
        deactivateExec = true;
        processingProperty = name;

        if(auto *r = std::any_cast<up::Range<float>>(&c)){
          if(r->ui == up::UI::Spinbox){
            gui["#F#"+name] = h.as<float>();
          }else{
            gui["#r#"+name] = h.as<float>();
          }
        }else if(auto *r = std::any_cast<up::Range<int>>(&c)){
          if(r->ui == up::UI::Spinbox){
            gui["#R#"+name] = h.as<int>();
          }else{
            // Snap the incoming value to the constraint's stepping so a
            // slider with step=N reflects the model's N-aligned grid.
            int val = h.as<int>();
            int step = (r->step == 0) ? 1 : r->step;
            val = (val/step)*step;
            gui["#r#"+name] = val;
          }
        }else if(std::any_cast<up::Menu<std::string>>(&c) ||
                 std::any_cast<up::Menu<int>>(&c) ||
                 std::any_cast<up::Menu<float>>(&c)){
          gui["#m#"+name] = h.as<std::string>();
        }else if(std::any_cast<up::Flag>(&c)){
          gui["#f#"+name] = h.as<bool>();
        }else if(std::any_cast<up::Info>(&c)){
          gui["#i#"+name] = h.as<std::string>();
        }else if(std::any_cast<up::Text>(&c)){
          gui["#S#"+name] = h.as<std::string>();
        }else if(std::any_cast<cp::Color>(&c)){
          gui["#C#"+name] = h.as<Color>();
        }else if(std::any_cast<cp::ImageView>(&c)){
          // Op wrote a core::Image into typed_value.  Push it into the
          // Display / ImageHandle.  Inspect the any's type_info rather
          // than going through AutoParse — avoids the string cascade
          // and silently skips if the writer stored something that
          // isn't a core::Image.
          if(h.typed_value.type() == typeid(core::Image)){
            core::Image img = std::any_cast<core::Image>(h.typed_value);
            if(!img.isNull()){
              auto &imgHandle = gui.get<ImageHandle>("#img#"+name);
              imgHandle = img;
              imgHandle.render();
            }
          }
        }
        // up::Command — nothing to push.

        deactivateExec = false;
        processingProperty = "";
      }

      void exec(const std::string &handle){
        // exec() runs on the GUI thread (Qt callback dispatch).
        // flushPendingPropertyUpdates — the only other mutator of
        // deactivateExec / processingProperty — also runs on the GUI
        // thread (posted via QMetaObject::invokeMethod), so the two
        // serialize on the event loop without needing an explicit
        // mutex.
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
            // `Any = handle` is not an enrolled AssignRegistry rule;
            // every handle with a string surface exposes `as<std::string>()`,
            // and `Any` is implicitly constructible from `std::string`
            // (it publicly inherits `std::string`), so the string path
            // reaches `setPropertyValue(const Any &)` via one cheap copy.
            conf->prop(prop).value = gui[handle].as<std::string>();
            break;
          case 'C':
            conf->prop(prop).value = gui[handle].as<Color>();
            break;
          case 'c':
            conf->prop(prop).value = "";
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

      static std::string getSyntax(){
        return std::string("prop(ConfigurableID)[general params]\n")+gen_params();
      }

    };

    struct CamPropertyWidget : public Tab {

      static std::string create_tab_list(){
         const std::vector<io::DeviceDescription> devs =
                            io::ImageSource::getDeviceList("",false);
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
        std::vector<io::DeviceDescription> devs = io::ImageSource::getDeviceList("",false);
        for(unsigned int i = 0; i < devs.size(); ++i){
          *this << Prop(devs.at(i).name(), {.label=devs.at(i).name()});
        }
        *this << Create();
      }
    };


    struct CamCfgGUIWidget : public GUIWidget {

      CamCfgGUIWidget(const CamCfg &c, const CreateContext &ctx):
        GUIWidget(c,ctx), m_cfg(nullptr), m_button(nullptr)
      {
        const bool hasDevice = !c.opts.deviceType.empty();
        if(!hasDevice){
          m_button = new QPushButton("camcfg",this);
          connect(m_button,SIGNAL(clicked()),this,SLOT(ioSlot()));
          addToGrid(m_button);
        } else {
          if(!m_cfg) m_cfg = new CamPropertyWidget();
          addToGrid(m_cfg -> getRootWidget());
        }

        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for the Camera Configuration GUI component!");
        }
      }

      virtual void processIO(){
          if(!m_cfg) m_cfg = new CamPropertyWidget();
          m_cfg->show();
      }

      static std::string getSyntax(){
        return std::string("camcfg()[general params]\n")+gen_params();
      }

      CamPropertyWidget *m_cfg;
      QPushButton *m_button;
    };

    struct ScrollGUIWidgetBase : public GUIWidget, public ProxyLayout{

      ScrollGUIWidgetBase(const ContainerComponent &c, const CreateContext &ctx, QBoxLayout::Direction d):
        GUIWidget(c,ctx,GUIWidget::noLayout){
        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }
        const int margin  = c.options().margin  > 0 ? c.options().margin  : 2;
        const int spacing = c.options().spacing > 0 ? c.options().spacing : 2;
        setLayout(new QBoxLayout(d,this));
        m_poScroll = new QScrollArea(this);
        layout()->addWidget(m_poScroll);
        layout()->setContentsMargins(0,0,0,0);
        m_poScroll->setWidget(new QWidget(m_poScroll));
        m_poScroll->setWidgetResizable(true);

        m_poScroll->widget()->setLayout(new QBoxLayout(d,m_poScroll));
        m_poScroll->widget()->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));

        m_poScroll->widget()->layout()->setContentsMargins(margin,margin,margin,margin);
        m_poScroll->widget()->layout()->setSpacing(spacing);

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<BoxHandle>(c.options().handle,BoxHandle(d != QBoxLayout::LeftToRight,m_poScroll->widget(),this,m_poScroll));
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


    struct HBoxGUIWidget : public GUIWidget{
      HBoxGUIWidget(const ContainerComponent &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::hboxLayout){
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<BoxHandle>(c.options().handle,BoxHandle(true,this,this));
          getGUI()->unlockData();
        }
      }
    };



    struct VBoxGUIWidget : public GUIWidget{
      VBoxGUIWidget(const ContainerComponent &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::vboxLayout){
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<BoxHandle>(c.options().handle,BoxHandle(false,this,this));
          getGUI()->unlockData();
        }
      }
    };


    struct SplitterGUIWidgetBase : public GUIWidget, public ProxyLayout{
      SplitterGUIWidgetBase(const ContainerComponent &c, const CreateContext &ctx, bool horz):GUIWidget(c,ctx,GUIWidget::noLayout){
        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        m_layout = new QGridLayout(this);
        m_splitter = new QSplitter(horz ? Qt::Horizontal:Qt::Vertical , this);
        m_layout->addWidget(m_splitter,0,0);
        m_layout->setContentsMargins(0,0,0,0);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<SplitterHandle>(c.options().handle,SplitterHandle(m_splitter,this));
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

      QSplitter *m_splitter;
      QGridLayout *m_layout;
    };

    struct TabGUIWidget : public GUIWidget, public ProxyLayout{
      TabGUIWidget(const ContainerComponent &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::noLayout){
        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }

        m_layout = new QGridLayout(this);
        m_tabWidget = new QTabWidget(this);
        m_layout->addWidget(m_tabWidget,0,0);

        m_layout->setContentsMargins(0,0,0,0);
        m_nextTabIdx = 0;
        m_tabNames = tok(c.param, ",");

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<TabHandle>(c.options().handle,TabHandle(m_tabWidget,this));
          getGUI()->unlockData();
        }
      }

      // implements the ProxyLayout interface, what should be done if components are added
      // using the GUI-stream operator <<
      virtual void addWidget(GUIWidget *widget){
        QString tabName;
        if(m_nextTabIdx < static_cast<int>(m_tabNames.size())){
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

      static std::string getSyntax(){
        return std::string("tab(COMMA_SEPERATED_TAB_LIST)[general params]\n")+gen_params();
      }

      std::vector<std::string> m_tabNames;
      QTabWidget *m_tabWidget;
      int m_nextTabIdx;
      QGridLayout *m_layout;
    };

    struct BorderGUIWidget : public GUIWidget{

      BorderGUIWidget(const ContainerComponent &c, const CreateContext &ctx):GUIWidget(c,ctx){

        if(!c.options().tooltip.empty()){
          WARNING_LOG("tooltip is not supported for Layouting GUI components!");
        }
        const int margin  = c.options().margin  > 0 ? c.options().margin  : 2;
        const int spacing = c.options().spacing > 0 ? c.options().spacing : 2;

        m_poGroupBox = new QGroupBox((c.param + "  ").c_str(),ctx.parentWidget);
        m_poLayout = new QVBoxLayout;
        m_poLayout->setContentsMargins(margin,margin,margin,margin);
        m_poLayout->setSpacing(spacing);
        m_poGroupBox->setLayout(m_poLayout);
        addToGrid(m_poGroupBox);
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<BorderHandle>(c.options().handle,BorderHandle(m_poGroupBox,this));
          getGUI()->unlockData();
        }

      }
      virtual QLayout *getGUIWidgetLayout() { return m_poLayout; }
      private:
      QGroupBox *m_poGroupBox;
      QVBoxLayout *m_poLayout;
    };



    struct ColorGUIWidget : public GUIWidget{
      ColorGUIWidget(const ColorSelect &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(6,2)){
        QPushButton *b = new QPushButton("select",ctx.parentWidget);
        addToGrid(b);
        connect(b,SIGNAL(clicked()),this,SLOT(ioSlot()));

        m_haveAlpha = (c.opts.alpha >= 0);

        m_color = Color4D(c.r, c.g, c.b, m_haveAlpha ? c.opts.alpha : 0);

        colorLabel = new ColorLabel(m_color,m_haveAlpha,ctx.parentWidget);

        if(!c.options().tooltip.empty()) colorLabel->setToolTip(c.options().tooltip.c_str());

        addToGrid(colorLabel,1,0,1,1);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          handle = &getGUI()->allocValue<ColorHandle>(c.options().handle,ColorHandle(colorLabel,this));
          getGUI()->unlockData();
        }else{
          handle = 0;
        }
      }

      virtual void processIO(){
        QColor c = !m_haveAlpha ?
                   QColor(m_color[0],m_color[1],m_color[2]) :
                   QColor(m_color[0],m_color[1],m_color[2],m_color[3]);

        QColor color = QColorDialog::getColor(c,this,"choose color...",
                                              m_haveAlpha ? QColorDialog::ShowAlphaChannel :
                                              static_cast<QColorDialog::ColorDialogOption>(0));

        colorLabel->setColor(Color4D(color.red(),color.green(),color.blue(),color.alpha()));
      }

      static std::string getSyntax(){
        return std::string("color(R,G,B[,A])[general params] \n")+
        std::string("\tgiven initial Red, Green and Blue values (Alpha is optional)\n")+
        gen_params();
      }
      ColorLabel *colorLabel;
      ColorHandle *handle;
      Color4D m_color;
      bool m_haveAlpha;
    };

    class ButtonGUIWidget : public GUIWidget{
    public:
      ButtonGUIWidget(const Button &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(4,1)){
        QPushButton *b = new QPushButton(c.text.c_str(),ctx.parentWidget);

        if(!c.options().tooltip.empty()) b->setToolTip(c.options().tooltip.c_str());

        addToGrid(b);
        connect(b,SIGNAL(pressed()),this,SLOT(ioSlot()));

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          m_poClickedEvent = &getGUI()->allocValue<ButtonHandle>(c.options().handle,ButtonHandle(b,this));
          getGUI()->unlockData();
        }else{
          m_poClickedEvent = 0;
        }
      }
      virtual void processIO(){
        if(m_poClickedEvent){
          m_poClickedEvent->trigger(false);
        }
      }
    private:
      ButtonHandle *m_poClickedEvent;
    };

    struct ButtonGroupGUIWidget : public GUIWidget{

      ButtonGroupGUIWidget(const ButtonGroup &c, const CreateContext &ctx):
        GUIWidget(c,ctx,GUIWidget::gridLayout,Size(4,1)), m_uiInitialIndex(0){

        std::vector<std::string> entries = tok(c.entries, ",");
        for(unsigned int i=0;i<entries.size();i++){
          std::string text = entries[i];
          if(text.length() && text[0]=='!'){
            m_uiInitialIndex = i;
            text = text.substr(1);
          }
          QRadioButton * b = new QRadioButton(text.c_str(),ctx.parentWidget);
          if(!c.options().tooltip.empty()) b->setToolTip(c.options().tooltip.c_str());

          m_vecButtons.push_back(b);
          addToGrid(b,0,i);
        }

        m_vecButtons[m_uiInitialIndex]->setChecked(true);

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        // Allocate the handle before wiring up ioSlot: ButtonGroupHandle
        // installs a per-button `toggled(bool)` lambda that updates its
        // cached selected index.  Qt's default per-object connection
        // order = connection order, so the lambda must be installed
        // first to fire before user callbacks.
        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<ButtonGroupHandle>(c.options().handle,ButtonGroupHandle(&m_vecButtons,this));
          getGUI()->unlockData();
        }

        for(QRadioButton *b : m_vecButtons){
          connect(b,SIGNAL(clicked()),this,SLOT(ioSlot()));
        }
      }
      virtual void processIO(){}
      //    virtual Size getDefaultSize() {
      //  return Size(4,m_vecButtons.size());
      //}
    private:
      std::vector<QRadioButton*> m_vecButtons;
      unsigned int m_uiInitialIndex ;
    };

    class ToggleButtonGUIWidget : public GUIWidget{
    public:
      ToggleButtonGUIWidget(const GUIComponent &comp, const CreateContext &ctx,
                            const std::string &u, const std::string &t, bool initToggled):
        GUIWidget(comp,ctx){

        m_poButton = new ToggleButtonWidget(u,t,ctx.parentWidget);
        if(!comp.options().tooltip.empty()) m_poButton->setToolTip(comp.options().tooltip.c_str());

        if(initToggled){
          m_poButton->setChecked(true);
        }

        addToGrid(m_poButton);

        // this must be connected to the toggled function too (not to the clicked() signal) because
        // the clicked()-signal is emitted BEFORE the toggled-signale, which makes the button get
        // out of sync-with it's underlying value :-(
        connect(m_poButton,SIGNAL(toggled(bool)),this,SLOT(ioSlot()));

        if(!comp.options().handle.empty()){
          getGUI()->lockData();
          m_poHandle = &getGUI()->allocValue<ButtonHandle>(comp.options().handle,ButtonHandle(m_poButton,this));
          getGUI()->unlockData();
        }else{
          m_poHandle = 0;
        }
      }
      virtual void processIO(){
        if(m_poHandle){
          m_poHandle->trigger(false);
        }
      }
    private:
      ToggleButtonWidget *m_poButton;
      ButtonHandle *m_poHandle;
    };



    struct CheckBoxGUIWidget : public GUIWidget{
    public:
      CheckBoxGUIWidget(const CheckBox &c, const CreateContext &ctx):
        GUIWidget(c,ctx){

        m_poCheckBox = new QCheckBox(c.text.c_str(),ctx.parentWidget);
        m_poCheckBox->setTristate(false);
        if(!c.options().tooltip.empty()) m_poCheckBox->setToolTip(c.options().tooltip.c_str());

        m_poCheckBox->setCheckState(c.opts.checked ? Qt::Checked : Qt::Unchecked);

        addToGrid(m_poCheckBox);

        // Handle first so CheckBoxHandle's stateChanged lambda runs
        // before the user-callback-dispatching ioSlot.  See
        // ComboGUIWidget for the full ordering rationale.
        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<CheckBoxHandle>(c.options().handle,CheckBoxHandle(m_poCheckBox,this));
          getGUI()->unlockData();
        }

        // this must be connected to the toggled function too (not to the clicked() signal) because
        // the clicked()-signal is emitted BEFORE the toggled-signale, which makes the button get
        // out of sync-with it's underlying value :-(
        connect(m_poCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ioSlot()));
      }
      virtual void processIO(){}
    private:
      QCheckBox *m_poCheckBox;
    };





    struct LabelGUIWidget : public GUIWidget{
      LabelGUIWidget(const Label &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(4,1)){

        m_poLabel = new CompabilityLabel(c.text.c_str(),ctx.parentWidget);
        if(!c.options().tooltip.empty()) m_poLabel->setToolTip(c.options().tooltip.c_str());

        addToGrid(m_poLabel);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<LabelHandle>(c.options().handle,LabelHandle(m_poLabel,this));
          getGUI()->unlockData();
        }
      }
    private:
      CompabilityLabel *m_poLabel;
    };




    /// Status bar: a thin horizontal strip docked to the bottom of its parent.
    /** Regardless of the parent's layout direction, the bar pins itself to the
        bottom edge (see dockToBottom).  It is capped to STATUSBAR_HEIGHT pixels
        and always exposes an initial, left-aligned text label under the fixed
        handle "status".  Components streamed into the bar are packed to the
        right of that label. */
    struct StatusBarGUIWidget : public GUIWidget{
      static const int STATUSBAR_HEIGHT = 24;

      StatusBarGUIWidget(const ContainerComponent &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::hboxLayout){
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
        setFixedHeight(STATUSBAR_HEIGHT);

        QHBoxLayout *box = static_cast<QHBoxLayout*>(layout());
        // keep an l/r (and small bottom) margin so children aren't clipped
        // by the host window's rounded bottom corners (e.g. macOS)
        box->setContentsMargins(12,0,12,2);
        box->setSpacing(6);

        // initial left-aligned status label, reachable as gui["status"].
        // CompabilityLabel has no sizeHint, so without an expanding policy it
        // collapses to ~0 width in the hbox and the text is clipped. Making it
        // expand also doubles as the spacer that packs later items to the right.
        m_label = new CompabilityLabel("",this);
        m_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        m_label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
        box->addWidget(m_label,1);

        getGUI()->lockData();
        getGUI()->allocValue<LabelHandle>("status",LabelHandle(m_label,this));
        if(!c.options().handle.empty())
          getGUI()->allocValue<BoxHandle>(c.options().handle,BoxHandle(true,this,this));
        getGUI()->unlockData();

        dockToBottom(ctx.parentWidget);
      }

      /// Restructure the parent so this bar sits at its bottom edge.
      /** For a vertical-box parent the base ctor already appended the bar last,
          so it is the bottom row — nothing to do.  For any other layout the
          parent's existing content is moved into a holder widget and stacked
          above the bar inside a fresh QVBoxLayout.  This rewrites the parent's
          layout, so a StatusBar must be the last component of its container. */
      void dockToBottom(QWidget *parent){
        if(!parent) return;
        QLayout *old = parent->layout();
        if(!old || qobject_cast<QVBoxLayout*>(old)) return;

        QBoxLayout *ob = qobject_cast<QBoxLayout*>(old);
        QWidget *content = new QWidget(parent);
        QBoxLayout *cl = new QBoxLayout(ob ? ob->direction() : QBoxLayout::LeftToRight);
        cl->setContentsMargins(0,0,0,0);
        if(ob) cl->setSpacing(ob->spacing());

        std::vector<QWidget*> kids;
        for(int i=0;i<old->count();++i)
          if(QWidget *w = old->itemAt(i)->widget())
            if(w != this) kids.push_back(w);
        for(QWidget *w : kids) cl->addWidget(w);   // reparents into content
        content->setLayout(cl);

        old->removeWidget(this);
        delete old;                                // detach (now empty) from parent
        QVBoxLayout *outer = new QVBoxLayout(parent);
        outer->setContentsMargins(0,0,0,0);
        outer->setSpacing(0);
        outer->addWidget(content,1);
        outer->addWidget(this,0);
      }

      static std::string getSyntax(){
        return std::string("statusbar()[general params]\n")+gen_params();
      }
    private:
      CompabilityLabel *m_label;
    };


    struct StateGUIWidget : public GUIWidget{
      StateGUIWidget(const State &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(4,1)){
        m_text = new ThreadedUpdatableTextView(ctx.parentWidget);
        m_text->setReadOnly(true);
        if(!c.options().tooltip.empty()) m_text->setToolTip(c.options().tooltip.c_str());

        addToGrid(m_text);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<StateHandle>(c.options().handle,StateHandle(m_text,this,c.maxLines));
          getGUI()->unlockData();
        }
      }
    private:
      ThreadedUpdatableTextView *m_text;
    };



    struct SliderGUIWidget : public GUIWidget{

      SliderGUIWidget(const Slider &s, const CreateContext &ctx)
        : GUIWidget(s, ctx, GUIWidget::gridLayout, s.opts.vertical?Size(1,4):Size(4,1)){
        m_stepping = s.opts.step;
        if(m_stepping < 1){
          ERROR_LOG("a slider gui component with stepping < 1 is not possible (using stepping 1)");
          m_stepping = 1;
        }

        const bool vert = s.opts.vertical;
        const int iMin = s.min, iMax = s.max, iCurr = s.val;

        m_poSlider = new ThreadedUpdatableSlider(vert?Qt::Vertical:Qt::Horizontal, ctx.parentWidget);
        m_poSlider->setStepping(m_stepping);

        if(!s.options().tooltip.empty()) m_poSlider->setToolTip(s.options().tooltip.c_str());

        addToGrid(m_poSlider);

        m_poSlider->setMinimum(iMin);
        m_poSlider->setMaximum(iMax);
        m_poSlider->setValue(iCurr);
        if(m_stepping != 1){
          m_poSlider->setSingleStep(m_stepping);
          m_poSlider->setTickInterval(m_stepping);
        }
        {
          int nDigits = iclMax(QString::number(iMin).length(),QString::number(iMax).length());
          m_poLCD = new QLCDNumber(nDigits, ctx.parentWidget);
          m_poLCD->display(iCurr);
          if(vert) addToGrid(m_poLCD,0,1,1,4);
          else     addToGrid(m_poLCD,1,0,4,1);
        }

        connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));

        m_bVerticalFlag = vert;

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(!s.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<SliderHandle>(s.options().handle,SliderHandle(m_poSlider,this,m_poLCD));
          getGUI()->unlockData();
        }
      }
      virtual void processIO(){
        //cb();
        //iStep is handled as a value that must '%' the slider to 0
        int value = m_poSlider->value();
        value = (value / m_stepping) * m_stepping;
        if(m_poLCD) m_poLCD->display(value);
      }
    private:
      ThreadedUpdatableSlider *m_poSlider;
      QLCDNumber *m_poLCD;
      bool m_bVerticalFlag;
      int m_stepping;

    };

    struct FloatSliderGUIWidget : public GUIWidget{

      FloatSliderGUIWidget(const FSlider &s, const CreateContext &ctx)
        : GUIWidget(s, ctx, GUIWidget::gridLayout, s.opts.vertical?Size(1,4):Size(4,1)){
        const bool vert = s.opts.vertical;
        m_fMinVal = s.min;
        m_fMaxVal = s.max;
        float fCurr = s.val;
        int nDigits = 6;

        m_fM = (m_fMaxVal-m_fMinVal)/10000.0;
        m_fB = m_fMinVal;

        m_poSlider = new ThreadedUpdatableSlider(vert?Qt::Vertical:Qt::Horizontal, ctx.parentWidget);
        if(!s.options().tooltip.empty()) m_poSlider->setToolTip(s.options().tooltip.c_str());

        addToGrid(m_poSlider);

        m_poSlider->setMinimum(0);
        m_poSlider->setMaximum(10000);
        m_poSlider->setValue(f2i(fCurr));

        m_poLCD = new QLCDNumber(nDigits, ctx.parentWidget);
        m_poLCD->display(fCurr);
        if(vert) addToGrid(m_poLCD,0,1,1,4);
        else     addToGrid(m_poLCD,1,0,4,1);

        connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

        if(!s.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<FSliderHandle>(s.options().handle,FSliderHandle(m_poSlider,&m_fMinVal,&m_fMaxVal,&m_fM,&m_fB,10000,this,m_poLCD));
          getGUI()->unlockData();
        }
      }
      virtual void processIO(){
        if(m_poLCD){
          m_poLCD->display(i2f(m_poSlider->value()));
        }
      }
    private:
      ThreadedUpdatableSlider *m_poSlider;
      QLCDNumber *m_poLCD;
      float m_fM,m_fB;
      float m_fMinVal, m_fMaxVal;
      int f2i(float f){
        return static_cast<int>((f-m_fB)/m_fM);
      }
      float i2f(int i){
        return m_fM*i+m_fB;
      }
    };

    struct IntGUIWidget : public GUIWidget{
  public:
      IntGUIWidget(const Int &c, const CreateContext &ctx):GUIWidget(c,ctx){
        m_poLineEdit = new QLineEdit(ctx.parentWidget);
        m_poLineEdit->setValidator(new QIntValidator(c.min,c.max,0));
        m_poLineEdit->setText(QString::number(c.val));

        if(!c.options().tooltip.empty()) m_poLineEdit->setToolTip(c.options().tooltip.c_str());

        QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));

        addToGrid(m_poLineEdit);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<IntHandle>(c.options().handle,IntHandle(m_poLineEdit,this));
          getGUI()->unlockData();
        }
      }
      virtual void processIO(){}
    private:
      QLineEdit *m_poLineEdit;
    };

    struct FloatGUIWidget : public GUIWidget{
  public:
      FloatGUIWidget(const Float &c, const CreateContext &ctx):GUIWidget(c,ctx){
        m_poLineEdit = new QLineEdit(ctx.parentWidget);
        m_poLineEdit->setValidator(new QDoubleValidator(c.min,c.max,20,0));
        m_poLineEdit->setText(QString::number(c.val));

        if(!c.options().tooltip.empty()) m_poLineEdit->setToolTip(c.options().tooltip.c_str());

        QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));

        addToGrid(m_poLineEdit);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<FloatHandle>(c.options().handle,FloatHandle(m_poLineEdit,this));
          getGUI()->unlockData();
        }

      }
      virtual void processIO(){}
    private:
      QLineEdit *m_poLineEdit;
    };

    struct StringGUIWidget : public GUIWidget{
      class StringLenValidator : public QValidator{
      public:
        StringLenValidator(int iMaxLen):QValidator(0){
          this->iMaxLen = iMaxLen;
        }
        virtual State validate(QString &sInput,int &iPos)const{
          static_cast<void>(iPos);
          return sInput.length()>iMaxLen ?  Invalid : Acceptable;
        }
      private:
        int iMaxLen;
      };

      StringGUIWidget(const String &c, const CreateContext &ctx):GUIWidget(c,ctx){
        m_poLineEdit = new QLineEdit(ctx.parentWidget);
        m_poLineEdit->setValidator(new StringLenValidator(c.opts.maxLen));
        m_poLineEdit->setText(c.text.c_str());

        if(!c.options().tooltip.empty()) m_poLineEdit->setToolTip(c.options().tooltip.c_str());

        QObject::connect(m_poLineEdit,SIGNAL(returnPressed ()),this,SLOT(ioSlot()));

        addToGrid(m_poLineEdit);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<StringHandle>(c.options().handle,StringHandle(m_poLineEdit,this));
          getGUI()->unlockData();
        }

      }
      virtual void processIO(){}
    private:
      QLineEdit *m_poLineEdit;
    };

    struct DispGUIWidget : public GUIWidget{

      DispGUIWidget(const Disp &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(c.nx*2,c.ny)){
        if(c.nx < 1 || c.ny < 1) throw ICLException("Disp: nx and ny must be > 0");
        int nW = c.nx, nH = c.ny;

        m_poLabelMatrix = new LabelMatrix(nW,nH);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<DispHandle>(c.options().handle,DispHandle(m_poLabelMatrix,this));
          getGUI()->unlockData();
        }

        for(int x=0;x<nW;x++){
          for(int y=0;y<nH;y++){
            CompabilityLabel *l = new CompabilityLabel("",ctx.parentWidget);
            if(!c.options().tooltip.empty()) l->setToolTip(c.options().tooltip.c_str());

            (*m_poLabelMatrix)(x,y) = LabelHandle(l,this);
            addToGrid(l,x,y);
          }
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }

    private:
      LabelMatrix *m_poLabelMatrix;
    };

    struct ImageGUIWidget : public GUIWidget{
      ImageGUIWidget(const Display &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(16,12)){

        m_poWidget = new ICLWidget(ctx.parentWidget);
        if(!c.options().tooltip.empty()) m_poWidget->setInfoText(c.options().tooltip);

        addToGrid(m_poWidget);


        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<ImageHandle>(c.options().handle,ImageHandle(m_poWidget,this));
          getGUI()->unlockData();
        }

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
    private:
      ICLWidget *m_poWidget;
    };


    struct PlotGUIWidget : public GUIWidget{
      PlotGUIWidget(const Plot &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(16,12)){

        m_plot = new PlotWidget(ctx.parentWidget);

        if(!c.options().tooltip.empty()) m_plot->setToolTip(c.options().tooltip.c_str());

        const std::string &xAxisLabel = c.opts.xLabel;
        const std::string &yAxisLabel = c.opts.yLabel;

        if(xAxisLabel.length() && xAxisLabel != "-"){
          m_plot->prop("labels.x-axis").value = xAxisLabel;
          m_plot->prop("borders.bottom").value = 55;
        }
        if(yAxisLabel.length() && yAxisLabel != "-"){
          m_plot->prop("labels.y-axis").value = yAxisLabel;
          m_plot->prop("borders.left").value = 55;
        }

        m_plot->setDataViewPort(Range32f(c.opts.minX,c.opts.maxX), Range32f(c.opts.minY,c.opts.maxY));

        if(c.opts.openGL){
          QOpenGLWidget *gl = new QOpenGLWidget(ctx.parentWidget);
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

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<PlotHandle>(c.options().handle,PlotHandle(m_plot,this));
          getGUI()->unlockData();
        }

        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
    private:
      PlotWidget *m_plot;
    };


    struct DrawGUIWidget : public GUIWidget{
      DrawGUIWidget(const Canvas &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(16,12)){
        m_poWidget = new ICLDrawWidget(ctx.parentWidget);
        if(!c.options().tooltip.empty()) m_poWidget->setInfoText(c.options().tooltip);

        m_poWidget->setViewPort(c.viewport);
        addToGrid(m_poWidget);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<DrawHandle>(c.options().handle,DrawHandle(m_poWidget,this));
          getGUI()->unlockData();
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
    private:
      ICLDrawWidget *m_poWidget;
    };




  #ifdef ICL_HAVE_OPENGL
    struct DrawGUIWidget3D : public GUIWidget{
      DrawGUIWidget3D(const Canvas3D &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(16,12)){
        m_poWidget3D = new ICLDrawWidget3D(ctx.parentWidget);
        m_poWidget3D->setViewPort(c.viewport);

        if(!c.options().tooltip.empty()) m_poWidget3D->setInfoText(c.options().tooltip);

        addToGrid(m_poWidget3D);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<DrawHandle3D>(c.options().handle,DrawHandle3D(m_poWidget3D,this));
          getGUI()->unlockData();
        }
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
      }
    private:
      ICLDrawWidget3D *m_poWidget3D;
    };
  #endif

    struct ComboGUIWidget : public GUIWidget{
      ComboGUIWidget(const Combo &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(4,1)){
        m_poCombo = new QComboBox(ctx.parentWidget);

        if(!c.options().tooltip.empty()) m_poCombo->setToolTip(c.options().tooltip.c_str());

        addToGrid(m_poCombo);

        std::vector<std::string> entries = tok(c.entries, ",");
        int selectedIndex = c.opts.initialIndex;
        for(unsigned int i=0;i<entries.size();i++){
          std::string s = entries[i];
          if(s.length() && s[0]=='!'){       // explicit per-entry selection marker
            s = s.substr(1);
            selectedIndex = i;
          }
          m_poCombo->addItem(s.c_str());
        }
        if(selectedIndex < 0 || selectedIndex >= static_cast<int>(entries.size())) selectedIndex = 0;
        m_poCombo->setCurrentIndex(selectedIndex);

        // Order matters: ComboHandle's ctor installs a Qt connection
        // that updates the handle's cached index/text on every
        // `currentIndexChanged(int)` firing.  The user-facing
        // `ioSlot()` hook (which runs registered callbacks) must be
        // connected AFTER that, so Qt fires the cache update first
        // and user callbacks see the new value via `handle.getSelectedIndex()`.
        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<ComboHandle>(c.options().handle,ComboHandle(m_poCombo,this));
          getGUI()->unlockData();
        }

        connect(m_poCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(ioSlot()));
      }
      virtual void processIO(){}

    private:
      QComboBox *m_poCombo;
    };

    struct SpinnerGUIWidget : public GUIWidget{
  public:
      SpinnerGUIWidget(const Spinner &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(4,1)){
        m_poSpinBox = new QSpinBox(ctx.parentWidget);
        m_poSpinBox->setRange(c.min,c.max);
        m_poSpinBox->setValue(c.val);

        if(!c.options().tooltip.empty()) m_poSpinBox->setToolTip(c.options().tooltip.c_str());

        addToGrid(m_poSpinBox);

        // Handle first, then ioSlot wiring — so the handle's
        // valueChanged cache-update lambda runs before user callbacks
        // registered via the handle.  See ComboGUIWidget for the
        // same ordering rationale.
        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<SpinnerHandle>(c.options().handle,SpinnerHandle(m_poSpinBox,this));
          getGUI()->unlockData();
        }

        QObject::connect(m_poSpinBox,SIGNAL(valueChanged(int)),this,SLOT(ioSlot()));

      }
      virtual void processIO(){}
    private:
      QSpinBox *m_poSpinBox;
    };


    struct FPSGUIWidget : public GUIWidget{
      FPSGUIWidget(const Fps &c, const CreateContext &ctx):GUIWidget(c,ctx,GUIWidget::gridLayout,Size(5,2)){
        m_poLabel = new CompabilityLabel("fps...",ctx.parentWidget);
        if(!c.options().tooltip.empty()) m_poLabel->setToolTip(c.options().tooltip.c_str());


        addToGrid(m_poLabel);

        if(!c.options().handle.empty()){
          getGUI()->lockData();
          getGUI()->allocValue<FPSHandle>(c.options().handle,FPSHandle(c.timeWindow,m_poLabel,this));
          getGUI()->unlockData();
        }
      }
      virtual Size getDefaultSize() {
        return Size(4,1);
      }
    private:
      CompabilityLabel *m_poLabel;
    };


    // Base default: a component without an own createWidget() override has no
    // widget (only the magic finalizers Show/Create/Dummy hit this, and they
    // are intercepted by operator<< before create() — so this never fires in
    // practice). Every real component overrides createWidget().
    GUIWidget *GUIComponent::createWidget(const CreateContext &) const {
      throw ICLException("GUIComponent::createWidget: component type '" + m_type +
                         "' has no widget factory");
    }

    // ---- per-component typed createWidget() overrides ---------------------
    // Each builds its own *GUIWidget directly from the component's typed fields.

    GUIWidget *Slider::createWidget(const CreateContext &ctx) const {
      return new SliderGUIWidget(*this, ctx);
    }
    GUIWidget *FSlider::createWidget(const CreateContext &ctx) const {
      return new FloatSliderGUIWidget(*this, ctx);
    }
    GUIWidget *Int::createWidget(const CreateContext &ctx) const {
      return new IntGUIWidget(*this, ctx);
    }
    GUIWidget *Float::createWidget(const CreateContext &ctx) const {
      return new FloatGUIWidget(*this, ctx);
    }
    GUIWidget *String::createWidget(const CreateContext &ctx) const {
      return new StringGUIWidget(*this, ctx);
    }
    GUIWidget *Label::createWidget(const CreateContext &ctx) const {
      return new LabelGUIWidget(*this, ctx);
    }
    GUIWidget *State::createWidget(const CreateContext &ctx) const {
      return new StateGUIWidget(*this, ctx);
    }
    GUIWidget *Spinner::createWidget(const CreateContext &ctx) const {
      return new SpinnerGUIWidget(*this, ctx);
    }
    GUIWidget *Button::createWidget(const CreateContext &ctx) const {
      if(opts.toggledText.empty()) return new ButtonGUIWidget(*this, ctx);
      return new ToggleButtonGUIWidget(*this, ctx, text, opts.toggledText, opts.initiallyToggled);
    }
    GUIWidget *ToggleButton::createWidget(const CreateContext &ctx) const {
      return new ToggleButtonGUIWidget(*this, ctx, untoggledText, toggledText, initiallyToggled);
    }
    GUIWidget *CheckBox::createWidget(const CreateContext &ctx) const {
      return new CheckBoxGUIWidget(*this, ctx);
    }
    GUIWidget *ButtonGroup::createWidget(const CreateContext &ctx) const {
      return new ButtonGroupGUIWidget(*this, ctx);
    }
    GUIWidget *Combo::createWidget(const CreateContext &ctx) const {
      return new ComboGUIWidget(*this, ctx);
    }
    GUIWidget *Display::createWidget(const CreateContext &ctx) const {
      return new ImageGUIWidget(*this, ctx);
    }
    GUIWidget *Canvas::createWidget(const CreateContext &ctx) const {
      return new DrawGUIWidget(*this, ctx);
    }
    GUIWidget *Canvas3D::createWidget(const CreateContext &ctx) const {
  #ifdef ICL_HAVE_OPENGL
      return new DrawGUIWidget3D(*this, ctx);
  #else
      throw ICLException("Canvas3D is not available without OpenGL support");
  #endif
    }
    GUIWidget *Disp::createWidget(const CreateContext &ctx) const {
      return new DispGUIWidget(*this, ctx);
    }
    GUIWidget *Plot::createWidget(const CreateContext &ctx) const {
      return new PlotGUIWidget(*this, ctx);
    }
    GUIWidget *Fps::createWidget(const CreateContext &ctx) const {
      return new FPSGUIWidget(*this, ctx);
    }
    GUIWidget *ColorSelect::createWidget(const CreateContext &ctx) const {
      return new ColorGUIWidget(*this, ctx);
    }
    GUIWidget *CamCfg::createWidget(const CreateContext &ctx) const {
      return new CamCfgGUIWidget(*this, ctx);
    }
    GUIWidget *Ps::createWidget(const CreateContext &ctx) const {
      return new ProcessMonitorGUIWidget(*this, ctx);
    }
    GUIWidget *Prop::createWidget(const CreateContext &ctx) const {
      return new ConfigurableGUIWidget(*this, ctx);
    }
    GUIWidget *ContainerComponent::createWidget(const CreateContext &ctx) const {
      switch(kind){
        case HBox:      return new HBoxGUIWidget(*this, ctx);
        case VBox:      return new VBoxGUIWidget(*this, ctx);
        case HScroll:   return new ScrollGUIWidgetBase(*this, ctx, QBoxLayout::LeftToRight);
        case VScroll:   return new ScrollGUIWidgetBase(*this, ctx, QBoxLayout::TopToBottom);
        case HSplit:    return new SplitterGUIWidgetBase(*this, ctx, true);
        case VSplit:    return new SplitterGUIWidgetBase(*this, ctx, false);
        case Tab:       return new TabGUIWidget(*this, ctx);
        case Border:    return new BorderGUIWidget(*this, ctx);
        case StatusBar: return new StatusBarGUIWidget(*this, ctx);
      }
      return nullptr;
    }


    GUI::GUI(QWidget *parent):
      // a bare GUI is a vbox container (a real polymorphic component, so its
      // createWidget() dispatches without the registry).
      m_component(new ContainerComponent(ContainerComponent::VBox)),
      m_poWidget(0),m_bCreated(false),m_poParent(parent){
    }

    GUI::GUI(const GUIComponent &component, QWidget *parent):
      m_component(component.clone()),
      m_poWidget(0),m_bCreated(false),m_poParent(parent){
    }


    GUI::GUI(const GUI &g,QWidget *parent):
      m_component(g.getComponent() ? g.getComponent()->clone() : nullptr),
      m_children(g.m_children),
      m_poWidget(nullptr),m_bCreated(false),
      m_poParent(parent){
    }


    GUI &GUI::operator=(const GUI &other){
      m_component = other.getComponent() ? other.getComponent()->clone() : nullptr;
      m_children = other.m_children;
      m_poWidget = nullptr;
      m_bCreated = false;
      m_poParent = other.m_poParent;
      m_oDataStore = other.m_oDataStore;
      return *this;
    }

    bool GUI::isDummy() const{
      const GUIComponent *c = getComponent();
      return !c || c->m_options.hide || c->m_type.empty() || c->m_type == "dummy";
    }

    GUI::~GUI(){
      // this leads to seg-faults
      //delete m_poWidget;
    }

    /// builds a Border container component carrying the (cell-incremented) sizes
    /// of the wrapped component — the structured equivalent of the legacy
    /// label→border string surgery.
    static ContainerComponent makeBorderComponent(const std::string &label,
                                                  const GUIComponent::Options &innerOpts){
      static const Size S11(1,1);
      ContainerComponent b(ContainerComponent::Border, label);
      if(innerOpts.minSize != Size::null) b.minSize(innerOpts.minSize + S11);
      if(innerOpts.maxSize != Size::null) b.maxSize(innerOpts.maxSize + S11);
      if(innerOpts.size    != Size::null) b.size(innerOpts.size + S11);
      return b;
    }

    /// adds a new GUI component
    /** Structured path — no toString()/re-parse round-trip, so free-text
        payloads (a Label's text, a `.label`/`.tooltip`) carrying the grammar
        metacharacters `, ( ) @ =` are carried verbatim. Replicates the legacy
        string operator<<: dummy/hidden are dropped, "!show"/"!create" finalize,
        a label wraps the component in a titled border (with cell-incremented
        sizes), everything else is pushed as a structured child node. */
    GUI &GUI::operator<<(const GUIComponent &component){
      if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }

      const GUIComponent::Options &o = component.m_options;
      const std::string &type = component.m_type;

      if(o.hide || type.empty() || type == "dummy") return *this;
      if(type == "!show"){ show(); return *this; }
      if(type == "!create"){ create(); return *this; }

      if(o.label.length()){
        GUIComponent inner = component;
        inner.m_options.label.clear();
        GUI *borderNode = new GUI(makeBorderComponent(o.label, o));
        (*borderNode) << inner;
        m_children.push_back(borderNode);
        return *this;
      }

      m_children.push_back(new GUI(component));
      return *this;
    }

    void GUI::to_string_recursive(const GUI *gui, std::ostream &str, int level){
      const GUIComponent *c = gui->getComponent();
      const GUIComponent::Options &o = c->options();
      const std::string &type = c->type();
      str << std::string(level*2,' ') << "<" << type;
      if(o.label.length())   str << " label=\"" <<  o.label << "\"";
      if(o.handle.length())  str << " handle=\"" << o.handle << "\"";
      if(o.size    != Size::null) str << " size=\""    << o.size    << "\"";
      if(o.minSize != Size::null) str << " minsize=\"" << o.minSize << "\"";
      if(o.maxSize != Size::null) str << " maxsize=\"" << o.maxSize << "\"";
      if(o.margin  > 0) str << " margin=\""  << o.margin  << "\"";
      if(o.spacing > 0) str << " spacing=\"" << o.spacing << "\"";
      if(o.tooltip.length()) str << " tooltip=\"" << o.tooltip << "\"";
      if(gui->m_children.size()){
        str << ">" << std::endl;
        for(size_t i=0;i<gui->m_children.size();++i){
          to_string_recursive(gui->m_children[i], str, level+1);
        }
        str << std::string(level*2,' ') << "</" << type << ">" << std::endl;
      }else{
        str << "/>" << std::endl;
      }
    }

    std::string GUI::createXMLDescription() const{
      std::ostringstream str;
      to_string_recursive(this,str,0);
      return str.str();
    }

    GUI &GUI::operator<<(const GUI &g){

      if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }

      if(g.isDummy()){
        return *this;
      }

      // Every GUI carries a structured GUIComponent. A label wraps the added
      // node in a titled border (structurally, no serialising); otherwise it is
      // pushed as-is.
      const GUIComponent *gc = g.getComponent();
      if(gc && gc->m_options.label.length()){
        ContainerComponent border = makeBorderComponent(gc->m_options.label, gc->m_options);
        GUI *borderNode = new GUI(border);
        GUI gNew(g);                                   // copy snapshots gc into m_component
        if(gNew.m_component) gNew.m_component->m_options.label.clear();
        (*borderNode) << gNew;
        m_children.push_back(borderNode);
        return *this;
      }
      m_children.push_back(new GUI(g));
      return *this;
    }


    void GUI::create(QLayout *parentLayout,ProxyLayout *proxy,QWidget *parentWidget, DataStore *ds){
      if(ds) m_oDataStore = *ds;
      try{
        if(isDummy()){
          throw ICLException("cannot create a \"dummy\"-GUI (Dummy GUIs are placeholders "
                             "that respect .hide and are skipped by the stream operator)");
        }
        CreateContext ctx{this, parentLayout, proxy, parentWidget};
        m_poWidget = getComponent()->createWidget(ctx);

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
      create();
      m_poWidget->setVisible(true);
    }


    void GUI::waitForCreation(){
      while(!m_bCreated){
  #ifndef WIN32
                usleep(1000*100);
  #else
                Sleep(100);
  #endif
      }
    }

    // The three *Callback*() methods used to reinterpret_cast the
    // stored handle to `GUIHandleBase*` via `MultiTypeMap::getValue`
    // (safe only because every handle inherits `GUIHandleBase` and
    // MultiTypeMap skipped the RTTI check when `typeCheck=false`).
    // std::any's `any_cast<GUIHandleBase&>` matches only the exact
    // stored type, so that trick no longer compiles.  Route through
    // the `Data` proxy instead — its `registerCallback` / `removeCallbacks`
    // smuggle an `Event` through AssignRegistry, which dispatches to
    // the registered `Assign<H, Event>::apply` (see
    // `icl/qt/HandleEventEnrollments.cpp`) for every handle that
    // supports callbacks.  To be retired together with the Event
    // smuggling itself.

    void GUI::registerCallback(const Callback &cb, const std::string &handleNamesList, char delim){
      std::string delims; delims+=delim;

      StrTok tok(handleNamesList,delims);
      while(tok.hasMoreTokens()){
        m_oDataStore[tok.nextToken()].registerCallback(cb);
      }
    }

    void GUI::registerCallback(const ComplexCallback &cb, const std::string &handleNamesList, char delim){
      std::string delims; delims+=delim;

      StrTok tok(handleNamesList,delims);
      while(tok.hasMoreTokens()){
        m_oDataStore[tok.nextToken()].registerCallback(cb);
      }
    }

    void GUI::removeCallbacks(const std::string &handleNamesList, char delim){
      std::string delims; delims+=delim;

      StrTok tok(handleNamesList,delims);
      while(tok.hasMoreTokens()){
        m_oDataStore[tok.nextToken()].removeCallbacks();
      }
    }

  } // namespace qt
}

//  LocalWords:  if
