#ifndef ICL_HISTOGRAMM_WIDGET_H
#define ICL_HISTOGRAMM_WIDGET_H

#include <ICLQt/ThreadedUpdatableWidget.h>
#include <ICLQt/ImageStatistics.h>
#include <ICLUtils/Mutex.h>
#include <vector>


namespace icl{
  
  /// Utility class used within the ICLWidget class
  /** The HistogrammWidget is used in the 'info' tab of the ICLWidget's on screen display */
  struct HistogrammWidget : public ThreadedUpdatableWidget{

    /// internal entries
    struct Entry{
      float color[3]; //!< color
      std::vector<int> histo; //!< histogram
    };
    /// entry list
    std::vector<Entry> entries;

    /// logarithmic mode
    bool logOn;
    
    /// mean averaging
    bool meanOn;
    
    /// median filtering
    bool medianOn;
    
    /// draw filled
    bool fillOn;
    
    /// draw accumulated (not yet implemented)
    bool accuMode;
    
    /// selected channel (or -1 for all)
    int selChannel;
    
    /// internally used mutex
    Mutex mutex;

    /// Base constructor with given parent widget
    HistogrammWidget(QWidget *parent);
    
    /// sets all features
    void setFeatures(bool logOn, bool meanOn, bool medianOn, bool fillOn, int selChannel, bool accuMode=false);
    
    /// sets the fill color
    void fillColor(int i,float color[3]);
    
    /// calls QWidget::update
    virtual void update();

    /// updates the histogramms
    void update(const ImageStatistics &s);
    
    /// overloaded painting
    virtual void paintEvent(QPaintEvent *e);
  };


}


#endif
