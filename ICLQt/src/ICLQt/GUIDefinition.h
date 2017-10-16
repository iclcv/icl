/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GUIDefinition.h                        **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Size.h>
#include <string>
#include <vector>

/** \cond */
class QLayout;
class QWidget;
/** \endcond */

namespace icl{
  namespace qt{

    /** \cond */
    class GUI;
    class ProxyLayout;
    /** \endcond */

    /// Utilty class to simplify creation of GUI components \ingroup UNCOMMON
    class ICLQt_API GUIDefinition{
      public:
      /// create a new GUI Definition
      GUIDefinition(const std::string &def, GUI *gui,
                    QLayout *parentLayout=0,
                    ProxyLayout *parentProxyLayout=0,
                    QWidget *parentWidget=0);

      /// return the type string
      const std::string &type() const { return m_sType; }

      /// return the lable string
      const std::string &label() const { return m_sLabel; }

      /// return the handle id string
      const std::string &handle() const { return m_sHandle; }

      /// return this size (or utils::Size::null)
      const utils::Size &size() const { return m_oSize; }

      /// retunrs the minimum size of the widget or size::null
      const utils::Size &minSize() const { return m_oMinSize; }

      /// returns the maxinum size of the widget or size::null
      const utils::Size &maxSize() const { return m_oMaxSize; }

      /// returns the layout margin
      int margin() const { return m_iMargin; }

      /// returns the layout spacing
      int spacing() const { return m_iSpacing; }

      /// return the parent GUI
      GUI *getGUI() const { return m_poGUI; }

      /// return the parent layout or null if there is nor parent
      QLayout *parentLayout() const { return m_poParentLayout; }

      /// returns the parent widgets proxy layout
      ProxyLayout *getProxyLayout() const { return m_poParentProxyLayout; }

      /// returns the number of params
      unsigned int numParams() const { return m_vecParams.size(); }

      /// returns a param at given index
      const std::string &param(unsigned int idx) const;

      /// returns a param at given index as int (using atoi)
      int intParam(unsigned int idx) const;

      /// returns a param at given index as float (using atof)
      float floatParam(unsigned int idx) const;

      /// returns an input name
      /** if the given input index was actually not defined, a dummy input name is
          created */
      const std::string &input(unsigned int idx) const;

      /// returns an output name
      /** if the given output index was actually not defined, a dummy output name is
          created */
      const std::string &output(unsigned int idx) const;

      /// returns the number of params
      unsigned int numInputs() const { return m_vecInputs.size(); }

      /// returns the number of params
      unsigned int numOutputs() const { return m_vecOutputs.size(); }

      /// show this GUIDefinition to std::out
      void show() const;

      /// returns the whole definition string (for debugging!)
      const std::string &defString() const { return m_sDefinitionString; }

      /// returns the current parent widget (or 0 if there is non)
      QWidget *parentWidget() const { return m_poParentWidget; }

      /// returns a list of all parameters
      const std::vector<std::string> &allParams() const { return m_vecParams; }

      /// returns the tooltip text
      inline const std::string &toolTip() const { return m_toolTip; }

      /// returns whether the tooltip text is not ""
      inline bool hasToolTip() const { return m_toolTip.length(); }

    private:
      std::string m_sDefinitionString;       //<! whole definition string
      std::string m_sType;                   //<! parsed type string
      std::vector<std::string> m_vecParams;  //<! vector of mandatory params
      std::vector<std::string> m_vecOutputs; //<! vector of output names
      std::vector<std::string> m_vecInputs;  //<! vector of input names
      std::string m_sLabel;                  //<! parsed label (unused this time)
      std::string m_sHandle;                 //<! parsed handle id string
      utils::Size m_oSize;                          //<! parsed size
      utils::Size m_oMinSize;                       //<! parsed minimal size
      utils::Size m_oMaxSize;                       //<! parsed maximum size
      int m_iMargin;                         //<! parsed layout margin
      int m_iSpacing;                        //<! parsed layout spacing
      GUI *m_poGUI;                          //<! parent GUI
      QLayout *m_poParentLayout;             //<! parent layout or NULL if there is no parent
      QWidget *m_poParentWidget;             //<! parent widget to avoid reparenting calls
      ProxyLayout *m_poParentProxyLayout;    //<! parent widget proxy layout
      std::string m_toolTip;                 //<! optionally given tooltip
    };
  } // namespace qt
}

