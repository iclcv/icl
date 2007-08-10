#ifndef ICL_GUI_DEFINITION_H
#define ICL_GUI_DEFINITION_H

#include <string>
#include <vector>
#include <iclSize.h>

/** \cond */
class QLayout;
/** \endcond */

namespace icl{
  /** \cond */
  class GUI;
  /** \endcond */

  /// Utilty class to simplify creation of GUI components
  class GUIDefinition{
    public:
    /// create a new GUI Definition
    GUIDefinition(const std::string &def, GUI *gui, QLayout *parentLayout=0);
      
    /// return the type string
    const std::string &type() const { return m_sType; }

    /// return the lable string
    const std::string &label() const { return m_sLabel; }
    
    /// return this size (or Size::null)
    const Size &size() const { return m_oSize; }
    
    /// return the parent GUI
    GUI *getGUI() const { return m_poGUI; }
    
    /// return the parent layout or null if there is nor parent
    QLayout *getParentLayout() const { return m_poParentLayout; }
    
    /// returns a param at given index
    const std::string &param(unsigned int idx) const;
    
    /// returns a param at given index as int (using atoi)
    int intParam(unsigned int idx) const;
    
    /// returns a param at given index as float (using atof)
    float floatParam(unsigned int idx) const;
    
    /// returns an output name
    const std::string &output(unsigned int idx) const;
    
    /// returns an input name
    const std::string &input(unsigned int idx) const;
    
    /// show this GUIDefinition to std::out
    void show() const;

  private:
    std::string m_sType;                   //<! parsed type string 
    std::vector<std::string> m_vecParams;  //<! vector of mandatory params
    std::vector<std::string> m_vecOutputs; //<! vector of output names
    std::vector<std::string> m_vecInputs;  //<! vector of input names
    std::string m_sLabel;                  //<! parsed label (unused this time)
    Size m_oSize;                          //<! parsed size
    GUI *m_poGUI;                          //<! parent GUI
    QLayout *m_poParentLayout;             //<! parent layout or NULL if there is no parent
  };
}

#endif
