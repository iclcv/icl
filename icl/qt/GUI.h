// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/DataStore.h>
#include <functional>
#include <icl/qt/GUIComponents.h>
#include <QLayout>
#include <QWidget>
#include <QApplication>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

/** \cond */
class QLayout;
/** \endcond */

namespace icl::qt {
  /** \cond */
  class GUIWidget;
  class ProxyLayout;
  class GUIDefinition;
  /** \endcond */

  /// Main Class of ICL's GUI creation framework
  /** Please refer to the ICL manual for details */
  class ICLQt_API GUI{
    protected:


    public:
    /// registered widget type creator function
    using CreatorFunction = std::function<GUIWidget*(const GUIDefinition&)>;

    /// registers a new widget type
    /** The registered widget can then be added using a corresponding extension
        of the GUIComponent class */
    static void register_widget_type(const std::string &tag, CreatorFunction f);

    /// cell width (all sizes are given in this unit)
    static const int CELLW = 20;
    /// cell height (all sizes are given in this unit)
    static const int CELLH = 20;

    /// Default constructor, creates a vbox GUI Component
    GUI(QWidget *parent=0);

    /// creates a GUI from a givne GUIComponent
    GUI(const GUIComponent &component, QWidget *parent=0);

    /// copy constructor
    GUI(const GUI &gui,QWidget *parent=0);

    /// gui-copy operator
    GUI &operator=(const GUI &other);

    /// Destructor
    virtual ~GUI();

    /// adds a new GUI component
    virtual GUI &operator<<(const GUIComponent &component);

    /// stream operator to add new other GUIs
    /** if the given GUI is a dummy GUI, this operator does nothing */
    virtual GUI &operator<<(const GUI &g);

    /// wraps the data-stores allocValue function
    template<class T>
    inline T &allocValue(const std::string &id, const T&val=T()){
      return m_oDataStore.allocValue<T>(id,val);
    }

    template<class T>
    T &get(const std::string &id, bool typeCheck=true){
      return m_oDataStore.getValue<T>(id,typeCheck);
    }

    /// returns a Data instance from the datastore
    DataStore::Slot operator[](const std::string &key){
      return m_oDataStore.operator[](key);
    }

    /// collects data from different components at once
    template<class T>
    std::vector<T> collect(const std::vector<std::string> &keys){
      return m_oDataStore.collect<T>(keys);
    }

    /// returns whether this gui is actually visible
    virtual bool isVisible() const;

    /// internally creates everything
    virtual void create();

    /// internally creates everything (and makes the gui visible)
    virtual void show();

    /// make this gui invisible (nothing more)
    virtual void hide();

    /// if widget is visible, this hides the widget, otherwise the widget is shown
    virtual void switchVisibility();

    /// returns the root widget of the gui (only avialable after create() or show())
    inline GUIWidget *getRootWidget(){
      return m_poWidget;
    }

    /// internally locks the datastore
    inline void lockData() {
      m_oDataStore.lock();
    }
    /// internally unlocks the data store
    inline void unlockData() {
      m_oDataStore.unlock();
    }
    /// waits for the gui to be created completely
    void waitForCreation();

    /// returns the GUI internal dataStore
    const DataStore &getDataStore() const { return m_oDataStore; }

    /// simple callback, that can be registered at GUI components
    /** Simple callback methods don't get any information about the source */
    using Callback = std::function<void()>;

    /// complex callback type that can be registered at GUI components
    /** Complex callback methods get the GUI components handle name as
        parameters. By these means, single callbacks can be registered
        to several components and still be able to handle events differently */
    using ComplexCallback = std::function<void(const std::string&)>;


    /// registers a callback function on each component
    /** @param cb callback to execute
        @param handleNamesList 'listDelim'-separated list of handle names
        @param listDelim delimiter for the handle list
        ownership is passed to the childrens; deletion is performed by
        the smart pointers that are used...
    */
    void registerCallback(const Callback &cb, const std::string &handleNamesList, char listDelim=',');

    /// registeres a complex callback at a given GUI component
    /** complex callbacks are called with the actual GUI components handle name as parameter */
    void registerCallback(const ComplexCallback &cb, const std::string &handleNamesList, char listDelim=',');

    /// removes all callbacks from components
    void removeCallbacks(const std::string &handleNamesList, char listDelim=',');

    /// returns whether this GUI is a dummy GUI
    /** Please note: dummy GUIs
        * cannot be created
        * are not added by the stream operator
    */
    bool isDummy() const;

    /// returns whether this GUI has been created or not
    bool hasBeenCreated() const;

    /// creates a hierarchical xml-description of the GUI Layout
    std::string createXMLDescription() const;

    protected:
    /// structured payload of this node (the single source of truth)
    /** create() builds the widget straight from this GUIComponent — there is
        no string serialisation / re-parse, so free-text payloads carrying the
        grammar metacharacters survive intact.  ContainerGUIComponent overrides
        this to expose its accumulating component. */
    virtual const GUIComponent *getComponent() const { return m_component.get(); }

    /// mutable view of getComponent() for in-place option accumulation
    /** Used by ContainerGUIComponent's chained setters. */
    GUIComponent *mutableComponent() const { return m_component.get(); }

    private:

    static void to_string_recursive(const GUI *gui, std::ostream &str, int level);

    void create(QLayout *parentLayout,ProxyLayout *proxy, QWidget *parentWidget, DataStore *ds);

    /// structured form of this node (every node has one)
    std::shared_ptr<GUIComponent> m_component;
    std::vector<GUI*> m_children;
    GUIWidget *m_poWidget;
    DataStore m_oDataStore;
    bool m_bCreated;
    QWidget *m_poParent;
  };
  } // namespace icl::qt