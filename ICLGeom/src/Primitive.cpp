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
    vertexIndices = other.vertexIndices;
    normalIndices = other.normalIndices;
    color = other.color;
    tex = other.tex.detached();
    type = other.type;
    hasNormals = other.hasNormals;
    return *this;
  }


  Primitive::Primitive():
    type(nothing){
  }
    
  /// Line-Constructor
  Primitive::Primitive(int a, int b, const GeomColor &color):
    vertexIndices(2),color(color),type(line),hasNormals(false){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
  }
  /// Line-Constructor
  Primitive::Primitive(int a, int b, const GeomColor &color, int na, int nb):
    vertexIndices(2),normalIndices(2),color(color),type(line),hasNormals(true){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    normalIndices[0] = na;
    normalIndices[1] = nb;
  }

  /// Triangle constructor
  Primitive::Primitive(int a, int b, int c, const GeomColor &color):
    vertexIndices(3),color(color),type(triangle),hasNormals(false){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
  }
    
  /// Triangle constructor
  Primitive::Primitive(int a, int b, int c, const GeomColor &color,int na, int nb, int nc):
    vertexIndices(3),normalIndices(3),color(color),type(triangle),hasNormals(true){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
  }
    
  /// Quad constructor
  Primitive::Primitive(int a, int b, int c, int d,const GeomColor &color):
    vertexIndices(4),color(color),type(quad),hasNormals(false){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
  }

  /// Quad constructor
  Primitive::Primitive(int a, int b, int c, int d,const GeomColor &color, int na, int nb, int nc, int nd):
    vertexIndices(4),normalIndices(4),color(color),type(quad),hasNormals(true){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
    normalIndices[3] = nd;
  }
    
  /// texture constructor
  Primitive::Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy, scalemode mode):
    vertexIndices(4),tex(tex),type(texture),mode(mode),hasNormals(false){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    if(deepCopy) this->tex.detach();
  }

  /// texture constructor
  Primitive::Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy, scalemode mode, 
                       int na, int nb, int nc, int nd):
    vertexIndices(4),normalIndices(4),tex(tex),type(texture),mode(mode),hasNormals(true){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
    normalIndices[3] = nd;
    if(deepCopy) this->tex.detach();
  }
    
  /// Special constructor to create a texture primitive that contains 3D text
  /** There is not special TEXT-type: type remains 'texture' */
  Primitive::Primitive(int a, int b, int c, int d, const std::string &text, const GeomColor &color, 
                       int textSize, scalemode mode):
    vertexIndices(4),tex(create_text_texture(text,color,textSize)),type(texture),
    mode(mode),hasNormals(false){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
  }

  /// Special constructor to create a texture primitive that contains 3D text
  /** There is not special TEXT-type: type remains 'texture' */
  Primitive::Primitive(int a, int b, int c, int d, const std::string &text, const GeomColor &color, 
                       int textSize, scalemode mode, int na, int nb, int nc, int nd):
    vertexIndices(4),normalIndices(4),tex(create_text_texture(text,color,textSize)),type(texture),
    mode(mode),hasNormals(true){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
    normalIndices[3] = nd;
  }


  /// Creates a polygon primitive
  Primitive::Primitive(const std::vector<int> &polyData, const GeomColor &color):
    vertexIndices(polyData),type(polygon),hasNormals(false){
  }

  /// Creates a polygon primitive
  Primitive::Primitive(const std::vector<int> &polyData, const GeomColor &color, const std::vector<int> &normalIndices):
    vertexIndices(polyData),normalIndices(normalIndices),type(polygon),hasNormals(true){
  }

  /// Creates a deep copy (in particular deep copy of the texture image)
  Primitive::Primitive(const Primitive &other){
    *this = other;
  }


}
