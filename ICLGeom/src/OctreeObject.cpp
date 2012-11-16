#include <ICLGeom/OctreeObject.h>

namespace icl{
  namespace geom{


    void octree_object_render_box(float x0, float y0, float z0, 
                                  float x1, float y1, float z1){
      glColor4f(0,1,0,0.25);
      glBegin(GL_LINES);
      
      //top
      glVertex3f(x0,y0,z0);
      glVertex3f(x1,y0,z0);
      
      glVertex3f(x1,y0,z0);
      glVertex3f(x1,y1,z0);
      
      glVertex3f(x1,y1,z0);
      glVertex3f(x0,y1,z0);
      
      glVertex3f(x0,y1,z0);
      glVertex3f(x0,y0,z0);
      
      // downwards
      glVertex3f(x0,y0,z0);
      glVertex3f(x0,y0,z1);
      
      glVertex3f(x1,y0,z0);
      glVertex3f(x1,y0,z1);
      
      glVertex3f(x0,y1,z0);
      glVertex3f(x0,y1,z1);
      
      glVertex3f(x1,y1,z0);
      glVertex3f(x1,y1,z1);
      
      // bottom
      glVertex3f(x0,y0,z1);
      glVertex3f(x1,y0,z1);
      
      glVertex3f(x1,y0,z1);
      glVertex3f(x1,y1,z1);
      
      glVertex3f(x1,y1,z1);
      glVertex3f(x0,y1,z1);
      
      glVertex3f(x0,y1,z1);
      glVertex3f(x0,y0,z1);
      glEnd();
    }
  }
}
