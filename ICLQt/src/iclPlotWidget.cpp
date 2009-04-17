#include <iclPlotWidget.h>

#include <iclGUIWidget.h>
#include <iclMutex.h>
#include <algorithm>
#include <iclMathematics.h>

namespace icl{

  struct FunctionFunctor{
    PlotWidget::Function *f;
    FunctionFunctor(PlotWidget::Function *f):f(f){}
    inline float operator()(float val){
      return (*f)(val);
    }
  };

  std::vector<float> PlotWidget::Function::operator()(const std::vector<float> &xs){
    std::vector<float> ys(xs.size());
    std::transform(xs.begin(),xs.end(),ys.begin(),FunctionFunctor(this));    
    return xs;
  }
  std::vector<float> &PlotWidget::Function::operator()(const std::vector<float> &xs, std::vector<float> &dst){
    dst.resize(xs.size());
    std::transform(xs.begin(),xs.end(),dst.begin(),FunctionFunctor(this));    
    return dst;
  }
  

  struct PlotWidget::Data{
    Data(bool useControlGUIUsed):
      controlGUIUsed(useControlGUIUsed),selectedFunction(-1),
      logMode(false),accuMode(false),fillMode(false),
      blurMode("none"),aaMode(true),xTicsEnabled(false),xTicsSpacing(1),
      yTicsEnabled(false),yTicsSpacing(1),ticLabelsEnabled(false),
      leftMargin(5),topMargin(5),rightMargin(5),
      bottomMargin(5),controlGUIHeight(150),xsRangeHint(0){
    }
    
    ~Data(){
      for(unsigned int i=0;i<funcs.size();++i){
        delete funcs[i];
      }
      ICL_DELETE(xsRangeHint);
    }
    std::vector<PlotWidget::Function*> funcs;

    bool controlGUIUsed;
    GUI controlGUI;  

    int selectedFunction;

    bool logMode;
    bool accuMode;
    bool fillMode;
    bool aaMode;
    QString blurMode;
    
    
    bool xTicsEnabled;
    float xTicsSpacing;
    bool yTicsEnabled;
    float yTicsSpacing;
    
    bool ticLabelsEnabled;

    int leftMargin;
    int topMargin;    
    int rightMargin;
    int bottomMargin;
    int controlGUIHeight;
  
    // one of this is used (that one that is not null)
    std::vector<icl32f> xsValues;
    Range32f *xsRangeHint;
    Mutex mutex;
    
    QRectF getWidgetViewPort(const QSize &s){
      bool top = this->topMargin + (controlGUIUsed ? controlGUIHeight : 0);
      return QRectF(leftMargin,top,s.width()-(leftMargin+rightMargin),s.height()-(topMargin+bottomMargin));
    }
    
    Range32f bounds(const std::vector<float> &v){
      if(!v.size()) return Range32f(0,0);
      float min_ = *std::min_element(xsValues.begin(),xsValues.end());
      float max_ = *std::max_element(xsValues.begin(),xsValues.end());
      return Range32f(min_,max_);      
    }
    
    Range32f getXRange(){
      if(xsRangeHint){
        return *xsRangeHint;
      }else{
        return bounds(xsValues);
      }
    }
    inline float adaptLog(float f){
      if(logMode){
        if(f<0) return 0;
        else return ::log(f);
      }else{
        return f;
      }
    }
    
    Range32f adaptLog(const Range32f &r){
      return Range32f(adaptLog(r.minVal),adaptLog(r.maxVal));
    }
    
    Range32f getYRange(){
      unsigned int N = funcs.size();
      if(selectedFunction>=0 && selectedFunction<N){
        /// accuMode doesn't matter here
        Function &f = *funcs[selectedFunction];
        Range32f yr = f.yrange();
        if(yr != Range32f(0,0)){
          return adaptLog(yr);
        }else{
          return adaptLog(bounds(f(xsValues)));
        }
      }
      if(accuMode){          
        bool allFuncsHaveRanges = true;
        Range32f r = Range32f::limits();
        for(unsigned int j=0;j<xsValues.size();++j){
          float sum = 0;
          for(unsigned int i=0;i<N;++i){
            sum += (*funcs[i])(xsValues[j]);
          }
          if(sum < r.minVal) r.minVal = sum;
          if(sum > r.maxVal) r.maxVal = sum;
        }
        return adaptLog(r);
      }
      int selectedFunctionSave = selectedFunction;
      Range32f r = Range32f::limits();
      std::swap(r.minVal,r.maxVal);
      for(unsigned int i=0;i<N;++i){
        selectedFunction = i;
        Range32f ri = getYRange();
        if(ri.minVal < r.minVal) r.minVal = ri.minVal;
        if(ri.maxVal < r.maxVal) r.maxVal = ri.maxVal;
      }
      selectedFunction = selectedFunctionSave;
      return r;
    }

    QRectF getDataViewPort(){
      Range32f xrange = getXRange();
      Range32f yrange = getYRange();
      return QRectF(xrange.minVal,yrange.minVal,xrange.getLength(),yrange.getLength());
    }
  };
  
#define LOCK_DATA Mutex::Locker LOCAL_MUTEX_LOCKER(m_data->mutex);
  
  void create_control_gui(PlotWidget *widget, PlotWidget::Data *data){
    


    
  }

  
  PlotWidget::PlotWidget(bool createControlGUI, QWidget *parent):
    QGLWidget(parent),m_data(new PlotWidget::Data(createControlGUI)){
    
    setBackgroundRole(QPalette::Base);
  }
  
  PlotWidget::~PlotWidget(){
    delete m_data;
  }
  
  
  void PlotWidget::paintEvent(QPaintEvent *e){
    LOCK_DATA;
    float sx=0,sy=0,ox=0,oy=0;
    QRectF widgetViewPort = m_data->getWidgetViewPort(size());
    QRectF dataViewPort = m_data->getDataViewPort();

  }
  
  void PlotWidget::addFunction(Function *f){
    LOCK_DATA;
    m_data->funcs.push_back(f);
  }
  
  void PlotWidget::deleteFunction(Function *f){
    LOCK_DATA;
    std::vector<Function*>::iterator it = std::find(m_data->funcs.begin(),m_data->funcs.end(),f);
    if(it != m_data->funcs.end()){
      delete *it;
      m_data->funcs.erase(it);
    }
  }
  void PlotWidget::removeFunction(Function *f){
    LOCK_DATA;
    std::vector<Function*>::iterator it = std::find(m_data->funcs.begin(),m_data->funcs.end(),f);
    if(it != m_data->funcs.end()){
      m_data->funcs.erase(it);
    }
  }
  
  void PlotWidget::deleteAllFunctions(){
    LOCK_DATA;
    for(unsigned int i=0;i<m_data->funcs.size();++i){
      delete m_data->funcs[i];
    }
    m_data->funcs.clear();
    
  }
  void PlotWidget::removeAllFunctions(){
    LOCK_DATA;
    m_data->funcs.clear();
  }
  
  void PlotWidget::setMargins(int left, int top, int right, int bottom){
    LOCK_DATA;
    m_data->leftMargin = left;
    m_data->topMargin = top;
    m_data->rightMargin = right;
    m_data->bottomMargin = bottom;
    
  }
  void PlotWidget::setXTicsSpacing(float val){
    LOCK_DATA;
    m_data->xTicsSpacing = val;
  }
  void PlotWidget::setYTicsSpacing(float val){
    LOCK_DATA;
    m_data->yTicsSpacing = val;
  }

  void PlotWidget::setXRange(const SteppingRange<icl32f> &xs){
    LOCK_DATA;
    m_data->xsValues.clear();
    m_data->xsValues.reserve(xs.getLength()/xs.stepping+1);
    for(float f=xs.minVal;f<=xs.maxVal;f+=xs.stepping){
      m_data->xsValues.push_back(f);
    }
    ICL_DELETE(m_data->xsRangeHint);
    m_data->xsRangeHint = new Range32f(xs.minVal,xs.maxVal);
  }
  void PlotWidget::setXValues(const std::vector<icl32f> &xs){
    LOCK_DATA;
    m_data->xsValues = xs;
    ICL_DELETE(m_data->xsRangeHint);
  }
  
  void PlotWidget::setFillModeEnabled(bool enabled){
    LOCK_DATA;
    m_data->fillMode = enabled;
  }
  void PlotWidget::setLogModeEnabled(bool enabled){
    LOCK_DATA;
    m_data->logMode = enabled;
  }
  void PlotWidget::setAccuModeEnalbed(bool enabled){
    LOCK_DATA;
    m_data->accuMode = enabled;
  }
  void PlotWidget::setAAMode(bool enabled){    
    LOCK_DATA;
    m_data->aaMode = enabled;
  }
  
  /// how to blur functions (none, mean3, mean5, median)
  void PlotWidget::setBlurModeEnabled(const QString &mode){
    LOCK_DATA;
    m_data->blurMode = mode;
  }
  
  void PlotWidget::setControlGUIEnabled(bool enabled){
    LOCK_DATA;
    // XXX TODO create menu or not
    if(m_data->controlGUIUsed != enabled){
      m_data->controlGUIUsed == enabled;
      if(enabled){
        create_control_gui(this,m_data);
      }else{
        delete m_data->controlGUI.getRootWidget();
        m_data->controlGUI = GUI();
      }
    }
  }
  
  void PlotWidget::selectFunction(int index){
    LOCK_DATA;
    m_data->selectedFunction = index;
  }
  
  void PlotWidget::deselectFunction(){
    LOCK_DATA;
    m_data->selectedFunction = -1;
  }
  
  
  void PlotWidget::setXTicsEnabled(bool enabled){
    LOCK_DATA;
    m_data->xTicsEnabled = enabled;
  }
  void PlotWidget::setYTicsEnabled(bool enabled){
    LOCK_DATA;
    m_data->yTicsEnabled = enabled;
    
  }
  void PlotWidget::setTicLablesEnabled(bool enabled){
    LOCK_DATA;
    m_data->ticLabelsEnabled = enabled;
  }
}
