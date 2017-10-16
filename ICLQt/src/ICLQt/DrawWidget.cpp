/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DrawWidget.cpp                         **
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

#include <ICLQt/DrawWidget.h>
#include <ICLQt/PaintEngine.h>
#include <ICLCore/ImgBase.h>
#include <ICLQt/GLImg.h>

#include <QtCore/QMutexLocker>

using std::string;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace qt{



    /// internally used classes
    class ICLDrawWidget::State{
      public:
      // {{{  struct
      State(){}
      bool aa;             // antializing on
      bool rel;            // relative or absolut coords
      Rect32f rect;           // current image rect
      Size32f size;           // current drawing widget size
      Size32f imsize;         // current image size
      //    unsigned char bg[4]; // background color
      QSizeF symsize;
      float textAngle; // text orientation
      float defaultFontSize;   // default font size that is use instead of 0


      //    float transform[6]; not implemented yet!
    };



    class ICLDrawWidget::DrawCommand{
    public:
      virtual ~DrawCommand(){}
      virtual void exec(PaintEngine *e, State* s){
        (void)e; (void)s;
        printf("drawCommand :: exec \n");
      }
    };



    static void clear_queue(std::vector<ICLDrawWidget::DrawCommand*> &q){
      for(std::vector<ICLDrawWidget::DrawCommand*>::iterator it = q.begin();it!= q.end();++it){
        delete (*it);
      }
      q.clear();
    }

    // {{{ abstract commands (intelligent, 2f, 3f 4f)

    class IntelligentDrawCommand : public ICLDrawWidget::DrawCommand{

    protected:
      Point32f tP(float x, float y, ICLDrawWidget::State *s, bool forRect=false){
        return Point32f(tX(x,s,forRect),tY(y,s,forRect));
      }
      Point32f tP(const Point32f &p, ICLDrawWidget::State *s,bool forRect=false){
        return Point32f(tX(p.x,s,forRect),tY(p.y,s,forRect));
      }
      Rect32f tR(float x, float y, float w, float h, ICLDrawWidget::State *s){
        Point32f a = tP(x,y,s,true);
        Point32f b = tP(x+w,y+h,s,true);
        return Rect32f(a,Size32f(b.x-a.x,b.y-a.y));
      }
      Size32f tS(float w, float h, ICLDrawWidget::State *s){
        if(s->rel)
          return Size32f(w*s->rect.width,h*s->rect.height);
        else
          return Size32f((w*s->rect.width)/s->imsize.width,
                         (h*s->rect.height)/s->imsize.height);
      }
      float tX(float x, ICLDrawWidget::State *s, bool forRect=false){
        if(s->rel){
          return tmb(x,s->rect.width,s->rect.x);
        }else{
          if(!forRect) x+=0.5;
          return t(x, s->imsize.width, s->rect.width, 0, s->rect.x);
        }
      }
      float tY(float y, ICLDrawWidget::State *s, bool forRect=false){
        if(s->rel){
          return tmb(y,s->rect.height,s->rect.y);
        } else {
          if(!forRect) y+=0.5;
          return t(y, s->imsize.height, s->rect.height, 0, s->rect.y);
        }
      }
    private:
      inline float tmb(float x, float m, float b){
        return m * x + b;
      }
      inline float t(float x, float dx, float dy, float xmin, float ymin){
        return (dy / dx) * (x - xmin) + ymin;
      }
    };


    class DrawCommand2F : public IntelligentDrawCommand{


    public:
      DrawCommand2F(float a, float b):m_fA(a),m_fB(b){}
    protected:
      float m_fA, m_fB;
    };



    class DrawCommand3F : public DrawCommand2F{

    public:
      DrawCommand3F(float a, float b, float c):
        DrawCommand2F(a,b),m_fC(c){}
    protected:
      float m_fC;
    };



    class DrawCommand4F : public DrawCommand3F{


    public:
      DrawCommand4F(float a, float b, float c, float d):
        DrawCommand3F(a,b,c),m_fD(d){}
    protected:
      float m_fD;
    };



    class DrawCommand5F : public DrawCommand4F{


    public:
      DrawCommand5F(float a, float b, float c, float d, float e):
        DrawCommand4F(a,b,c,d),m_fE(e){}
    protected:
      float m_fE;
    };



    class DrawCommand6F : public DrawCommand4F{


    public:
      DrawCommand6F(float a, float b, float c, float d,float e,float f):
        DrawCommand4F(a,b,c,d),m_fE(e),m_fF(f){}
    protected:
      float m_fE;
      float m_fF;
    };



    class DrawCommand8F : public DrawCommand6F{


    public:
      DrawCommand8F(float a, float b, float c, float d,float e,float f,float g, float h):
        DrawCommand6F(a,b,c,d,e,f),m_fG(g),m_fH(h){}
    protected:
      float m_fG;
      float m_fH;
    };





    // {{{ geometric commands ( point, line, rect, ellipse )

    class PointCommand : public DrawCommand2F{


      public:
      PointCommand(float x, float y):DrawCommand2F(x,y){};
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->point(tP(m_fA,m_fB,s));
      }
    };



    class PointsCommand : public IntelligentDrawCommand{


      public:
      PointsCommand(const std::vector<Point> &pts, int xfac, int yfac, bool connectPoints, bool closeLoop):
        pts(pts),xfac(xfac),yfac(yfac),connectPoints(connectPoints),closeLoop(closeLoop){}

      //PointsCommand(const std::vector<Point32f> &pts32f, int xfac, int yfac, bool connectPoints, bool closeLoop):
      //  pts32f(pts32f),xfac(xfac),yfac(yfac),connectPoints(connectPoints),closeLoop(closeLoop){}

      // This could be used for Point32f as well ...

      template<class Pt>
      void exec_t(PaintEngine *e, ICLDrawWidget::State *s, const std::vector<Pt> &pts){
        if(!pts.size()) return;
        if(connectPoints){
          if(pts.size() == 2){
            e->line(tP(pts[0].x/xfac,pts[0].y/yfac,s),tP(pts[1].x/xfac,pts[1].y/yfac,s));
            e->point(tP(pts[0].x/xfac,pts[0].y/yfac,s));
            e->point(tP(pts[1].x/xfac,pts[1].y/yfac,s));
            return;
          }
          ///  x----------x---------x------------x-------------x
          ///  0          1         2            3             4  n=5
          if(xfac==1 && yfac==1){
            for(unsigned int i=1;i<pts.size();i++){
              e->line(tP(pts[i-1].x,pts[i-1].y,s),tP(pts[i].x,pts[i].y,s));
              e->point(tP(pts[i].x,pts[i].y,s));
            }
            if(closeLoop){
              e->line(tP(pts[pts.size()-1].x,pts[pts.size()-1].y,s),tP(pts[0].x,pts[0].y,s));
            }
          }else{
            for(unsigned int i=1;i<pts.size();i++){
              e->line(tP(float(pts[i-1].x)/xfac,float(pts[i-1].y)/yfac,s),tP(float(pts[i].x)/xfac,float(pts[i].y)/yfac,s));
              e->point(tP(float(pts[i].x)/xfac,float(pts[i].y)/yfac,s));
            }
            if(connectPoints){
              e->line(tP(float(pts[pts.size()-1].x)/xfac,float(pts[pts.size()-1].y)/yfac,s),tP(float(pts[0].x)/xfac,float(pts[0].y)/yfac,s));
            }
          }
        }else{
          if(xfac==1 && yfac==1){
            for(unsigned int i=0;i<pts.size();i++){
              e->point(tP(pts[i].x,pts[i].y,s));
            }
          }else{
            for(unsigned int i=0;i<pts.size();i++){
              e->point(tP(float(pts[i].x)/xfac,float(pts[i].y)/yfac,s));
            }
          }
        }
      }

      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        if(pts.size()) exec_t(e,s,pts);
        else exec_t(e,s,pts32f);
      }
      std::vector<Point> pts;
      std::vector<Point32f> pts32f;
      int xfac;
      int yfac;
      bool connectPoints;
      bool closeLoop;
    };




    class PolygonCommand : public IntelligentDrawCommand{


      public:
      PolygonCommand(const std::vector<Point32f> &pts):pts(pts),center(0,0){
        if(pts.size()){
          for(unsigned int i=0;i<pts.size();++i){
            center += pts[i];
          }
          center *= 1.0/pts.size();
        }
      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        if(pts.size()< 3) return;

        float colorSave[4];
        e->getColor(colorSave);
        e->color(0,0,0,0);

        Point c = tP(center.x,center.y,s);

        const unsigned int n = pts.size();
        for(unsigned int i=0; i<n;++i){
          unsigned int next =i+1;
          if(next == n) next = 0;
          const Point32f &a = pts[i];
          const Point32f &b = pts[next];
          Point A = tP(a.x,a.y,s);
          Point B = tP(b.x,b.y,s);
          e->color(0,0,0,0);
          e->triangle(A,B,c);
          e->color(colorSave[0],colorSave[1],colorSave[2],colorSave[3]);
          e->line(A,B);
        }


      }
      std::vector<Point32f> pts;
      Point32f center;
    };




   class Points32fCommand : public IntelligentDrawCommand{


      public:
      Points32fCommand(const std::vector<Point32f> &pts, bool connectPoints, bool closeLoop):
        pts(pts),connectPoints(connectPoints),closeLoop(closeLoop){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        if(!pts.size()) return;
        if(connectPoints){
          if(pts.size() == 2){
            e->line(tP(pts[0].x,pts[0].y,s),tP(pts[1].x,pts[1].y,s));
            e->point(tP(pts[0].x,pts[0].y,s));
            e->point(tP(pts[1].x,pts[1].y,s));
            return;
          }
          ///  x----------x---------x------------x-------------x
          ///  0          1         2            3             4  n=5

          for(unsigned int i=1;i<pts.size();i++){
            e->line(tP(pts[i-1].x,pts[i-1].y,s),tP(pts[i].x,pts[i].y,s));
            e->point(tP(pts[i].x,pts[i].y,s));
          }
          if(closeLoop){
            e->line(tP(pts[pts.size()-1].x,pts[pts.size()-1].y,s),tP(pts[0].x,pts[0].y,s));
          }
        }else{
          for(unsigned int i=0;i<pts.size();i++){
            e->point(tP(pts[i].x,pts[i].y,s));
          }
        }
      }
      std::vector<Point32f> pts;
      bool connectPoints;
      bool closeLoop;
    };





    class LineCommand : public DrawCommand4F{


    public:
      LineCommand(float x1, float y1, float x2, float y2):
      DrawCommand4F(x1,y1,x2,y2){
      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->line(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s));
      }
    };




    class ArrowCommand : public IntelligentDrawCommand{

      Point32f a,b;
      float c; // capsize
    public:
      ArrowCommand(float ax, float ay, float bx, float by, float capsize):
        a(ax,ay),b(bx,by),c(capsize){
      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->line(tP(a.x,a.y,s),tP(b.x,b.y,s));
        Point32f v = b-a;
        Point32f w(b.y-a.y,-(b.x-a.x));
        w = w*(0.5*c/w.norm()); // normalized v2 to length capsize
        v = v*(c/v.norm());
        e->triangle(tP(b.x,b.y,s),tP(b.x+w.x-v.x,b.y+w.y-v.y,s),tP(b.x-w.x-v.x,b.y-w.y-v.y,s));
      }
    };




    class RectCommand : public DrawCommand4F{

    public:
      RectCommand(float x, float y, float w, float h):
        DrawCommand4F(x,y,w,h){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->rect(tR(m_fA,m_fB,m_fC,m_fD,s));
      }
    };



    class TriangleCommand : public DrawCommand6F{

    public:
      TriangleCommand(float x1, float y1, float x2, float y2,float x3,float y3):
        DrawCommand6F(x1,y1,x2,y2,x3,y3){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->triangle(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s),tP(m_fE,m_fF,s));
      }
    };



    class QuadCommand : public DrawCommand8F{

    public:
      QuadCommand(float x1, float y1, float x2, float y2,float x3,float y3,float x4,float y4):
        DrawCommand8F(x1,y1,x2,y2,x3,y3,x4,y4){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->quad(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s),tP(m_fE,m_fF,s),tP(m_fG,m_fH,s));
      }
    };




    class EllipseCommand : public DrawCommand4F{

    public:
      EllipseCommand(float x, float y, float w, float h):
        DrawCommand4F(x,y,w,h){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->ellipse(tR(m_fA,m_fB,m_fC,m_fD,s));
      }
    };


    class GridCommand : public IntelligentDrawCommand{

      std::vector<Point32f> pts;
      int nx,ny;
      bool rowMajor;
    public:
      GridCommand(const Point32f *points, int nx, int ny, bool rowMajor):
        pts(points,points+nx*ny),nx(nx),ny(ny),rowMajor(rowMajor){
      }


      struct AnyData{
        std::vector<Point32f> &p;
        int wh;
        AnyData(std::vector<Point32f> & p, int wh):p(p),wh(wh){}
      };
      struct RowMajorData : public AnyData{
        RowMajorData(std::vector<Point32f> &p, int w):AnyData(p,w){}
        const Point32f &operator()(int x, int y) const { return p[x+wh*y]; }
      };
      struct ColMajorData : public AnyData{
        ColMajorData(std::vector<Point32f> &p, int h):AnyData(p,h){}
        const Point32f &operator()(int x, int y) const { return p[y+wh*x]; }
      };


      template<class Data>
      void exec_t(PaintEngine *e, ICLDrawWidget::State *s,const Data &data){
        for(int x=0;x<nx-1;++x){
          for(int y=0;y<ny-1;++y){
            Point32f a = tP(data(x,y),s);
            Point32f b = tP(data(x+1,y),s);
            Point32f c = tP(data(x,y+1),s);

            e->line(a,b);
            e->line(a,c);
          }
        }
        for(int x=0;x<nx-1;++x){
          Point32f a = tP(data(x,ny-1),s);
          Point32f b = tP(data(x+1,ny-1),s);
          e->line(a,b);
        }
        for(int y=0;y<ny-1;++y){
          Point32f a = tP(data(nx-1,y),s);
          Point32f b = tP(data(nx-1,y+1),s);
          e->line(a,b);
        }
      }

      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        if(rowMajor){
          exec_t(e,s,RowMajorData(pts,nx));
        }else{
          exec_t(e,s,ColMajorData(pts,ny));
        }
      }
    };


    class SymCommand : public IntelligentDrawCommand{

    public:
      SymCommand(float x, float y, ICLDrawWidget::Sym s):
        m_fX(x),m_fY(y),m_eS(s){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        Rect r(tP(m_fX,m_fY,s),tS(s->symsize.width(),s->symsize.height(),s));
        r.x-=r.width/2;
        r.y-=r.height/2;
        switch(m_eS){
          case ICLDrawWidget::symRect:
            e->rect(r);
            break;
          case ICLDrawWidget::symCircle:
            e->ellipse(r);
            break;
          case ICLDrawWidget::symCross:
            e->line(r.ul(), r.lr());
            e->line(r.ll(), r.ur());
            break;
          case ICLDrawWidget::symPlus:
            e->line( Point( (r.x+r.right())/2, r.y ),
                     Point( (r.x+r.right())/2, r.bottom() ) );
            e->line( Point( r.x, (r.y+r.bottom())/2 ),
                     Point( r.right(), (r.y+r.bottom())/2 ) );
            break;
          case ICLDrawWidget::symTriangle:
            e->line( Point( (r.x+r.right())/2, r.y ), r.ul() );
            e->line( Point( (r.x+r.right())/2, r.y ), r.lr() );
            e->line( r.ll(), r.lr() );
            break;
        }
      }
      float m_fX, m_fY;
      ICLDrawWidget::Sym m_eS;
    };



    /** old
    class ImageCommand : public DrawCommand4F{


    public:
      ImageCommand(ImgBase *image, float x, float y, float w, float h):
        DrawCommand4F(x,y,w,h), m_poImage(0){
        image->deepCopy(&m_poImage);
      }
      virtual ~ImageCommand(){
        if(m_poImage)delete m_poImage;
      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->image(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),m_poImage,PaintEngine::Justify);
      }
      ImgBase *m_poImage;
    };


    **/
    class ImageCommand : public DrawCommand4F{


    public:
      ImageCommand(ImgBase *image, float x, float y, float w, float h):
        DrawCommand4F(x,y,w,h),image(image){
      }
      virtual ~ImageCommand(){

      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)e;
        //Rect r(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s));
        image.draw2D(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),s->size);
      }
      GLImg image;
    };



    class ImageQuadrangleCommand : public IntelligentDrawCommand{

      float a[2],b[2],c[2],d[2];
    public:
      ImageQuadrangleCommand(const ImgBase *image, const float a[2],const float b[2],
                   const float c[2],const float d[2]):image(image){
        for(int i=0;i<2;++i){
          this->a[i] = a[i];
          this->b[i] = b[i];
          this->c[i] = c[i];
          this->d[i] = d[i];
        }
      }
      virtual ~ImageQuadrangleCommand(){

      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)e;
        const float da[2] = { tX(a[0],s), tY(a[1],s) };
        const float db[2] = { tX(b[0],s), tY(b[1],s) };
        const float dc[2] = { tX(c[0],s), tY(c[1],s) };
        const float dd[2] = { tX(d[0],s), tY(d[1],s) };

        image.draw2D(da,db,dc,dd, s->size);
      }
      GLImg image;
    };




    class TextCommand : public DrawCommand4F{


    public:
      TextCommand(std::string text, float x, float y, float w, float h, float fontsize):
        DrawCommand4F(x,y,w,h),text(text),fontsize(fontsize){
      }
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        float fs = fontsize ? fontsize : s->defaultFontSize;

        int oldFontSize = e->getFontSize();
        e->fontsize(fs < 0 ? (-fs * float(s->rect.height)/float(s->imsize.height)) : fs);
        if(m_fC == -1 || m_fD == -1){
          e->text(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),text,PaintEngine::NoAlign, s->textAngle);
        }else{
          e->text(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),text,PaintEngine::Justify, s->textAngle);
        }
        e->fontsize(oldFontSize);
      }
    protected:
      string text;
      float fontsize;
    };




    // {{{ state commands( (no)edge, (no)fill, abs, rel, clear, setimagesize)



    class EdgeCommand : public DrawCommand4F{

    public:
      EdgeCommand(float r, float g, float b, float alpha):
        DrawCommand4F(r,g,b,alpha){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)s;
        e->color((int)m_fA,(int)m_fB,(int)m_fC,(int)m_fD);
      }
    };



    class FillCommand : public DrawCommand4F{

    public:
      FillCommand(float r, float g, float b, float alpha):
        DrawCommand4F(r,g,b,alpha){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)s;
        e->fill((int)m_fA,(int)m_fB,(int)m_fC,(int)m_fD);
      }
    };



    class NoEdgeCommand : public ICLDrawWidget::DrawCommand{

    public:
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)s;
        e->color(0,0,0,0);
      }
    };



    class NoFillCommand : public ICLDrawWidget::DrawCommand{

    public:
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)s;
        e->fill(0,0,0,0);
      }
    };



    class AbsCommand : public ICLDrawWidget::DrawCommand{


      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)e;
        s->rel = false;
      }
    };



    class RelCommand : public ICLDrawWidget::DrawCommand{


      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)e;
        s->rel = true;
      }
    };



  #if 0
    class ClearCommand : public DrawCommand4F{


    public:
      ClearCommand(float r, float g, float b, float alpha):
        DrawCommand4F(r,g,b,alpha){}

      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        float aiFill[4],aiLine[4];
        e->getFill(aiFill);
        e->getColor(aiLine);
        e->fill(m_fA,m_fB,m_fC,m_fD);
        e->color(0,0,0,0);
        e->rect(Rect32f(0,0,s->size.width,s->size.height));
        e->fill(aiFill[0],aiFill[1],aiFill[2],aiFill[3]);
        e->color(aiLine[0],aiLine[1],aiLine[2],aiLine[3]);
      }
    };


  #endif

    class SetImageSizeCommand : public ICLDrawWidget::DrawCommand{

    public:
      SetImageSizeCommand(const Size32f &s):m_oSize(s){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)e;
        s->imsize = m_oSize;
      }
    protected:
      Size32f m_oSize;
    };



    class SymSizeCommand : public ICLDrawWidget::DrawCommand{


    public:
      SymSizeCommand(float w, float h) : m_fW(w), m_fH(h){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        (void)e;
        s->symsize = QSizeF(m_fW,m_fH);
      }
    protected:
      float m_fW, m_fH;
    };




    class LineWidthCommand : public ICLDrawWidget::DrawCommand{


    public:
      LineWidthCommand(float w):m_w(w){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->linewidth(m_w);
      }
    protected:
      float m_w;
    };



    class PointSizeCommand : public ICLDrawWidget::DrawCommand{


    public:
      PointSizeCommand(float s):m_s(s){}
      virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
        e->pointsize(m_s);
      }
    protected:
      float m_s;
    };



    class TextAngleCommand : public ICLDrawWidget::DrawCommand{


    public:
      TextAngleCommand(float angle):m_angle(angle){}
      virtual void exec(PaintEngine *, ICLDrawWidget::State *s){
        s->textAngle = m_angle;
      }
    protected:
      float m_angle;
    };



    class FontSizeCommand : public ICLDrawWidget::DrawCommand{

    public:
      FontSizeCommand(float size):m_size(size){}
      virtual void exec(PaintEngine *, ICLDrawWidget::State *s){
        s->defaultFontSize = m_size;
      }
    protected:
      float m_size;
    };





    ICLDrawWidget::ICLDrawWidget(QWidget *poParent):


      ICLWidget(poParent),m_oCommandMutex(QMutex::Recursive){
      m_poState = new State();
      m_poState->aa = false;
      m_poState->rel = false;
      m_poState->rect = getImageRect();
      m_poState->size = Size(width(),height());
      m_poState->imsize = getImageSize();
      m_poState->symsize = QSizeF(5,5);

      //    memset(m_poState->bg,0,4*sizeof(unsigned char));

      m_queues[0] = new std::vector<DrawCommand*>;
      m_queues[1] = new std::vector<DrawCommand*>;

      setShowNoImageWarnings(false);

      setAutoRenderOnSetImage(false);

      m_autoResetQueue = true;
    }


    ICLDrawWidget::~ICLDrawWidget(){
      //lock();
      //reset();
      if(m_poState)delete m_poState;

      clear_queue(*m_queues[0]);
      clear_queue(*m_queues[1]);

      delete m_queues[0];
      delete m_queues[1];

      //unlock();
    }

    // {{{ commands: line, sym, rel, ...
    void ICLDrawWidget::image(ImgBase *image,float x, float y, float w, float h){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new ImageCommand(image,x,y,w,h));
    }
    void ICLDrawWidget::image(const ImgBase *image, const float a[2], const float b[2],
                              const float c[2], const float d[2]){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new ImageQuadrangleCommand(image,a,b,c,d));
    }

    void ICLDrawWidget::text(string text, float x, float y, float w, float h, float fontsize){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new TextCommand(text,x,y,w,h,fontsize));
    }

    void ICLDrawWidget::line(float x1, float y1, float x2, float y2){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new LineCommand(x1,y1,x2,y2));
    }
    void ICLDrawWidget::line(const Point32f &a, const Point32f &b){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new LineCommand(a.x,a.y,b.x,b.y));
    }
    void ICLDrawWidget::arrow(float ax, float ay, float bx, float by, float capsize){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new ArrowCommand(ax,ay,bx,by,capsize));
    }
    void ICLDrawWidget::arrow(const Point32f &a, const Point32f &b, float capsize){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new ArrowCommand(a.x,a.y,b.x,b.y,capsize));
    }
    void ICLDrawWidget::sym(float x, float y, Sym s){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new SymCommand(x,y,s));
    }
    void ICLDrawWidget::symsize(float w, float h){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new SymSizeCommand(w,h==-1? w : h));
    }

    void ICLDrawWidget::linewidth(float w){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new LineWidthCommand(w));
    }

    void ICLDrawWidget::pointsize(float s){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new PointSizeCommand(s));
    }

    void ICLDrawWidget::point(float x, float y){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new PointCommand(x,y));
    }
    void ICLDrawWidget::points(const std::vector<Point> &pts, int xfac, int yfac){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new PointsCommand(pts,xfac,yfac,false,false));
    }
    void ICLDrawWidget::points(const std::vector<Point32f> &pts){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new Points32fCommand(pts,false,false));
    }

    void  ICLDrawWidget::linestrip(const std::vector<Point> &pts, bool closeLoop, int xfac, int yfac){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new PointsCommand(pts,xfac,yfac,true,closeLoop));
    }
    void  ICLDrawWidget::linestrip(const std::vector<Point32f> &pts, bool closeLoop){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new Points32fCommand(pts,true,closeLoop));
    }

    void ICLDrawWidget::polygon(const std::vector<Point32f> &ps){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new PolygonCommand(ps));
    }

    void ICLDrawWidget::polygon(const std::vector<Point> &ps){
      polygon(std::vector<Point32f>(ps.begin(),ps.end()));
    }


    void ICLDrawWidget::grid(const Point32f *points, int nx, int ny, bool rowMajor){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new GridCommand(points,nx,ny,rowMajor));
    }

    void ICLDrawWidget::abs(){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new AbsCommand());
    }
    void ICLDrawWidget::rel(){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new RelCommand());
    }
    void ICLDrawWidget::rect(float x, float y, float w, float h){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new RectCommand(x,y,w,h));
    }
    void ICLDrawWidget::rect(const Rect &r){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new RectCommand(r.x,r.y,r.width,r.height));
    }
    void ICLDrawWidget::rect(const Rect32f &r){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new RectCommand(r.x,r.y,r.width,r.height));
    }
    void ICLDrawWidget::triangle(float x1, float y1, float x2, float y2, float x3, float y3){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new TriangleCommand(x1,y1,x2,y2,x3,y3));
    }
    void ICLDrawWidget::triangle(const Point32f &a, const Point32f &b, const Point32f &c){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new TriangleCommand(a.x,a.y,b.x,b.y,c.x,c.y));
    }
    void ICLDrawWidget::quad(float x1, float y1, float x2, float y2, float x3, float y3,float x4,float y4){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new QuadCommand(x1,y1,x2,y2,x3,y3,x4,y4));
    }
    void ICLDrawWidget::quad(const Point32f &a, const Point32f &b, const Point32f &c, const Point32f &d){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new QuadCommand(a.x,a.y,b.x,b.y,c.x,c.y,d.x,d.y));
    }
    void ICLDrawWidget::ellipse(float x, float y, float w, float h){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new EllipseCommand(x,y,w,h));
    }
    void ICLDrawWidget::ellipse(const Rect &r){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new EllipseCommand(r.x,r.y,r.width,r.height));
    }
    void ICLDrawWidget::ellipse(const Rect32f &r){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new EllipseCommand(r.x,r.y,r.width,r.height));
    }
    void ICLDrawWidget::circle(float cx, float cy, float r){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new EllipseCommand(cx-r,cy-r,2*r,2*r));
    }
    void ICLDrawWidget::circle(const Point32f &center, float radius){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new EllipseCommand(center.x-radius,center.y-radius,2*radius,2*radius));
    }
    void ICLDrawWidget::color(float r, float g, float b, float alpha){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new EdgeCommand(r,g,b,alpha));
    }
    void ICLDrawWidget::fill(float r, float g, float b, float alpha){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new FillCommand(r,g,b,alpha));
    }
    void ICLDrawWidget::nocolor(){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new NoEdgeCommand());
    }
    void ICLDrawWidget::nofill(){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new NoFillCommand());
    }
    void ICLDrawWidget::fontsize(float size){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new FontSizeCommand(size));
    }
    void ICLDrawWidget::textangle(float angle){
      QMutexLocker lock(&m_oCommandMutex);
      m_queues[0]->push_back(new TextAngleCommand(angle));
    }

  #if 0
    void ICLDrawWidget::clear(float r, float g, float b, float alpha){
      m_queues[0]->push_back(new ClearCommand(r,g,b,alpha));
    }
    void ICLDrawWidget::reset(){
      clear_queue(*m_queues[0]);
    }
  #endif



    void ICLDrawWidget::initializeCustomPaintEvent(PaintEngine *e){

      (void)e;
    }


    void ICLDrawWidget::finishCustomPaintEvent(PaintEngine *e){

      (void)e;
    }


    void ICLDrawWidget::customPaintEvent(PaintEngine *e){

      QMutexLocker lock(&m_oCommandMutex);
      //Rect r = getImageRect();
      m_poState->aa = false;
      m_poState->rel = false;
      m_poState->rect = getImageRect(true);
      m_poState->size = Size32f(width(),height());
      m_poState->imsize = getImageSize(true);
      m_poState->symsize = QSizeF(5,5);
      m_poState->textAngle = 0;
      m_poState->defaultFontSize = 10;
      e->font("Arial",8,PaintEngine::DemiBold);
      e->color(255,255,255);
      e->fill(0,0,0);
      initializeCustomPaintEvent(e);

      std::vector<DrawCommand*> &q = *m_queues[1];

      for(std::vector<DrawCommand*>::iterator it = q.begin();it!= q.end();++it){
        (*it)->exec(e,m_poState);
      }

      finishCustomPaintEvent(e);
    }



    void ICLDrawWidget::swapQueues(){
      QMutexLocker lock(&m_oCommandMutex);
      if(m_autoResetQueue){
        std::swap(m_queues[0],m_queues[1]);
        clear_queue(*m_queues[0]);
       }else{
        std::copy(m_queues[0]->begin(), m_queues[0]->end(), std::back_inserter(*m_queues[1]));
        m_queues[0]->clear();
      }
    }

    void ICLDrawWidget::setAutoResetQueue(bool on){
      m_autoResetQueue = on;
    }


    void ICLDrawWidget::resetQueue(){
      QMutexLocker lock(&m_oCommandMutex);
      clear_queue(*m_queues[0]);
      clear_queue(*m_queues[1]);
    }
    void ICLDrawWidget::draw(const VisualizationDescription &d){
      const std::vector<VisualizationDescription::Part> &parts = d.getParts();
      for(size_t i=0;i<parts.size();++i){
        switch(parts[i].type){
          case 'c': {
            VisualizationDescription::Color c = parts[i].content;
            this->color(c.r(),c.g(),c.b(),c.a());
            break;
          }
          case 'f':{
            VisualizationDescription::Color c = parts[i].content;
            this->fill(c.r(),c.g(),c.b(),c.a());
            break;
          }
          case 'r':
            this->rect(parts[i].content.as<Rect32f>());
            break;
          case 'e':
            this->ellipse(parts[i].content.as<Rect32f>());
            break;
          case 'l':{
            Rect32f r = parts[i].content.as<Rect32f>();
            this->line(r.ul(),r.lr());
            break;
          }
          case 't':{
            VisualizationDescription::Text t = parts[i].content;
            this->text(t.text,t.pos, 0);
            break;
          }
          case 'L':
            this->linewidth(parts[i].content.as<float>());
            break;
          case 'P':
            this->pointsize(parts[i].content.as<float>());
            break;
          case 'p':{
            std::vector<float> v = parts[i].content;
            Point32f *p = (Point32f*)v.data();
            this->points(std::vector<Point32f>(p,p+v.size()/2));
            break;
          }
          case 'y':{
            std::vector<float> v = parts[i].content;
            Point32f *p = (Point32f*)v.data();
            this->polygon(std::vector<Point32f>(p,p+v.size()/2));
            break;
          }
          case 'A':
            this->textangle(parts[i].content.as<float>());
            break;
          case 'F':
            this->fontsize(parts[i].content.as<float>());
            break;
          case '.':
            this->point(parts[i].content.as<Point32f>());
            break;
          case '+':
          case 'x':
          case 'o':{
            Point32f pos = parts[i].content;
            sym(pos,parts[i].type);
            break;
          }
          default:
            WARNING_LOG("unable to render VisualizationDescription::Part with type '" << parts[i].type << "'");
            break;
        }
      }
    }
  } // namespace qt
}
