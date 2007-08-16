#ifndef PAINT_ENGINE_H
#define PAINT_ENGINE_H

#include <stdio.h>
#include <iclPoint.h>
#include <iclSize.h>
#include <iclRect.h>
#include <iclTypes.h>
#include <string>
#include <QImage>

namespace icl{

  /** \cond */
  class ImgBase;
  /** \endcond */  
  
  /// pure virtual Paint engine interface \ingroup UNCOMMON
  class PaintEngine{
    public:
    virtual ~PaintEngine(){}
    enum AlignMode {NoAlign, Centered, Justify};
    enum TextWeight {Light, Normal, DemiBold, Bold, Black};
    enum TextStyle {StyleNormal, StyleItalic, StyleOblique };

    virtual void color(int r, int g, int b, int a=255)=0;
    virtual void fill(int r, int g, int b, int a=255)=0;
    virtual void fontsize(int size)=0;
    virtual void font(std::string name, int size = -1, TextWeight weight = Normal, TextStyle style = StyleNormal)=0;

    virtual void line(const Point &a, const Point &b)=0;
    virtual void point(const Point &p)=0;
    virtual void image(const Rect &r,ImgBase *image, AlignMode mode = Justify)=0;
    virtual void image(const Rect &r,const QImage &image, AlignMode mode = Justify)=0;
    virtual void rect(const Rect &r)=0;
    virtual void triangle(const Point &a, const Point &b, const Point &c)=0;
    virtual void quad(const Point &a, const Point &b, const Point &c, const Point &d)=0;
    virtual void ellipse(const Rect &r)=0;
    virtual void text(const Rect &r, const std::string text, AlignMode mode = Centered)=0;

    /// brightness-constrast intensity adjustment (for images only)
    virtual void bci(int brightness=0, int contrast=0, int intensity=0)=0;
    virtual void bciAuto()=0;
    
    virtual void getColor(int *piColor)=0;
    virtual void getFill(int *piColor)=0;

  };
}// namespace

#endif

