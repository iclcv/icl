/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DynamicGUI.h                           **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
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
#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLQt/GUI.h>

/** \cond */
namespace pugi { class xml_node; }
/** \endcond */

namespace icl{
  namespace qt{

    /// Special GUI implementation that allows for defining the GUI layout in an XML-file
    /** In some cases, it might be useful to not embed the design of the
        GUI layout into the c++-source code. To this end, ICL provides
        the DynamicGUI class, which is defined from XML that is
        dynamically loaded and parsed at runtime. By this, it is
        possible to add or re-arrange GUI components or to adapt
        features of these such as sizes or labels dynamically,
        i.e. without having to recompile the C++ application. The
        abstraction mechanism can also be used to implement different
        GUI layouts e.g. optimized for different screen resolutions.

        Here is an examplary xml-definition file
        <pre>
        <?xml version="1.0"?>
        <hbox>
          <vbox minsize="16x12" label="a box">
            <button args="Push" minsize="10x2" handle="the button"/>
            <togglebutton args="Toggle,Me" minsize="10x2" handle="the t-button"/>
            <string args="test,20" handle="string"/>
            <int args="0,1000,500" handle="int"/>
            <checkbox args="my checkbox,toggled" handle="checkbox"/>
            <float args="-0.2,0.8,0.6" handle="float" label="float"/>
          </vbox>
          \<include href="other.xml"/\>
          <vsplit>
            <image minsize="16x12" label="an image" handle="image"/>
            <slider args="0,255,0" label="the slider" handle="s"/>
            <hbox>
              <image minsize="16x12" label="another image" handle="what"/>
              <slider args="0,255,0" label="yet another slider"/>
            </hbox>
          </vsplit>
          <hbox margin="20" label="box with margin">
            <image minsize="16x12" label="last image then" handle="otherimage"/>
            <slider args="0,255,0" label="slider slider slider"/>
          </hbox>
        </hbox>
      </pre>

        As one can see, the hierarchical GUI definition syntax is generically
        translated to the XML-structure.

        \section INC the include tag
        With the include tag, (e.g. \<include href="other.xml"/\>) GUI files can even be
        included in a hierarchical fashion, but here, the user has to take care to not create
        infinite inclusion loops.
    */
    class DynamicGUI : public qt::GUI, public utils::Uncopyable{
      /// internal data
      struct Data;

      /// internal data pointer
      Data *m_data;

      public:

      /// internal xml-parsing node class
      /** The node class represents the hierarchical structure of the
          GUI definition and by exposing the interface, it can be used
          and re-parsed by other components Each GUI component (e.g. a
          Slider) is defined by its name (which is 'slider') and a set
          of arguments. Here, the class distinguishes between direct
          arguments, which are the component specific arguments, e.g. in
          case of a slider, the min, the max and the initial value, such
          as  "0,255,128". In addition other arguments are stored in a
          key-value-pair fashion, such as "handle='the slider'".
          */
      class Node{
        typedef utils::SmartPtr<Node> NodePtr;                 //!< SmartPtr typedef
        typedef std::pair<std::string,std::string> KeyValue;   //!< KeyValue pair class

        Node(const std::string &name="", NodePtr parent=NodePtr(), int level=0); // private constructur

        /// internally used appending of children
        NodePtr appendChild(const std::string &name, NodePtr parent, int level);

        /// internal utility methods
        void grabArgs(const pugi::xml_node &n);

        /// makes parent DynamicGUI class a friend of this one
        friend class DynamicGUI;


        public:
        std::string name;  //!< component type (e.g. 'slider', or 'combo' or 'image')
        NodePtr parent;    //!< parent container component (or null for the top-level component)
        int level;         //!< hierarchy containment level of this component
        qt::GUI parsedGUI; //!< corresponding parse (but not created) GUI instance

        std::string directArgs;            //!< direct, e.g. component specific arguments
        std::vector<KeyValue> args;        //!< general arguments, such as handle, label or minsize
        std::vector<NodePtr> children;     //!< all child-components (for containers only)

        bool isContainer() const;          //!< dedicated method to distinguish containers
        bool hasHandle() const;            //!< returns whether a handle was given (e.g. is part of args)
        std::string getHandleName() const; //!< returns the handle (if there is one)

      };

      typedef utils::SmartPtr<Node> NodePtr;  //!< typedef for node-pointers
      typedef Node ParseTree;                 //!< A tree is simply the root-node
      typedef utils::SmartPtr<ParseTree> ParseTreePtr; //!< pointer typedef

      /// creates a new DynamicGUI instance from a given XML-description filename
      /** if cfgFileName is "", a null GUI instance is created that can be initialized
          in hindsight using initialize or load */
      DynamicGUI(const std::string &cfgFileName="", QWidget *parent=0);

      /// Destructor
      ~DynamicGUI();

      /// intialize DynamicGUI instance from given XML-string
      /** here, the XML-string is already in memory */
      void initialize(const std::string &cfgXMLString);

      /// intialize DynamicGUI instance from given XML-file
      void load(const std::string &cfgFileName);

      /// internally releases everything
      void release();

      /// returns the internal parse-tree representation
      ParseTreePtr getParseTree();


      private:
      /// internally used initialization method
      void initInternal(pugi::xml_node &root);

      /// ostream operator for the node class
      friend std::ostream &operator<<(std::ostream &s, const DynamicGUI::Node &n);

      /// internal tree-traversal method
      static void traverse_tree(const pugi::xml_node &n, int level,
                                utils::SmartPtr<DynamicGUI::Node> target);

      /// internal GUI-creation method
      static void create_gui(Node &n);
    };

  }

}
