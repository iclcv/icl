/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ChromaGUI.h                              **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_CHROMA_GUI_H
#define ICL_CHROMA_GUI_H

#include <ICLQt/GUI.h>
#include <ICLCC/ChromaClassifier.h>
#include <ICLCC/ChromaAndRGBClassifier.h>
#include <ICLQt/SliderHandle.h>
namespace icl{

  /** \cond */
  class ChromaWidget;
  /** \endcond */
  
  /// Dedicated GUI component which can be used to adjust segmentation parameters \ingroup COMMON
  /** The RG-Chromaticity space is a special Color-Space, which can be used for
      a lighting-independend skin color segmentation. The segmentation is parameterized
      by two parabolic functions \f$P_1(x)\f$ and \f$P_2(x)\f$.
      The pixel classification rule (is pixels skin-colored or not) can be defindes as 
      follows:
      \code
      bool is_pixel_skin_colored(int r,int g,int b, Parable p1, Parable p2){
         int R = (255*r)/(r+g+b+1); // "+1" to avoid a div-zero
         int G = (255*g)/(r+g+b+1);
         return p1(R) > G && p2(R) < G;
      }
      \endcode
      The so called "skin-locus" is defined by the two enclosing parables \f$P_1(x)\f$ and 
      \f$P_2(x)\f$. A pixel is skin colored if it is between the two parables - or more
      precisely: if it is on the bottom side of the first and on the top side of the second.
      
      In addition the ChromaGUI class provides a simple RGB thresholded reference color
      segmenter, which can be combined with the chromaticity-space segmenter, and which 
      can also be adjusted using GUI components.
  **/
  class ChromaGUI : public QObject, public GUI{
    Q_OBJECT
    public:
    
    /// Create a new ChromaGUI with given parent widget
    ChromaGUI(QWidget *parent=0);
    
    /// retuns a ChomaClassifier with current parameters
    ChromaClassifier getChromaClassifier();

    /// retuns a CombiClassifier with current parameters
    ChromaAndRGBClassifier getChromaAndRGBClassifier();
    
    public slots:
    /// used for the blue-value visualization slider
    void blueSliderChanged(int val);
    
    /// used for the save-button
    void save(const std::string &filename="");
    
    /// used for the load-button
    void load(const std::string &filename="");
    
    private:
    /// Wrapped ChromaWidget (an internal class)
    ChromaWidget *m_poChromaWidget;

    /// Handles to the embedded sliders
    SliderHandle m_aoSliderHandles[2][3];
    
  };
}

#endif
