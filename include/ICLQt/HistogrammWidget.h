#ifndef ICL_HISTOGRAMM_WIDGET_H
#define ICL_HISTOGRAMM_WIDGET_H

#include <ICLQt/PlotWidget.h>
#include <ICLQt/ImageStatistics.h>

namespace icl{
  
  /// Utility class used within the ICLWidget class
  /** The HistogrammWidget is used in the 'info' tab of the ICLWidget's on screen display */
  struct HistogrammWidget : public PlotWidget{
    /// logarithmic mode
    bool logOn;
    
    /// mean averaging
    bool meanOn;
    
    /// median filtering
    bool medianOn;
    
    /// selected channel (or -1 for all)
    int selChannel;

    std::vector<float> buf;

    /// Base constructor with given parent widget
    HistogrammWidget(QWidget *parent);
    
    /// sets all features
    void setFeatures(bool logOn, bool meanOn, bool medianOn, int selChannel);
    
    /// updates the histogramms
    void updateData(const ImageStatistics &s);
  };


}


#endif
