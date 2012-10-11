/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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
#include <ICLUtils/SmartArray.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl{
  namespace qt{
    
    
    struct PlotWidget::Data{
      struct Buffer : SmartArray<float>{
        Buffer(){}
        Buffer(int len):SmartArray<float>(new float[len]), len(len), used(false){}
        int len;
        bool used;
      };
      std::vector<Buffer> buffers;
      
      void freeBuffers(){
        for(unsigned int i=0;i<buffers.size();++i){
          buffers[i].used = false;
        }
        if(buffers.size() > 20) buffers.resize(20);
      }
      
      float *getBuffer(int len){
        for(unsigned int i=0;i<buffers.size();++i){
          if(!buffers[i].used && buffers[i].len >= len){
            buffers[i].used = true;
            return buffers[i].get();
          }
        }
        buffers.push_back(Buffer(len));
        buffers.back().used = true;
       return buffers.back().get();
      }
      
      
      AbstractPlotWidget::Pen state;
      std::string label;
    };
    
    PlotWidget::PlotWidget(QWidget *parent):
      LowLevelPlotWidget(parent),m_data(new Data){
  
      clear();
    }
    
    PlotWidget::~PlotWidget(){
      delete m_data;
    }
    
    void PlotWidget::clear() {
      LowLevelPlotWidget::clear();
      m_data->freeBuffers();
      m_data->state.linePen = QPen(QColor(255,0,0));
      m_data->state.symbolPen = QPen(QColor(255,0,0));
      m_data->state.fillBrush = Qt::NoBrush;
      m_data->state.symbol = ' ';
      m_data->state.symbolSize = 5;
    }
    void PlotWidget::label(const std::string &l){
      m_data->label = l;
    }
    
    void PlotWidget::color(int r, int g, int b, int a){
      m_data->state.linePen.setColor(a==255 ? QColor(r,g,b) : QColor(r,g,b,a));
      m_data->state.symbolPen.setColor(a==255 ? QColor(r,g,b) : QColor(r,g,b,a));
    }
    void PlotWidget::pen(const QPen &pen){
      m_data->state.linePen = m_data->state.symbolPen = pen;
    }  
  
    void PlotWidget::fill(int r, int g, int b, int a){
      m_data->state.fillBrush = QBrush(a==255 ? QColor(r,g,b) : QColor(r,g,b,a));
    }
    void PlotWidget::brush(const QBrush &brush){
      m_data->state.fillBrush = brush;
    }
  
    void PlotWidget::sym(char s){
      m_data->state.symbol = s;
    }
    
    void PlotWidget::linewidth(float width){
      m_data->state.linePen.setWidthF(width);
      m_data->state.symbolPen.setWidthF(width);
    }
    
    void PlotWidget::symsize(float size){
      m_data->state.symbolSize = size;
    }
    
    template<class T>
    void PlotWidget::scatter(const T *xs, const T *ys, int num, int xStride, int yStride, bool connect){
      float *b = m_data->getBuffer(2*num);
      float *startB = b;
      float *endB = b + 2*num;
      while(b<endB){
        *b++ = *xs;
        *b++ = *ys;
        xs += xStride;
        ys += yStride;
      }
      const QColor c = m_data->state.symbolPen.color();
      
      addScatterData((m_data->state.symbol == ' ' && !connect) ? '.' : m_data->state.symbol,
                     startB, startB+1, num, m_data->label,
                     c.red(), c.green(), c.blue(),
                     m_data->state.symbolSize,
                     connect, 2, 2, false, false, false);
    }
  
  
    template<class T>
    void PlotWidget::series(const T *data, int num, int stride){
      float *b = m_data->getBuffer(num);
      if(stride ==1){
        std::copy(data,data+num, b);
      }else{
        for(int i=0;i<num;++i){
          b[i] =  data[i*stride];
        }
      }
      addSeriesData(b,num,new AbstractPlotWidget::Pen(m_data->state), m_data->label, 1, false, false);
    }
  
    template<class T>
    void PlotWidget::bars(const T *data, int num, int stride){
      float *b = m_data->getBuffer(num);
      if(stride ==1){
        std::copy(data,data+num, b);
      }else{
        for(int i=0;i<num;++i){
          b[i] =  data[i*stride];
        }
      }
      addBarPlotData(b,num,new AbstractPlotWidget::Pen(m_data->state), m_data->label, 1, false, false);
    }
  
    
  #define ICL_INSTANTIATE_DEPTH(D)                                        \
    template void PlotWidget::bars<icl##D>(const icl##D*,int,int);        \
    template void PlotWidget::series<icl##D>(const icl##D*,int,int);      \
    template void PlotWidget::scatter<icl##D>(const icl##D*, const icl##D*, int,int,int,bool);
    
    ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
    
   
    void PlotWidget::line(const Point32f &a, const Point32f &b){
      const float data[] = { a.x, a.y, b.x, b.y };
      addAnnotations('l',data, 1, m_data->state.linePen, Qt::NoBrush);
    }

    void PlotWidget::draw(const VisualizationDescription &d){
      const std::vector<VisualizationDescription::Part> &parts = d.getParts();
      for(size_t i=0;i<parts.size();++i){
        switch(parts[i].type){
          case 'c': {
            VisualizationDescription::Color c = parts[i].content;
            this->color(c.r,c.g,c.b,c.a);
            break;
          }
          case 'f':{
            VisualizationDescription::Color c = parts[i].content;
            this->fill(c.r,c.g,c.b,c.a);
            break;
          }
          case 'r':
            this->rect(parts[i].content.as<Rect32f>());
            break;
          case 'e':{
            const Rect32f r = parts[i].content.as<Rect32f>();
            const float cx = r.x+r.width/2, cy = r.y+r.height/2;
            const float rx = r.width/2, ry = r.height/2;
            
            std::vector<Point32f> ps(100,Point32f(cx,cy));
            for(float i=0;i<100;++i){
              float a = i/100.0f * 2.0f * M_PI;
              ps[i].x += cos(a) * rx;
              ps[i].y += sin(a) * ry;
            }
            this->linestrip(ps);
            break;
          }
          case 'l':{
            Rect32f r = parts[i].content.as<Rect32f>();
            this->line(r.ul(),r.lr());
            break;
          }
          case 't':{
            VisualizationDescription::Text t = parts[i].content;
            this->text(t.pos,t.text);
            break;
          }
          case '+':
          case 'x':
          case 'o':{
            Point32f pos = parts[i].content;
            this->sym(parts[i].type);
            this->scatter(&pos.x,&pos.y,1,2,2);
            break;
          }
          default:
            WARNING_LOG("unable to render VisualizationDescription::Part with type '" << parts[i].type << "'");
            break;
        }
      }
    }
    
    void PlotWidget::linestrip(const std::vector<Point32f> &ps, bool closedLoop){
      linestrip(&ps[0].x, &ps[0].y, ps.size(), closedLoop,2);
    }
    void PlotWidget::linestrip(const std::vector<Point> &ps, bool closedLoop){
      linestrip(ps.data(), ps.size(), closedLoop);
    }
    void PlotWidget::linestrip(const Point32f *ps, int num, bool closedLoop){
      linestrip(&ps[0].x, &ps[0].y, num, closedLoop, 2);
    }
  
    void PlotWidget::linestrip(const Point *ps, int num, bool closedLoop){
      std::vector<Point32f> data(num + !!closedLoop);
      std::copy(ps,ps+num,data.begin());
      if(closedLoop) data.back() = *ps;
      addAnnotations('L',&data[0].x, data.size(), m_data->state.linePen, Qt::NoBrush, m_data->label, '\0');
    }
    void PlotWidget::linestrip(const float *xs, const float *ys, int num, bool closedLoop, int stride){
      std::vector<Point32f> data(num + !!closedLoop);
      for(int i=0;i<num;++i){
        data[i] = Point32f(xs[i*stride], ys[i*stride]);
      }
      if(closedLoop) data.back() = Point32f(xs[0], ys[0]);
      addAnnotations('L',&data[0].x, data.size(), m_data->state.linePen, Qt::NoBrush);
    }
    
    
    void PlotWidget::rect(const Point32f &ul, const Point32f &lr){
      rect(Rect32f(ul.x,ul.y, lr.x-ul.x, lr.y-ul.y));
    }
    void PlotWidget::rect(const Rect &r){
      rect(Rect32f(r.x,r.y,r.width,r.height));
    }
    void PlotWidget::rect(const Rect32f &r){
      addAnnotations('r',&r.x, 1,m_data->state.linePen, m_data->state.fillBrush);
    }
    void PlotWidget::rect(float x, float y, float w, float h){
      rect(Rect32f(x,y,w,h));
    }
    
    void PlotWidget::circle(const Point32f &c, float r){
      circle(c.x,c.y,r);
    }
    void PlotWidget::circle(float cx, float cy, float r){
      const float d[] = {cx,cy,r};
      addAnnotations('c',d,1,m_data->state.linePen, m_data->state.fillBrush);
    }
    
    void PlotWidget::text(float x, float y, const std::string &text){
      this->text(Point32f(x,y),text);
    }
    void PlotWidget::text(const Point32f &p, const std::string &text){
      addAnnotations('t',&p.x,1,m_data->state.linePen, m_data->state.fillBrush, text, "\n");
    }
    
    void PlotWidget::grid(int nX, int nY, const float *xs, const float *ys, int stride){
      const int dim = nX * nY;
      float *b = m_data->getBuffer(dim*2 + 2);
      *b++ = nX;
      *b++ = nY;
      for(int i=0;i<dim;++i){
        b[2*i] = xs[i*stride];
        b[2*i+1] = ys[i*stride];
      }
      b -= 2;
      addAnnotations('g', b, 1, m_data->state.linePen, Qt::NoBrush);
    }
    
    void PlotWidget::grid(const Array2D<Point> &data){
      const int *xs = &data(0,0).x;
      const int *ys = &data(0,0).y;
      const int stride = 2;
      
      const int dim = data.getDim();
      float *b = m_data->getBuffer(dim*2 + 2);
      *b++ = data.getWidth();
      *b++ = data.getHeight();
      for(int i=0;i<dim;++i){
        b[2*i] = xs[i*stride];
        b[2*i+1] = ys[i*stride];
      }
      addAnnotations('g', b-2, 1, m_data->state.linePen, Qt::NoBrush);
      
    }
  
  } // namespace qt
}
