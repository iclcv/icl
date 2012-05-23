/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/Primitive.cpp                              **
** Module : ICLGeom                                                **
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

#ifdef HAVE_QT
#include <QtGui/QFontMetrics>
#include <QtCore/QRectF>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QFont>
#include <ICLCC/CCFunctions.h>
#endif

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <ICLGeom/Primitive.h>
#include <ICLGeom/SceneObject.h>


namespace icl{

  Img8u TextPrimitive::create_texture(const std::string &text,const GeomColor &color, int textSize){
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
    painter.setPen(QColor( b,g,r,iclMin(254,a)));
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


  static void gl_color(const Primitive::RenderContext &ctx, int vertexIndex, bool condition=true){
    if(condition) glColor4fv(ctx.vertexColors[vertexIndex].data());
  }
  static void gl_vertex(const Primitive::RenderContext &ctx, int vertexIndex){
    glVertex3fv(ctx.vertices[vertexIndex].data());
  }
  static void gl_normal(const Primitive::RenderContext &ctx, int normalIndex){
    if(normalIndex >= 0) glNormal3fv(ctx.normals[normalIndex].data());
  }
  static void gl_auto_normal(const Primitive::RenderContext &ctx, int a, int b, int c, bool condition=true){
    if(!condition) return;
    const Vec &va = ctx.vertices[a];
    const Vec &vb = ctx.vertices[b];
    const Vec &vc = ctx.vertices[c];
    
    glNormal3fv(normalize(cross(va-vc,vb-vc)).data());
  }

  void LinePrimitive::render(const Primitive::RenderContext &ctx){
    GLboolean lightWasOn = true;
    glGetBooleanv(GL_LIGHTING,&lightWasOn);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    glColor4fv(color.data());

    for(int j=0;j<2;++j){
      gl_color(ctx,i(j),ctx.lineColorsFromVertices);
      gl_vertex(ctx,i(j));
    }

    glEnd();
    
    if(lightWasOn){
      glEnable(GL_LIGHTING);
    }           
  }

  void TrianglePrimitive::render(const Primitive::RenderContext &ctx){
    glBegin(GL_TRIANGLES);

    gl_auto_normal(ctx, i(0), i(1), i(2), i(3)==-1);
    
    glColor4fv(color.data());

    for(int j=0;j<3;++j){
      gl_normal(ctx,i(j+3));
      gl_color(ctx,i(j),ctx.triangleColorsFromVertices);
      gl_vertex(ctx,i(j));
    }
    glEnd();
  }

  void QuadPrimitive::render(const Primitive::RenderContext &ctx){
    glBegin(GL_QUADS);
    
    gl_auto_normal(ctx, i(3), i(1), i(2), i(4)==-1);
    
    glColor4fv(color.data());

    for(int j=0;j<4;++j){
      gl_normal(ctx,i(j+4));
      gl_color(ctx,i(j),ctx.quadColorsFromVertices);
      gl_vertex(ctx,i(j));
    }
    glEnd();
  }

  void PolygonPrimitive::render(const Primitive::RenderContext &ctx){
    glBegin(GL_POLYGON);
    
    // no autonormals supported!
    bool haveNormals = (idx.getHeight() == 2);
    glColor4fv(color.data());
    
    for(int j=0;j<idx.getWidth();++j){
      if(haveNormals) gl_normal(ctx,idx(j,1));
      gl_color(ctx,idx(j,0),ctx.polygonColorsFromVertices);
      gl_vertex(ctx,idx(j,0));
    }
    glEnd();
  }
  
  void TexturePrimitive::render(const Primitive::RenderContext &ctx){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1); 

    if(image){
      texture.update(image);
    }

    const Vec &a = ctx.vertices[i(0)];
    const Vec &b = ctx.vertices[i(1)];
    const Vec &c = ctx.vertices[i(2)];
    const Vec &d = ctx.vertices[i(3)];
    
    if(i(4) != -1 && i(5) != -1 && i(6) != -1 && i(7) != -1){
      const Vec &na = ctx.normals[i(4)];
      const Vec &nb = ctx.normals[i(5)];
      const Vec &nc = ctx.normals[i(6)];
      const Vec &nd = ctx.normals[i(7)];
      texture.draw3D(a.data(),b.data(),c.data(),d.data(),
                     na.data(), nb.data(), nc.data(), nd.data());
    }else{
      gl_auto_normal(ctx, i(3), i(1), i(2));
      texture.draw3D(a.data(),b.data(),c.data(),d.data());
    }
    glAlphaFunc(GL_GREATER,0.05); 

  }

  void SharedTexturePrimitive::render(const Primitive::RenderContext &ctx){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1); 

    GLImg &gli = const_cast<GLImg&>(*ctx.sharedTextures[sharedTextureIndex]);
    const Vec &a = ctx.vertices[i(0)];
    const Vec &b = ctx.vertices[i(1)];
    const Vec &c = ctx.vertices[i(2)];
    const Vec &d = ctx.vertices[i(3)];
    if(i(4) != -1 && i(5) != -1 && i(6) != -1 && i(7) != -1){
      const Vec &na = ctx.normals[i(4)];
      const Vec &nb = ctx.normals[i(5)];
      const Vec &nc = ctx.normals[i(6)];
      const Vec &nd = ctx.normals[i(7)];
      gli.draw3D(a.data(),b.data(),c.data(),d.data(),
                 na.data(), nb.data(), nc.data(), nd.data());
    }else{
      gl_auto_normal(ctx, i(3), i(1), i(2));
      gli.draw3D(a.data(),b.data(),c.data(),d.data());
    }
    glAlphaFunc(GL_GREATER,0.05); 
  }
  
  void TextPrimitive::render(const Primitive::RenderContext &ctx){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.3); 
    
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);

    if(billboardHeight > 0){
      const Vec &a = ctx.vertices[i(0)];

      glMatrixMode(GL_MODELVIEW);
      float m[16];
      glGetFloatv(GL_MODELVIEW_MATRIX, m);
             
      /// inverted rotation matrix
      Mat R(m[0],m[1],m[2],0,
            m[4],m[5],m[6],0,
            m[8],m[9],m[10],0,
            0,0,0,1);
              
      float ry = billboardHeight/2;
      float rx = ry * (float(texture.getWidth())/float(texture.getHeight()));
              
      Vec p1 = a + R*Vec(-rx,-ry,0,1);
      Vec p2 = a + R*Vec(rx,-ry,0,1);
      Vec p3 = a + R*Vec(rx,ry,0,1);
      Vec p4 = a + R*Vec(-rx,ry,0,1);
              
      /// -normal as we draw the backface
      glNormal3fv(icl::normalize(-(cross(p2-p3,p4-p3))).data());
              
      /// draw the backface to flip x direction
      texture.draw3D(p2.begin(),p1.begin(),p4.begin(),p3.begin());
    }else{
      TexturePrimitive::render(ctx);
    }
    glPopAttrib();
    
    glAlphaFunc(GL_GREATER,0.05); 
  }

  void TextureGridPrimitive::render(const Primitive::RenderContext &ctx){
    if(image){
      texture.update(image);
    }
    texture.drawToGrid(w,h,px,py,pz,pnx,pny,pnz,stride);
  }

  void TextureGridPrimitive::getAABB(Range32f aabb[3]){
    Range32f limits = Range32f::limits(); 
    std::swap(limits.minVal,limits.maxVal);
    std::fill(aabb,aabb+3,limits);
    
    const int dim = w * h * stride;
    for(int i=0;i<dim;i+=stride){
      aabb[0].extend(px[i]);
      aabb[1].extend(py[i]);
      aabb[2].extend(pz[i]);
    }
  }
  
}
