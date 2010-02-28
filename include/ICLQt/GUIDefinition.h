#ifndef ICL_GUI_DEFINITION_H
#define ICL_GUI_DEFINITION_H

#include <string>
#include <vector>
#include <ICLUtils/Size.h>

/** \cond */
class QLayout;
class QWidget;
/** \endcond */

namespace icl{
  /** \cond */
  class GUI;
  class ProxyLayout;
  /** \endcond */

  /// Utilty class to simplify creation of GUI components \ingroup UNCOMMON
  class GUIDefinition{
    public:
    /// create a new GUI Definition
    GUIDefinition(const std::string &def, GUI *gui, QLayout *parentLayout=0, ProxyLayout *parentProxyLayout=0, QWidget *parentWidget=0);
      
    /// return the type string
    const std::string &type() const { return m_sType; }

    /// return the lable string
    const std::string &label() const { return m_sLabel; }

    /// return the handle id string
    const std::string &handle() const { return m_sHandle; }
    
    /// return this size (or Size::null)
    const Size &size() const { return m_oSize; }
    
    /// retunrs the minimum size of the widget or size::null
    const Size &minSize() const { return m_oMinSize; }

    /// returns the maxinum size of the widget or size::null
    const Size &maxSize() const { return m_oMaxSize; }
    
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

  private:
    std::string m_sDefinitionString;       //<! whole definition string
    std::string m_sType;                   //<! parsed type string 
    std::vector<std::string> m_vecParams;  //<! vector of mandatory params
    std::vector<std::string> m_vecOutputs; //<! vector of output names
    std::vector<std::string> m_vecInputs;  //<! vector of input names
    std::string m_sLabel;                  //<! parsed label (unused this time)
    std::string m_sHandle;                 //<! parsed handle id string
    Size m_oSize;                          //<! parsed size
    Size m_oMinSize;                       //<! parsed minimal size
    Size m_oMaxSize;                       //<! parsed maximum size
    int m_iMargin;                         //<! parsed layout margin
    int m_iSpacing;                        //<! parsed layout spacing
    GUI *m_poGUI;                          //<! parent GUI
    QLayout *m_poParentLayout;             //<! parent layout or NULL if there is no parent
    QWidget *m_poParentWidget;             //<! parent widget to avoid reparenting calls
    ProxyLayout *m_poParentProxyLayout;    //<! parent widget proxy layout
  };
}

#endif
