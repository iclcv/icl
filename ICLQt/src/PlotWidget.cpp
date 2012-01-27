/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/PlotWidget.cpp                               **
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

#include <ICLQt/PlotWidget.h>
#include <ICLUtils/LinearTransform1D.h>
#include <ICLUtils/SmartArray.h>
#include <ICLUtils/Macros.h>

#include <QtGui/QPainter>

namespace icl{

  struct PlotWidget::DrawState{
    bool allowLines;
    bool allowSymbols;
    bool allowFill;
      
    float b_left;
    float b_right;
    float b_top;
    float b_bottom;
      
    bool render_symbols_as_images;

    Rect32f dynamicViewPort;
    Rect32f dataViewPort;
    
    bool zoomed;
  };


  struct PlotWidget::Data{
    struct AbstractData{
      std::string name;
      int numPoints;
      PenPtr style;
      AbstractData(const std::string &name, int numPoints):name(name),numPoints(numPoints){}
      virtual ~AbstractData(){}
      int size() const { return numPoints; }
    };
    
    struct ScatterData : public AbstractData{
      ScatterData(char sym, int num, const std::string &name, 
                  int r, int g, int b, int size, bool filled, bool connect,
                  int xStride, int yStride):
        AbstractData(name, num),xStride(xStride),yStride(yStride){
        QColor color(r,g,b);
        style = new AbstractPlotWidget::Pen(connect ? QPen(color) : QPen(Qt::NoPen),
                                            QPen(color), sym, size, 
                                            filled ? QBrush(color) : QBrush(Qt::NoBrush));
        
      }
      int xStride;
      int yStride;
      SmartArray<float> xs;
      SmartArray<float> ys;

      float xAt(int i) const { return xs[i*xStride]; }
      float yAt(int i) const { return ys[i*xStride]; }
    };
    
    struct SeriesData : public AbstractData{
      SeriesData(const PenPtr &style, int num,
                 const std::string &name, int stride):
        AbstractData(name, num), stride(stride){
        this->style = style;
      }
      int stride;
      SmartArray<float> data;
      float at(int i) const { return data[i*stride]; }
    };


    std::vector<float> ybuf;
    std::vector<float> xbuf;
    std::vector<bool> yClipBuf;
    std::vector<QPoint> polygonBuf;
    std::vector<ScatterData*> scatterData;
    std::vector<SeriesData*> seriesData;
    std::vector<QPoint> qpointBuf;
    int numUsedQPoints;
    
    int getMaxSeriesDataRowLen() const{
      int maxLineLen = 0;
      for(unsigned int i=0;i<seriesData.size();++i){
        const SeriesData &d = *seriesData[i];
        if(d.size() > maxLineLen) maxLineLen = d.size();
      }
      return maxLineLen;
    }
  };

  

  PlotWidget::PlotWidget(QWidget *parent) : AbstractPlotWidget(parent),data(new Data){

  }

  PlotWidget::~PlotWidget(){
    delete data;
  }
  
  bool PlotWidget::drawData(QPainter &p){
    const Rect32f dynViewPort = getDynamicDataViewPort();
    const Rect32f dataViewPort = getDataViewPort();

    const DrawState state = {
      getPropertyValue("enable lines").as<bool>(),
      getPropertyValue("enable symbols").as<bool>(),
      getPropertyValue("enable fill").as<bool>(),
      getPropertyValue("borders.left").as<float>(),
      getPropertyValue("borders.right").as<float>(),
      getPropertyValue("borders.top").as<float>(),
      getPropertyValue("borders.bottom").as<float>(),
      getPropertyValue("render symbols as images").as<bool>(),
      dynViewPort,
      dataViewPort,
      (dynViewPort != dataViewPort),

    };

    p.resetTransform();    
    
    if(state.zoomed){
      p.setClipping(true);
      p.setClipRect(QRect(state.b_left, state.b_top, width()-state.b_right, height()-state.b_bottom));
    }
    
    bool result = drawSeriesData(p,state)  | drawScatterData(p,state);
    
    if(state.zoomed){
      p.setClipping(false);
    }
    return result;
    
  }
  
  bool PlotWidget::drawScatterData(QPainter &p, const DrawState &state){
    const Rect32f &v = state.dynamicViewPort;
    const int w = width(), h = height();
    const Range32f xdata(v.x,v.right()), ydata(v.y,v.bottom());
    const Range32f xwin(state.b_left, w - state.b_right);
    const Range32f ywin(state.b_top, h - state.b_bottom);
    LinearTransform1D tx(xdata,xwin), ty(ydata,Range32f(ywin.maxVal, ywin.minVal));

    for(unsigned int i=0;i<data->scatterData.size();++i){
      const Data::ScatterData &s = *data->scatterData[i];
      const PenPtr &style = s.style;
      const int symSize = style->symbolSize;

      std::vector<float> &xbuf = data->xbuf;
      std::vector<float> &ybuf = data->ybuf;
      std::vector<bool> &clipBuf = data->yClipBuf;
      if((int)xbuf.size() < s.size()) xbuf.resize(s.size());
      if((int)ybuf.size() < s.size()) ybuf.resize(s.size());
      if((int)clipBuf.size() < s.size()) clipBuf.resize(s.size());

      // optimization for points: TODO!!!
      if(state.allowSymbols && style->symbolPen != Qt::NoPen && style->symbol){
        std::vector<QPoint> &qpointBuf = data->qpointBuf; 
        if((int)qpointBuf.size() < s.size()) qpointBuf.resize(s.size());
        int &numUsedQPoints = data->numUsedQPoints;
        numUsedQPoints = -1;
        for(int j=0;j<s.size();++j){
          xbuf[j] = tx(s.xAt(j));
          ybuf[j] = ty(s.yAt(j));
          if( (clipBuf[j] = xwin.contains(xbuf[j]) && ywin.contains(ybuf[j]) ) ){
            qpointBuf[++numUsedQPoints] = QPoint(xbuf[j],ybuf[j]);
          }
        }
        ++numUsedQPoints; // count is always one more!
      }else{
        /// normal version
        for(int j=0;j<s.size();++j){
          xbuf[j] = tx(s.xAt(j));
          ybuf[j] = ty(s.yAt(j));
          clipBuf[j] = xwin.contains(xbuf[j]) && ywin.contains(ybuf[j]);
        }
      }


      if(state.allowFill && style->fillBrush != Qt::NoBrush){

        if(state.allowLines){
          p.setPen(style->linePen);
        }else{
          p.setPen(Qt::NoPen);
        }
        p.setBrush(style->fillBrush);
                   
        std::vector<QPoint> &polygonBuf = data->polygonBuf;
        if((int)polygonBuf.size() < (int)s.size()){
          polygonBuf.resize(s.size());
        }
        for(int j=0;j<s.size();++j){
          polygonBuf[j] = QPoint(xbuf[j],ybuf[j]);
        }
        p.drawConvexPolygon(polygonBuf.data(), polygonBuf.size());
        
      }else if(state.allowLines && style->linePen != Qt::NoPen){
        p.setPen(style->linePen);
        for(int j=1;j<s.size();++j){
          if(clipBuf[j] ||clipBuf[j-1]){
            p.drawLine(xbuf[j-1], ybuf[j-1], xbuf[j], ybuf[j]);
          }
        } 
      }

      
      if(state.allowSymbols && style->symbolPen != Qt::NoPen){
        p.setPen(style->symbolPen);
        if('A' <= style->symbol && style->symbol <= 'Z'){
          p.setBrush(style->symbolPen.color());
        }else{
          p.setBrush(Qt::NoBrush);
        }
        switch(style->symbol){
          case '.':
            p.drawPoints(data->qpointBuf.data(), data->numUsedQPoints);
            break;
#define PLOT_WIDGET_CASE_X(X)                                           \
          case X:                                                       \
            for(int j=0;j<s.size();++j) {                               \
              if(clipBuf[j]){                                           \
                draw_symbol<X>(p,symSize,xbuf[j],ybuf[j]);              \
              }                                                         \
            }                                                           \
            break;
#define PLOT_WIDGET_CASE_XY(X,Y)                \
          case Y:                               \
            PLOT_WIDGET_CASE_X(X)

          // PLOT_WIDGET_CASE_X('.');
            PLOT_WIDGET_CASE_X('-');
            PLOT_WIDGET_CASE_X('x');
            PLOT_WIDGET_CASE_X('*');
            PLOT_WIDGET_CASE_X('+');
            PLOT_WIDGET_CASE_XY('s','S');
            PLOT_WIDGET_CASE_XY('t','T');
            PLOT_WIDGET_CASE_XY('o','O');
            PLOT_WIDGET_CASE_XY('d','D');
          default: break;
        }
#undef PLOT_WIDGET_CASE_XY
#undef PLOT_WIDGET_CASE_X
      }

    }
    return true;
  }
  
  bool PlotWidget::drawSeriesData(QPainter &p, const DrawState &state){
    if(!data->seriesData.size()) return false;

    bool clipping = p.hasClipping();
    if(!clipping) {
      p.setClipping(true);
      p.setClipRect(QRect(state.b_left, state.b_top, width()-state.b_right, height()-state.b_bottom));
    }

    const int rows = (int)data->seriesData.size();

    const Rect32f &v = state.dynamicViewPort;
    const Range32f xrange(v.x,v.right()), yrange(v.y,v.bottom());
    const Rect32f &vd = state.dataViewPort;

    LinearTransform1D A(Range32f(vd.left(), vd.right()),Range32f(0,1));
    float lFrac = A(v.left()), rFrac = A(v.right());
    float len = data->getMaxSeriesDataRowLen()-1;
    // we use (len-1) as range since with 100 bins, we have only 99 gaps
    LinearTransform1D lx(Range32f(lFrac*(len-1), rFrac*(len-1)),
                         Range32f(state.b_left,width()-state.b_right));
    LinearTransform1D ly(yrange, Range32f(height()-state.b_bottom,state.b_top)); 
#if 0
    const int firstVisibleX = iclMax(0,(int)floor(lFrac*len));
    const int lastVisibleX = iclMin((int)len, (int)ceil(rFrac*len)+1);
#endif
    //  SHOW(len << "    "  << lastVisibleX);
    
    Range32s winYRange(state.b_top, height()-state.b_bottom);
    
    for(int y=0;y<rows;++y){
      const Data::SeriesData &sd = *data->seriesData[y];
      const float *r = sd.data.get();
      const int stride = sd.stride;

      const int firstVisibleX = iclMax(0,(int)floor(lFrac*sd.size()));
      const int lastVisibleX = iclMin((int)sd.size(), (int)ceil(rFrac*sd.size())+1);


      std::vector<float> &ybuf = data->ybuf;
      std::vector<float> &xbuf = data->xbuf;
      std::vector<bool> &yClipBuf = data->yClipBuf;
      if((int)ybuf.size() < sd.size()) ybuf.resize(sd.size());
      if((int)xbuf.size() < sd.size()) xbuf.resize(sd.size());
      if(state.zoomed && (int)yClipBuf.size() < sd.size()) yClipBuf.resize(sd.size());
      
      for(int x=firstVisibleX;x<=lastVisibleX;++x){
        ybuf[x] = ly(r[stride*x]);
        xbuf[x] = lx(x);
        if( state.zoomed ){
          yClipBuf[x] = winYRange.contains(ybuf[x]);
        }
      }
      
      const PenPtr &s = sd.style;

      QPoint psFill[3];
      const bool drawFill = state.allowFill && (s->fillBrush != Qt::NoBrush);
      const bool drawLines = state.allowLines && (s->linePen != Qt::NoPen);
      const bool drawSymbols = ( state.allowSymbols && (s->symbolPen != Qt::NoPen) 
                                 && s->symbol != ' '
                                 && s->symbolSize > 0 );
      const bool symbolFilled = ( drawSymbols &&  'A' <= s->symbol  && s->symbol <= 'Z');

      if(drawFill){

        const int fillBottom = height()-state.b_bottom;
        p.setBrush(s->fillBrush);
        p.setPen(Qt::NoPen);
        // const int yMinVisible = state.b_top;
        // const int yMaxVisible = height()-state.b_bottom;
        for(int x=firstVisibleX+1;x<lastVisibleX;++x){
          const int currY = ybuf[x];//icl::clip((int)ybuf[x],yMinVisible,yMaxVisible); 
          const int lastY = ybuf[x-1]; //icl::clip((int)ybuf[x-1],yMinVisible,yMaxVisible); 
          const int minY = iclMax(currY,lastY);
          
          psFill[0] = QPoint(xbuf[x-1],lastY);
          psFill[1] = QPoint(xbuf[x],currY);
          psFill[2] = ( minY == psFill[0].y() ?
                        QPoint(xbuf[x],lastY) :
                        QPoint(xbuf[x-1],currY) );

          p.drawConvexPolygon(psFill,3);
          p.drawRect(QRect(QPoint(xbuf[x-1],minY), QPoint(xbuf[x]-1, fillBottom)));
        }
      }
      
      if(drawLines){
        if(state.zoomed){
          for(int x=firstVisibleX+1;x<lastVisibleX;++x){
            if(yClipBuf[x-1] || yClipBuf[x]){
              p.setPen(s->linePen);
              p.drawLine(QPoint(xbuf[x],ybuf[x]), QPoint(xbuf[x-1],ybuf[x-1]));
            }
          }
        }else{
          for(int x=firstVisibleX+1;x<lastVisibleX;++x){
            p.setPen(s->linePen);
            p.drawLine(QPoint(xbuf[x],ybuf[x]), QPoint(xbuf[x-1],ybuf[x-1]));
          }
        }
      }

      if(drawSymbols){
        if(state.render_symbols_as_images){
          
          const int symbolSize = s->symbolSize;
          QImage symbolImage(symbolSize*2+1,symbolSize*2+1,QImage::Format_ARGB32_Premultiplied);
          symbolImage.fill(0);
          QPainter ps(&symbolImage);
          ps.setPen(s->symbolPen);
          ps.setBrush(symbolFilled ? QBrush(s->symbolPen.color()) : QBrush(Qt::NoBrush));
#define PLOT_WIDGET_CASE_X(X)                                           \
          case X:                                                       \
            AbstractPlotWidget::draw_symbol<X>(ps,symbolSize,           \
                                               symbolSize,symbolSize);  \
            break
          
#define PLOT_WIDGET_CASE_XY(X,Y)                       \
          case Y:                                      \
            PLOT_WIDGET_CASE_X(X)
          
          switch(s->symbol){
            PLOT_WIDGET_CASE_X('.');
            PLOT_WIDGET_CASE_X('-');
            PLOT_WIDGET_CASE_X('x');
            PLOT_WIDGET_CASE_X('*');
            PLOT_WIDGET_CASE_X('+');
            PLOT_WIDGET_CASE_XY('s','S');
            PLOT_WIDGET_CASE_XY('t','T');
            PLOT_WIDGET_CASE_XY('o','O');
            PLOT_WIDGET_CASE_XY('d','D');
            case ' ': break;
            default:
              ERROR_LOG("unabled to draw symbol of unknown type " + str((int) s->symbol));
          }
#undef PLOT_WIDGET_CASE_XY
#undef PLOT_WIDGET_CASE_X

          for(int x=firstVisibleX;x<lastVisibleX;++x){
            p.drawImage(QPointF(xbuf[x]-symbolSize+0.5,ybuf[x]-symbolSize+0.5),symbolImage);
          }
          
        }else{ // normal symbol rendering ..
          const int symbolSize = s->symbolSize;
          p.setPen(s->symbolPen);
          p.setBrush(symbolFilled ? QBrush(s->symbolPen.color()) : QBrush(Qt::NoBrush));
          
#define PLOT_WIDGET_CASE_X(X)                                           \
          case X:                                                       \
            for(int x=firstVisibleX;x<lastVisibleX;++x){                \
              AbstractPlotWidget::draw_symbol<X>(p,symbolSize,xbuf[x],ybuf[x]); \
            }                                                           \
            break
          
#define PLOT_WIDGET_CASE_XY(X,Y)    \
          case Y:                   \
            PLOT_WIDGET_CASE_X(X)
          
          switch(s->symbol){
            PLOT_WIDGET_CASE_X('.');
            PLOT_WIDGET_CASE_X('-');
            PLOT_WIDGET_CASE_X('x');
            PLOT_WIDGET_CASE_X('*');
            PLOT_WIDGET_CASE_X('+');
            PLOT_WIDGET_CASE_XY('s','S');
            PLOT_WIDGET_CASE_XY('t','T');
            PLOT_WIDGET_CASE_XY('o','O');
            PLOT_WIDGET_CASE_XY('d','D');
            case ' ': break;
            default:
              ERROR_LOG("unabled to draw symbol of unknown type " + str("[") + s->symbol + "]");
          }
#undef PLOT_WIDGET_CASE_X
#undef PLOT_WIDGET_CASE_XY
          
        }
      }
    }
    if(!clipping)p.setClipping(false);

    return true;
  }


  Range32f PlotWidget::estimateDataXRange() const{
    bool haveSeries = data->seriesData.size(), haveScatter = data->scatterData.size();
    if(!haveSeries && !haveScatter){
      // no data at all
      return Range32f(0,0);
    }
    if(haveSeries && haveScatter){
      // both is given, but not viewport is set -> what to use?
      WARNING_LOG("both scatter- and series data is given, data x-viewport is missing");
      return Range32f(0,0);
    }

    if(haveSeries){
      // use max line len, 0
      return Range32f(0, data->getMaxSeriesDataRowLen()-1);
    }else{
      Range32f r = Range32f::limits();
      std::swap(r.minVal, r.maxVal);
      for(unsigned int i=0;i<data->scatterData.size();++i){
        const Data::ScatterData &d = *data->scatterData[i];
        for(int j=0;j<d.size();++j){
          r.extend(d.xAt(j));
        }
      }
      return r;
    }
  }

  Range32f PlotWidget::estimateDataYRange() const{
    bool haveSeries = data->seriesData.size(), haveScatter = data->scatterData.size();
    if(!haveSeries && !haveScatter){
      // no data at all
      return Range32f(0,0);
    }
    
    /// find range for both, scatter and series data
    Range32f r = Range32f::limits();
    std::swap(r.minVal, r.maxVal);

    if(haveSeries){
      for(unsigned int i=0;i<data->seriesData.size();++i){
        const Data::SeriesData &d = *data->seriesData[i];
        for(int j=0;j<d.size();++j){
          r.extend(d.at(j));
        }
      }
    }
    if(haveScatter){
      for(unsigned int i=0;i<data->scatterData.size();++i){
        const Data::ScatterData &d = *data->scatterData[i];
        for(int j=0;j<d.size();++j){
          r.extend(d.yAt(j));
        }
      }
    }
    return r;
  }
  
  Rect32f PlotWidget::getDataViewPort() const{
    Locker lock(this);
    Rect32f r = AbstractPlotWidget::getDataViewPort();
    if(!r.width){
      Range32f rx = estimateDataXRange();
      r.x = rx.minVal;
      r.width = rx.getLength();
    }
    if(!r.height){
      Range32f ry = estimateDataYRange();
      r.y = ry.minVal;
      r.height = ry.getLength();
    }
    return r;
  }
  
  void PlotWidget::drawLegend(QPainter &p,const Rect &where, bool horizontal){
    int num = data->scatterData.size() + data->seriesData.size();
    if(!num) return;
    
    std::vector<std::string> rowNames(num);
    std::vector<PenPtr> rowStyles(num);
    for(unsigned int i=0;i<data->seriesData.size();++i){
      rowNames[i] = data->seriesData[i]->name;
      rowStyles[i] = data->seriesData[i]->style;
    }
    const int offs = data->seriesData.size();
    for(unsigned int i=0;i<data->scatterData.size();++i){
      rowNames[offs+i] = data->scatterData[i]->name;
      rowStyles[offs+i] = data->scatterData[i]->style;
    }

    drawDefaultLedgend(p,where,horizontal,rowNames,rowStyles);
  }

  void PlotWidget::clearSeriesData(){
    Locker lock(this);
    for(unsigned int i=0;i<data->seriesData.size();++i){
      delete data->seriesData[i];
    }
    data->seriesData.clear();
  }
  
  void PlotWidget::clearScatterData(){
    Locker lock(this);
    for(unsigned int i=0;i<data->scatterData.size();++i){
      delete data->scatterData[i];
    }
    data->scatterData.clear();
  }

  /// adds series data
  void PlotWidget::addSeriesData(const float *data, int len, 
                                 const AbstractPlotWidget::PenPtr &style,
                                 const std::string &name, int stride, bool deepCopyData, bool passOwnerShip){
    Data::SeriesData *s = new Data::SeriesData(style, len, name, deepCopyData?1:stride);

    if(deepCopyData){
      s->data = new float[len];
      for(int i=0;i<len;++i){
        s->data[i] = data[i*stride];
      }
    }else{
      s->data = SmartArray<float>(const_cast<float*>(data), passOwnerShip);
    }
    Locker lock(this);
    this->data->seriesData.push_back(s);
  
  }
  
  void PlotWidget::addScatterData(char sym, const float *xs, const float *ys, int num, 
                                  const std::string &name, int r, int g, int b, int size, bool filled,
                                  int xStride, int yStride, bool connectingLine,
                                  bool deepCopyData, bool passDataOwnerShip){
    Data::ScatterData *s = new Data::ScatterData(sym, num, name, r,g,b, size, filled, connectingLine, 
                                                 deepCopyData?1:xStride, deepCopyData?1:yStride);
    if(deepCopyData){
      s->xs = new float[num];
      s->ys = new float[num];
      for(int i=0;i<num;++i){
        s->xs[i] = xs[i*xStride];
        s->ys[i] = ys[i*yStride];
      }
    }else{
      s->xs = SmartArray<float>(const_cast<float*>(xs), passDataOwnerShip);
      s->ys = SmartArray<float>(const_cast<float*>(ys), passDataOwnerShip);
    }
    Locker lock(this);
    data->scatterData.push_back(s);
  }


  void PlotWidget::clear() {
    clearAnnotations();
    clearSeriesData(); 
    clearScatterData(); 
  }
}
 


