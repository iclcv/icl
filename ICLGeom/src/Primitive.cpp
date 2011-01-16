#include <ICLGeom/Primitive.h>

#ifdef HAVE_QT
#include <QtGui/QFontMetrics>
#include <QtCore/QRectF>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QFont>
#include <ICLCC/CCFunctions.h>
#endif
namespace icl{
  Img8u Primitive::create_text_texture(const std::string &text,const GeomColor &color, int textSize){
#ifdef HAVE_QT
    int r = color[0];
    int g = color[1];
    int b = color[2];
    int a = color[3];
    QFont font("Arial",textSize);
    QFontMetrics m(font);
    QRectF br = m.boundingRect(text.c_str());
    //    QImage img(br.width()+2,br.height()+2,QImage::Format_ARGB32);
    // sometimes some of the right-most letters where cropped, so 
    // we add 10-extra pixels to the right hand side of the texture image
    QImage img(br.width()+12,br.height()+2,QImage::Format_ARGB32); 
    img.fill(0);
    QPainter painter(&img);
    painter.setFont(font);
    painter.setPen(QColor( r,g,b,iclMin(254,a)));
    painter.drawText(QPointF(1,img.height()-m.descent()-1),text.c_str());
    painter.end();
    
    Img8u buf(Size(img.width(),img.height()),4);   
    interleavedToPlanar(img.bits(),&buf);
    return buf;
#else
    // think of an ugly fallback implementation (maybe with this funky label image function from the IO package)
    return Img8u();
#endif
  }

  Primitive &Primitive::operator=(const Primitive &other){
    a = other.a;      
    b = other.b;
    c = other.c;
    d = other.d;
    polyData = other.polyData;
    color = other.color;
    tex = other.tex.detached();
    type = other.type;
    return *this;
  }

}
