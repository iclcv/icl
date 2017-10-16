#pragma once

#include <ICLCore/Img.h>
#include <ICLCore/Line32f.h>

#include <ICLGeom/SceneObject.h>
#include <ICLGeom/PlaneEquation.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>
#include <ICLPhysics/RigidCompoundObject.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/PhysicsScene.h>
#include <HoleObject.h>

// #include "maze_utils.h"

namespace icl{

    #define NUM_MAZE_SECTIONS 22
    #define NUM_HOLES 9
    #define MAX_SECTION_NEIGBOURS 4

    geom::Mat invert_hom_4x4(const geom::Mat &T);

    //=================================================

    class TextObject : public geom::SceneObject {

    public:

        TextObject(const std::string &text,
                   const geom::Vec &pos,
                   const geom::Vec &pos_text,
                   const geom::GeomColor &color);

    protected:

        std::string m_text;
        geom::Vec m_pos;
        geom::GeomColor m_color;

    };

    //=================================================

    class MazeWall : public core::Line32f {
    public:
        MazeWall(const utils::Point32f &p1,
                 const utils::Point32f &p2,
                 bool outer);

        MazeWall(const float a[2],
                 const float b[2],
                 bool outer);

        bool hasCollisionWithCircle(const utils::Point32f &center,
                                 const float r);

    protected:

        float m_wall_offset;
    };

    //=================================================

    class MazeSection : public geom::SceneObject {

    public:
        /**
             u_left .___________.
                    |           |
                    |           |
                    |           |
                    !___________! l_right
        */
        MazeSection(const utils::Point32f &pos,
                 const utils::Size32f &size, const int id);

        void setWalls(std::vector<MazeWall*> &walls);

        bool checkActivated(const utils::Point32f &pos, bool autohighlight = true);
        int checkCircleCollideWalls(const utils::Point32f &pos, const float r);

        void setActivated(const bool act);

        const int getID() const { return m_id; }

        geom::Vec getPosition() { return m_pos; }

        bool operator==(const MazeSection &other) const {
            return (m_id == other.getID());
        }

        bool operator!=(const MazeSection &other) const {
            return (m_id != other.getID());
        }

    protected:

        std::vector<utils::Point32f> points;
        int m_id;
        std::vector<MazeWall*> m_walls;

        static const geom::GeomColor fg_color[2];
        static const geom::GeomColor bg_color[2];

        geom::Vec m_pos;

        geom::TextPrimitive *text;
    };

    //=================================================

  class MazeObject : public physics::RigidCompoundObject{

    void create_wall_x(float a[2], float b[2], bool outer);
    void create_wall(const float a[2], const float b[2], bool outer);
    static const float sections[4*NUM_MAZE_SECTIONS];
    static const int sectionWalls[5*NUM_MAZE_SECTIONS];
    static const int neighbourhood[4*NUM_MAZE_SECTIONS];
    static const int sectionWallCount[NUM_MAZE_SECTIONS];
    static const float points[2*39];
    static const int lines[2*26];
    static const geom::Vec hole_positions[NUM_HOLES];
    static core::Img32f topImage;


    public:
    physics::RigidBoxObject* mazeGround;
    physics::RigidSphereObject* mazeBall;

    struct TopEdge{
      utils::Point32f a,b;
    };
    const std::vector<TopEdge> &getTopEdges() const{
      return topEdges;
    }

    const core::Img32f &getTopImage() const{
      return topImage;
    }

    MazeObject();

    /// loads trajectory of vicon markers from given file
    bool loadTrajecetory(const std::string &filepath);

    /// updates transformation from internal loaded trajectory and returns the index for the data
    int update(int index = -1);

    /// updates transformation from external data (needs maze points)
//     void update(const MazePoints &pts);

    geom::Vec getCenterOffset() { return center_offset; }

    geom::Vec transformWorldToMaze(const geom::Vec &v) { return getWorldToMazeTransform() * v; }

    geom::Vec transformMazeToWorld(const geom::Vec &v) { return getMazeToWorldTransform() * v; }

    geom::Mat getWorldToMazeTransform() { return invert_hom_4x4(getTransformation()); }

    geom::Mat getMazeToWorldTransform() { return getTransformation(); }

    const geom::PlaneEquation &getMazePlane() { return maze_3d_plane; }

    const geom::PlaneEquation &getMazePlane() const { return maze_3d_plane; }

    void getViconPoints(std::vector<geom::Vec> &data);

//     int getFrameCount() { return loaded_trajectory.size(); }

    utils::Rect computeMazeROI(const geom::Camera &cam);

    MazeSection* whichSection(const utils::Point32f &pos);

    void transformForMazeVis(geom::Vec &val);

    void addToWorld(physics::PhysicsScene *scene);

    void ballCallback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos);

    private:

    geom::Vec draw_offset;
    geom::Vec center_offset;
    geom::Mat world_to_maze_T;
    geom::Mat maze_to_world_T;
    geom::PlaneEquation maze_3d_plane;
//     MazePoints last_data;

    /// internal trajectory for the maze
//     std::vector<MazePoints> loaded_trajectory;
    int data_index;

    const float m_z_offset;
    const float maze_ball_diameter;

    std::vector<MazeSection*> parts;
    std::vector<MazeWall*> walls;
    std::vector<physics::HoleObject*> m_holes;
    int cur_part_index;

    std::vector<TopEdge> topEdges;

    geom::ComplexCoordinateFrameSceneObject *coord;
  };
}
