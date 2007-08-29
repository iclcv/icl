#ifndef ICL_CHROMA_GUI_H
#define ICL_CHROMA_GUI_H

#include <iclGUI.h>
#include <iclParable.h>

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
    Q_OBJECT;
    public:
    
    /// Classifier interface using RG-chromaticity space and two parables \ingroup COMMON
    struct ChromaClassifier{
      public:
      /// classifies a given R-G-Pixel
      inline bool operator()(icl8u chromaR, icl8u chromaG) const{
        return parables[0](chromaR) > chromaG && parables[1](chromaR) < chromaG;
      }
      /// classifies a given r-g-b-Pixel
      inline bool operator()(icl8u r, icl8u g, icl8u b) const{
        int sum = r+g+b+1;
        return (*this)((255*r)/sum,(255*g)/sum);
      }
      /// Shows this classifier to std::out
      void show()const{
        parables.show();
      }
      /// Used two parables
      ParableSet parables;
    };
    
    /// Combination classifier using RG-chroma. as well as RGB-thresholded reference color classifiation \ingroup COMMON
    struct CombiClassifier{
      /// classifies a given r-g-b-Pixel
      /**The function is:
          \code
          bool is_pixel_skin_colored(int r, int g, int b, ChromaClassifier c, int refcol[3], int threshold[3]){
             return c(r,g,b) 
                    && abs(r-refcol[0])<threshold[0]
                    && abs(g-refcol[1])<threshold[1]
                    && abs(b-refcol[2])<threshold[2];
          }
          \endcode
      */
      inline bool operator()(icl8u r, icl8u g, icl8u b) const{
        return c(r,g,b) && ::abs(r-ref[0])<thresh[0] && ::abs(g-ref[1])<thresh[1] && ::abs(b-ref[2])<thresh[2];
      }
      /// wrapped ChromaClassifier
      ChromaClassifier c;
      
      /// r-g-b reference color
      icl8u ref[3];
      
      /// r-g-b threshold
      icl8u thresh[3];
      
      /// shows this classifier to std::out
      void show()const{
        printf("Combi-Classifier\n");
        c.show();
        printf("reference color:  %d %d %d \n",ref[0],ref[1],ref[2]);
        printf("color thresholds: %d %d %d \n",thresh[0],thresh[1],thresh[2]);
      }
    };
    
    /// Create a new ChromaGUI with given parent widget
    ChromaGUI(QWidget *parent=0);
    
    /// retuns a ChomaClassifier with current parameters
    ChromaClassifier getChromaClassifier();

    /// retuns a CombiClassifier with current parameters
    CombiClassifier getCombiClassifier();
    
    
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
