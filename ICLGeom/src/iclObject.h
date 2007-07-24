#ifndef ICL_OBJECT_H
#define ICL_OBJECT_H

#include <iclImg.h>
#include <iclGeom.h>

/// The icl namespace
namespace icl{

  /** \cond */
  class ICLDrawWidget;
  class ImgBase;
  /** \endcond */

  class Object{
    public:
    typedef std::pair<int,int> Tuple;
    
    Object();
    
    virtual ~Object();
      

    virtual void translate(float dx, float dy, float dz){ transform(Mat::trans(dx,dy,dz)); }
    virtual void rotate(float rx, float ry, float rz){ transform(Mat::rot(rx,ry,rz)); }
    virtual void transform(const Mat &m);
    virtual void project(const Mat &cameramatrix);
    
    void tintPoint(int i, const Vec &color);
    void tintLine(int i, const Vec &color);

    inline int getPointCount() const { return (int)(m_vecPtsOrig.size()); }
    inline int getLineCount() const { return (int)(m_vecConnections.size()); }

    inline const Vec &getPointOrig(int i) const { return m_vecPtsOrig[i]; }
    inline const Vec &getPointTrans(int i) const { return m_vecPtsTrans[i]; }
    inline const Vec &getPointProj(int i) const { return m_vecPtsProj[i]; }
    
    inline Vec &getPointOrig(int i) { return m_vecPtsOrig[i]; }
    inline Vec &getPointTrans(int i) { return m_vecPtsTrans[i]; }
    inline Vec &getPointProj(int i) { return m_vecPtsProj[i]; }

    inline void pushState(){ TPushed = T; }
    inline void popState() { T = TPushed; }

    const Mat &getT() const { return T; }
    void setT(const Mat &m){ T=m; }
    
    VecArray &getPointsOrig() { return m_vecPtsOrig; }
    const VecArray &getPointsOrig() const{ return m_vecPtsOrig; }

    VecArray &getPointsTrans() { return m_vecPtsTrans; }
    const VecArray &getPointsTrans() const{ return m_vecPtsTrans; }

    VecArray &getPointsProj() { return m_vecPtsProj; }
    const VecArray &getPointsProj() const{ return m_vecPtsProj; }

    virtual void render(ICLDrawWidget *widget) const;
    virtual void render(Img32f *image) const;
    
    protected:
    void add(const Vec &p, const Vec &color=Vec(255,0,0,255));
    void add(const Tuple &t, const Vec &color=Vec(100,100,100,255));
    
    private:   
    Mat T,TPushed;

    VecArray m_vecPtsOrig;
    VecArray m_vecPtsTrans;
    VecArray m_vecPtsProj;
    
    VecArray m_vecPtsColors;
    VecArray m_vecLineColors;
    
    std::vector<Tuple> m_vecConnections;
  };
}

#endif
