#include <iclDrawWidget3D.h>

namespace icl{
 
  struct ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    virtual void execute()=0;
  };

  // }}}
  
  struct Cube3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Cube3DDrawCommand(float x,float y,float z, float d):x(x),y(y),z(z),d(d){}
    virtual void execute(){
      printf("cube command \n");
      static GLfloat n[6][3] = {  
        {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
      static GLint faces[6][4] = {  
        {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
        {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
      static GLfloat v[8][3];  
      bool first = true;
      if(first){
        first = false;
         v[0][0] = v[1][0] = v[2][0] = v[3][0] = -d;
         v[4][0] = v[5][0] = v[6][0] = v[7][0] = d;
         v[0][1] = v[1][1] = v[4][1] = v[5][1] = -d;
         v[2][1] = v[3][1] = v[6][1] = v[7][1] = d;
         v[0][2] = v[3][2] = v[4][2] = v[7][2] = d;
         v[1][2] = v[2][2] = v[5][2] = v[6][2] = -d;
      }
      glTranslatef(x,y,z);
      for(int i=0;i<6;i++) {
        glBegin(GL_QUADS);
        glNormal3fv(&n[i][0]);
        glVertex3fv(&v[faces[i][0]][0]);
        glVertex3fv(&v[faces[i][1]][0]);
        glVertex3fv(&v[faces[i][2]][0]);
        glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
      }
      glTranslatef(-x,-y,-z);
    }
    float x,y,z,d;
  };

  // }}}
  struct Color3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Color3DDrawCommand(float r,float g,float b, float a):r(r),g(b),b(b),a(a){}
    virtual void execute(){
      printf("color command \n");
      glColor4f(r,g,b,a);
    }
    float r,g,b,a;
  };

  // }}}
  struct Clear3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    virtual void execute(){
      printf("clear command \n");
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
  };

  // }}}
  struct LookAt3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    LookAt3DDrawCommand(float ex, float ey, float ez, float cx, float cy, float cz, float ux, float uy, float uz):
      ex(ex),ey(ey),ez(ez),cx(cx),cy(cy),cz(cz),ux(ux),uy(uy),uz(uz){}
    virtual void execute(){
      printf("applying lookAt \n");
      glMatrixMode(GL_MODELVIEW);
      gluLookAt(ex,ey,ez,cx,cy,cz,ux,uy,uz);
    }
    float ex,ey,ez,cx,cy,cz,ux,uy,uz;
  };

  // }}}
  struct Frustum3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open
    Frustum3DDrawCommand(float left,float right,float bottom,float top,float zNear,float zFar):
      left(left),right(right),bottom(bottom),top(top),zNear(zNear),zFar(zFar){}

    virtual void execute(){
      printf("setting frustrum \n");
      glMatrixMode(GL_PROJECTION);
      glFrustum(left,right,bottom,top,zNear,zFar);
    }
    float left,right,bottom,top,zNear,zFar;
  };

  // }}} 
  struct Rotate3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Rotate3DDrawCommand(float rx,float ry,float rz):rx(rx),ry(ry),rz(rz){}
    virtual void execute(){
      printf("rotate by %f %f %f \n",rx,ry,rz);
      glRotatef(rx,1,0,0);
      glRotatef(ry,0,1,0);
      glRotatef(rz,0,0,1);
    }
    float rx,ry,rz;
  };

  // }}}
  struct Translate3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Translate3DDrawCommand(float tx,float ty,float tz):tx(tx),ty(ty),tz(tz){}
    virtual void execute(){
      printf("translate by %f %f %f \n",tx,ty,tz);
      glTranslatef(tx,ty,tz);
    }
    float tx,ty,tz;
  };

  // }}}
  struct MultMat3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    MultMat3DDrawCommand(float *mat){
      printf("multmat\n");
      memcpy(this->mat,mat,16*sizeof(float));
    }
    virtual void execute(){
      glMultMatrixf(mat);
    }
    float mat[16];
  };

  // }}}
  struct SetMat3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    SetMat3DDrawCommand(float *mat){
      memcpy(this->mat,mat,16*sizeof(float));
    }
    virtual void execute(){
      printf("setmat\n");
      glLoadIdentity();
      glMultMatrixf(mat);
    }
    float mat[16];
  };

  // }}}
  struct MatrixMode3DDrawCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    MatrixMode3DDrawCommand(bool projection){
      m = projection ? GL_PROJECTION : GL_MODELVIEW;
    }
    virtual void execute(){
      printf("matrix mode set to %s\n",m==GL_PROJECTION ? "GL_PROJECTION" : "GL_MODELVIEW");
      glMatrixMode(m);
    }
    
    GLenum m;
  };

  // }}}
  struct Perspective3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    Perspective3DCommand(float angle,float aspect,float near,float far):
      angle(angle),aspect(aspect),near(near),far(far){}
    virtual void execute(){
      glMatrixMode(GL_PROJECTION);
      gluPerspective(angle,aspect,near,far);
    }
    float angle,aspect,near,far;
  };

  // }}}
  struct ID3DCommand : public ICLDrawWidget3D::DrawCommand3D{
    // {{{ open

    virtual void execute() { glLoadIdentity(); }
  };

  // }}}

  ICLDrawWidget3D::ICLDrawWidget3D(QWidget *parent):ICLDrawWidget(parent){
    // {{{ open

  }

  // }}}
  ICLDrawWidget3D::~ICLDrawWidget3D(){
    // {{{ open

  }

  // }}}

  void ICLDrawWidget3D::customPaintEvent(PaintEngine *e){
    // {{{ open
    lock();
    glClear(GL_DEPTH_BUFFER_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  // A White diffuse light
    GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  // location is infinite

    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);


    glMatrixMode(GL_PROJECTION);
    gluPerspective( 45,  float(width())/height(), 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0, 0, -1,   // pos
              0, 0,  0,   // view center point
              1, 0,  0 );// up vector
    

#ifdef DO_NOT_USE_GL_VISUALIZATION
    ERROR_LOG("3D Visualization is not supported without OpenGL!");
#else
    printf("------------------------------------------------------ \n");
    for(unsigned int i=0;i<m_vecCommands3D.size();++i){
      m_vecCommands3D[i]->execute();
    }
    
#endif
    
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    unlock();

    ICLDrawWidget::customPaintEvent(e);
  }

  // }}}
  void ICLDrawWidget3D::reset3D(){
    // {{{ open
    for(unsigned int i=0;i<m_vecCommands3D.size();++i){
      delete m_vecCommands3D[i];
    }
    m_vecCommands3D.clear();
  }

  // }}}
  
  
  void ICLDrawWidget3D::clear3D(){
    m_vecCommands3D.push_back(new Clear3DDrawCommand);
  }
  void ICLDrawWidget3D::cube3D(const ICLDrawWidget3D::vec &v, float d){
    m_vecCommands3D.push_back(new Cube3DDrawCommand(v.x,v.y,v.z,d));
  }
  void ICLDrawWidget3D::color3D(float r, float g, float b, float a){
    m_vecCommands3D.push_back(new Color3DDrawCommand(r,g,b,a));
  }
  void ICLDrawWidget3D::lookAt3D(const vec &eye, const vec &c, const vec &up){
    m_vecCommands3D.push_back(new LookAt3DDrawCommand(eye.x,eye.y,eye.z,c.x,c.y,c.z,up.x,up.y,up.z));
  }
  void ICLDrawWidget3D::frustum3D(float left,float right,float bottom, float top,float zNear,float zFar){
    m_vecCommands3D.push_back(new Frustum3DDrawCommand(left,right,bottom,top,zNear,zFar));
  }
  void ICLDrawWidget3D::rotate3D(float rx, float ry, float rz){
    m_vecCommands3D.push_back(new Rotate3DDrawCommand(rx,ry,rz));
  }
  void ICLDrawWidget3D::translate3D(float tx, float ty, float tz){
    m_vecCommands3D.push_back(new Translate3DDrawCommand(tx,ty,tz));
  }
  void ICLDrawWidget3D::multMat3D(float *mat){
    m_vecCommands3D.push_back(new MultMat3DDrawCommand(mat));
  }
  void ICLDrawWidget3D::setMat3D(float *mat){
    m_vecCommands3D.push_back(new SetMat3DDrawCommand(mat));
  }
  void ICLDrawWidget3D::modelview(){
    m_vecCommands3D.push_back(new MatrixMode3DDrawCommand(false));
  }
  void ICLDrawWidget3D::projection(){
    m_vecCommands3D.push_back(new MatrixMode3DDrawCommand(true));
  }
  void ICLDrawWidget3D::perspective(float angle, float aspect, float near, float far){
    m_vecCommands3D.push_back(new Perspective3DCommand(angle,aspect,near,far));
  }
  void ICLDrawWidget3D::id(){
    m_vecCommands3D.push_back(new ID3DCommand);
  }
}
