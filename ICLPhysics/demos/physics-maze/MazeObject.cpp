#include <MazeObject.h>
#include <ICLQt/Quick.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/SixDOFConstraint.h>
// #include <MILabIO/viconreader.h>

namespace icl{
    using namespace physics;
    using namespace geom;
    using namespace utils;
    using namespace qt;

    //***************************************
    // TextObject
    //***************************************

//     TextObject::TextObject(const std::string &text,
//                            const geom::Vec &pos,
//                            const Vec &pos_text,
//                            const GeomColor &color)
//         : m_text(text), m_pos(pos), m_color(color) {
// 
// 
//         //addLine(0,1,m_color);
//         QString str(text.c_str());
//         QStringList list = str.split(",");
//         int count = 0;
//         //addVertex(pos_text,geom_invisible());
//         Vec t_pos = pos_text;
//         foreach (QString elem, list) {
//             addVertex(t_pos,geom_invisible());
//             addText(count,elem.toStdString(),15,m_color,40);
//             t_pos[1] += 20;
//             ++count;
//         }
// 
//         setLineWidth(3);
// 
//     }

    //***************************************
    // MazeWall Object
    //***************************************

    MazeWall::MazeWall(const utils::Point32f &p1,
                       const utils::Point32f &p2,
                       bool outer)
        : Line32f(p1,p2),
          m_wall_offset(outer ? 2.5 : 2) {}

    //=============================================================

    MazeWall::MazeWall(const float a[2],
                       const float b[2],
                       bool outer)
        : Line32f(Point32f(a[0],a[1]),Point32f(b[0],b[1])),
          m_wall_offset(outer ? 2.5 : 2) {}

    //=============================================================

    bool MazeWall::hasCollisionWithCircle(const utils::Point32f &center,
                                       const float r) {

//         // the "vectors" in 2d
//         Point32f AC = center-start;
//         Point32f BC = center-end;
//         Point32f AB = end-start;
//         Point32f BA = start-end;
// 
//         // project the circle vectors onto the line segment
//         Point32f AAC = AB*(scalar2D(AC,AB)/scalar2D(AB,AB));
//         Point32f BBC = BA*(scalar2D(BC,BA)/scalar2D(BA,BA));
// 
//         // get the length of the projected vectors
//         float l_AAC = length2D(AAC);
//         float l_BBC = length2D(BBC);
//         float l_AB = this->length();
// 
//         // if the length of the projected vectors is longer than the line segment,
//         // there is no intersection (circle around line-center + radius)
//         if (l_AAC > r+m_wall_offset+l_AB || l_BBC > r+m_wall_offset+l_AB) {
//             return false;
//         }
// 
//         // if the perpendicular bisectors of the sides's length is greater than the radius r,
//         // there is no collision
//         float dH = length2D(AC-AAC);
//         if (dH > r+m_wall_offset) {
//             return false;
//         }

        return true;
    }

    //***************************************
    // MazePart Object
    //***************************************

//    const GeomColor MazeSection::fg_color[2] = {
//        GeomColor(0,0,0,200), // inactive
//        GeomColor(80,80,80,200) // active
//    };
//    const GeomColor MazeSection::bg_color[2] = {
//        GeomColor(100,100,100,100), // inactive
//        GeomColor(250,250,250,200) // active
//    };

    const GeomColor MazeSection::fg_color[2] = {
        GeomColor(255,255,255,255), // inactive
        GeomColor(255,255,255,255) // active
    };
    const GeomColor MazeSection::bg_color[2] = {
        GeomColor(255,255,255,255), // inactive
        GeomColor(255,255,255,255) // active
    };

    //=============================================================

    Img32f MazeObject::topImage;

    /*        <-- width -->
              .___________. ^
              |           | |
              |           | height
              |           | |
          pos !___________! v
    */
    MazeSection::MazeSection(const utils::Point32f &pos,
                       const utils::Size32f &size,
                       const int id)
        : SceneObject(), m_id(id) {

        static const float cx = 82, cy = 72;
        static const float wall_height = 12.0f;
        static const float floor_z = 0.2f;
        //static const Mat rot = create_hom_4x4<float>(M_PI,0,0);

        const float &w = size.width;
        const float &h = size.height;

        addCuboid(pos.x - cx + (w-1)/2, // x
                  pos.y - cy + (h-1)/2, // y
                  -wall_height,      // z
                  (w-1),(h-1),floor_z);     // dimensions

        m_pos = Vec(pos.x - cx + w/2, pos.y - cy + h/2, -wall_height+1.0,1);

        addVertex(m_pos,
                  geom_invisible());


        std::stringstream sstream;
        sstream << id;
        addText(0,sstream.str(),15,fg_color[0],40);

        setLineWidth(2);
        setVisible(Primitive::all,true);
        setColor(Primitive::quad, bg_color[0]);
        setColor(Primitive::line, fg_color[0]);
        setColor(Primitive::text, fg_color[0]);

        //rotate(M_PI,0,0);
        //m_pos = rot * m_pos;

        // internal structure (2D)
        points.push_back(pos);
        points.push_back(Point32f(pos.x,pos.y+h));
        points.push_back(Point32f(pos.x+w,pos.y+h));
        points.push_back(Point32f(pos.x+w,pos.y));

    }

    //-----------------------------------------------------------------

    void MazeSection::setWalls(std::vector<MazeWall*> &walls) { m_walls = walls; }

    //-----------------------------------------------------------------

    int MazeSection::checkCircleCollideWalls(const Point32f &pos, const float r) {
        int counter = 0;
        for(uint i = 0; i < m_walls.size(); ++i) {
            if (m_walls[i]->hasCollisionWithCircle(pos,r))
                ++counter;
        }
        return counter;
    }

    //-----------------------------------------------------------------

    bool MazeSection::checkActivated(const utils::Point32f &pos, bool autohighlight) {

        if (points.size() < 4)
            return false;

        bool result = pos.inTriangle(points[0],points[1],points[2])
                || pos.inTriangle(points[0],points[3],points[2]);

        return result;
    }

    //-----------------------------------------------------------------

    void MazeSection::setActivated(const bool act) {
        setColor(Primitive::quad, bg_color[(int)act]);
        setColor(Primitive::line, fg_color[(int)act]);
        setColor(Primitive::text, fg_color[(int)act]);
    }

    //==================================================================

    void MazeObject::create_wall_x(float a[2], float b[2], bool outer){
        float m = outer ? 2.5 : 2; // wall thickness := 2m
        static const float h = 6;    // wall inner height := 2h
        static const float cx = 82, cy = 72;
        if(a[0] == b[0]) { // vertical
//            addCuboid(a[0] - cx, (a[1]+b[1])/2-cy , -h,
//                    2*m, ::abs(a[1]-b[1])+2*m, 2*h);
            addChild(new RigidBoxObject(a[0] - cx, (a[1]+b[1])/2-cy , -h,
                    2*m, ::abs(a[1]-b[1])+2*m, 2*h,0));

            TopEdge e1 = { Point32f(a[0] - cx-m,a[1]-cy),
                         Point32f(b[0] - cx-m,b[1]-cy) };
            e1.a.y *=-1;
            e1.b.y *=-1;
            topEdges.push_back(e1);

            TopEdge e2 = { Point32f(a[0] - cx+m,a[1]-cy),
                         Point32f(b[0] - cx+m,b[1]-cy) };
            e2.a.y *=-1;
            e2.b.y *=-1;
            topEdges.push_back(e2);

            if(a[1] > b[1]){
            std::swap(a[1], b[1]);
            }
            if(m == 2.5) m = 2;
            rect(topImage,
               round(20+10*(a[0]-m)),
               round(20+10*(a[1]-m)),
               round(10*m*2),
               round(10*(b[1]-a[1]+2*m)));

        }else{ // horizontal
//            addCuboid((a[0]+b[0])/2 - cx,a[1] -cy, -h,
//                    ::abs(a[0]-b[0])+2*m,2*m, 2*h);
            addChild(new RigidBoxObject((a[0]+b[0])/2 - cx,a[1] -cy, -h,
                    ::abs(a[0]-b[0])+2*m,2*m, 2*h,0));
            TopEdge e1 = { Point32f(a[0] - cx,a[1]-cy-m),
                           Point32f(b[0] - cx,b[1]-cy-m) };
            e1.a.y *=-1;
            e1.b.y *=-1;
            topEdges.push_back(e1);
            TopEdge e2 = { Point32f(a[0] - cx,a[1]-cy+m),
                           Point32f(b[0] - cx,b[1]-cy+m) };

            e2.a.y *=-1;
            e2.b.y *=-1;
            topEdges.push_back(e2);

            if(a[0] > b[0]){
            std::swap(a[0], b[0]);
            }
            if(m == 2.5) m = 2;
            rect(topImage,
               round(20+10*(a[0]-m)),
               round(20+10*(a[1]-m)),
               round(10*(b[0]-a[0]+2*m)),
               round(10*m*2));
            }

    }

    //==================================================================

    void MazeObject::create_wall(const float a[2], const float b[2], bool outer){
        float aa[2] = {a[0], a[1]}, bb[2] = {b[0], b[1]};
        create_wall_x(aa,bb,outer);
    }

    //==================================================================

    MazeObject::MazeObject()
        : RigidCompoundObject(0,0,0,0), m_z_offset(12),
          maze_ball_diameter(8),
          cur_part_index(-1) {

        data_index = -1;
        static const float cx = 82, cy = 72;
        topImage = ones((2*cx)*10, (2*cy)*10, 3) * 255;
        color(255,255,255,255);
        fill(30,30,30,255);

        for(size_t i=0;i<sizeof(lines)/(2*sizeof(int));++i){
            create_wall(points+2*lines[2*i], points+2*lines[2*i+1], i<4);
            MazeWall *wall = new MazeWall(points+2*lines[2*i], points+2*lines[2*i+1], i<4);
            walls.push_back(wall);
        }

        //set physics properties
        setRestitution(0.1f);
        setFriction(0.8f);
        setRollingFriction(0.0f);
        //add mazeGround
        mazeGround = new RigidBoxObject(85-2.5 - cx, 75-2.5 - cy, -13.5, 170, 150, 3,1);
        mazeGround->setColor(Primitive::all,geom_white());

        //add holes
        for(int i = 0; i < NUM_HOLES; i++) {
          m_holes.push_back(new HoleObject(hole_positions[i][0],hole_positions[i][1],hole_positions[i][2]));
        }

        //add ball

        mazeBall = new RigidSphereObject(15,0,0,7,0.008);
        mazeBall->setRestitution(0.1f);
        mazeBall->setFriction(0.5f);
        mazeBall->setRollingFriction(0.0f);
        mazeBall->setColor(Primitive::all,geom_red());
        mazeBall->setCollisionCallback(function(this,&MazeObject::ballCallback));

        setLineSmoothingEnabled(false);
        setPolygonSmoothingEnabled(false);
        setColor(Primitive::line, GeomColor(0,100,200,255),true);
        //setVisible(Primitive::quad,false);
        setColor(Primitive::quad, GeomColor(255,255,255,255));

        for (size_t i=0; i < NUM_MAZE_SECTIONS; ++i) {
            MazeSection *part = new MazeSection(Point32f(sections[i*4], sections[i*4+1]),
                                          Size32f(sections[i*4+2], sections[i*4+3]),
                                          (int)i);
            SceneObject::addChild(part);
            std::vector<MazeWall*> tmp;
            for (int w = 0; w < 5; ++w) {
                int wall_id = sectionWalls[i*5+w];
                if (wall_id >= 0)
                    tmp.push_back(walls[wall_id]);
            }
            part->setWalls(tmp);
            parts.push_back(part);
        }

        //addChild(new ComplexCoordinateFrameSceneObject(20,3));
        coord = new ComplexCoordinateFrameSceneObject(20,3);
        SceneObject::addChild(coord,false);

        setLineSmoothingEnabled(false);
        setPolygonSmoothingEnabled(false);

        //rotate(M_PI,0,0);
    }

    //==================================================================

    bool MazeObject::loadTrajecetory(const std::string &filepath) {
//         loaded_trajectory.clear();
//         data_index = 0;
//         MILabIO::ViconReader vr(filepath,true);
//         if ( !vr.isValid() ) {
//             return false;
//         }
//         float fps = vr.getCaptureFrequency();
//         MILabCommon::ViconFrameVector frames = vr.getFramesAsPoints();
//         std::cout << "number of frames loaded: " << frames.size()  << std::endl;
//         for(unsigned int i = 0; i < frames.size(); ++i) {
//             MazePoints p;
//             MILabCommon::ViconFrame &frame = frames[i];
//             //std::cout << "points in frame: " << frame.size()  << std::endl;
//             if (frame.size() == 4) {
//                 p.a = frame[0]->toICLColVector4D();
//                 p.b = frame[1]->toICLColVector4D();
//                 p.c = frame[2]->toICLColVector4D();
//                 p.d = frame[3]->toICLColVector4D();
//             } else {
//                 std::cout << "#points in frame exceed expectation: " << frame.size()  << std::endl;
//             }
//             // fill in dummy data
//             p.idx = i+1;
//             p.ms = i*(1.0f/fps);
//             loaded_trajectory.push_back(p);
//         }
//         // update first pose
//         if (loaded_trajectory.size() > 0)
//             update(loaded_trajectory[0]);
//         else
//             data_index = -1;
// 
        return true;
    }

    //==================================================================

//     void MazeObject::getViconPoints(std::vector<geom::Vec> &data) {
//         if (data.size() < 4 || data_index < 0)
//             return;
//         MazePoints &pts = loaded_trajectory[data_index];
//         data[0] = pts.a;
//         data[1] = pts.b;
//         data[2] = pts.c;
//         data[3] = pts.d;
//     }

    //==================================================================

    Mat invert_hom_4x4(const Mat &T) {

        Mat R(T[0],T[1],T[2],0,
            T[4],T[5],T[6],0,
            T[8],T[9],T[10],0,
            0,0,0,1);
        Vec trans(T[3],T[7],T[11]);

        Mat R_i = R.inv();
        Vec trans_i = R_i * trans * -1;

        R_i[3] = trans_i[0];
        R_i[7] = trans_i[1];
        R_i[11] = trans_i[2];

        return R_i;
    }

    //==================================================================

    int MazeObject::update(int index) {
/*
        if (index >= 0 && loaded_trajectory.size() < (uint)index)
            data_index = index;

        if (data_index >= (int)loaded_trajectory.size())
            data_index = 0;

        MazePoints &pts = loaded_trajectory[data_index];
        int index_swap = data_index;
        data_index = (data_index+1)%loaded_trajectory.size();
        update(pts);

        return index_swap;*/
	return 0;
    }

    void MazeObject::addToWorld(PhysicsScene *scene) {
      scene->addObject(this);
      scene->addObject(mazeBall);
      scene->addObject(mazeGround);
      SixDOFConstraint *c = new SixDOFConstraint(mazeGround,
                                                 this,
                                                 Mat::id(),
                                                 mazeGround->getTransformation());
      scene->addConstraint(c,true,true);
      for(int i = 0; i < NUM_HOLES; i++) {
        scene->addObject(m_holes[i]);
        c = new SixDOFConstraint(m_holes[i],
                                 this,
                                 Mat::id(),
                                 m_holes[i]->getTransformation());
        scene->addConstraint(c,true,true);
      }
    }

    void MazeObject::ballCallback(PhysicsObject* self, PhysicsObject* other, geom::Vec pos) {
      if(dynamic_cast<MazeObject*>(other)) {
          //do whatever is supposed to be deone on collision with a wall
      }
    }

    //==================================================================

    /*
        c .______________. d
          | ___  ___|    |
          |  |_  _|  _|  |
          | _|  _| _|   _|
          | |______|__|  |
          |___     |  ___|
        b !____|_________! a
      */
//     void MazeObject::update(const MazePoints &pts) {
// 
//         Vec x_ = pts.a - pts.b;
//         x_.normalize();
//         Vec y_ = pts.c - pts.b;
//         y_.normalize();
//         Vec z_ = icl::geom::cross(x_,y_);
// 
//         Vec center = (pts.a + pts.b + pts.c + pts.d) / 4.0f;
//         center_offset = center - pts.b;
// 
//         static const Mat offset_rot = create_hom_4x4<float>(0,0,0);//(M_PI,0.0f,0.0f);
// 
//         // the visual transformation of the maze (the maze center is the origin)
//         Mat vis_transform(x_[0],y_[0],z_[0],center[0],
//                           x_[1],y_[1],z_[1],center[1],
//                           x_[2],y_[2],z_[2],center[2],
//                           0,0,0,1);
//         setTransformation(vis_transform*offset_rot);
// 
//         // create the transformation from global to maze coord and vice versa (vicon point 'b' is the origin)
//         maze_to_world_T = Mat(x_[0],y_[0],z_[0],pts.b[0],
//                              x_[1],y_[1],z_[1],pts.b[1],
//                              x_[2],y_[2],z_[2],pts.b[2] - m_z_offset,
//                              0,0,0,1);
//         world_to_maze_T = invert_hom_4x4(maze_to_world_T);
// 
//         // transform the coord-system representation:
//         static const float cx = 92, cy = 82;
//         Mat coord_transform(1,0,0,-cx,
//                            0,1,0,-cy,
//                            0,0,1,-m_z_offset,
//                            0,0,0,1);
//         coord->setTransformation(offset_rot*coord_transform);
// 
//         // create the maze plane (for computation of 2D positions of the ball)
//         Mat plane_transform(1,0,0,0,
//                             0,1,0,0,
//                             0,0,1, - m_z_offset + (maze_ball_diameter/2.0f),
//                             0,0,0,1);
//         maze_3d_plane = PlaneEquation(plane_transform*pts.b,z_);
// 
//         last_data = pts;
// 
//     }

    void MazeObject::transformForMazeVis(geom::Vec &val) {
        static const Mat offset_rot = create_hom_4x4<float>(M_PI,0.0f,0.0f);
        static const float cx = 82, cy = 72;
        Mat coord_transform(1,0,0,-cx,
                            0,1,0,+cy,
                            0,0,1,+m_z_offset,
                           0,0,0,1);
        /*Mat coord_transform(1,0,0,+center_offset[0],
                            0,1,0,-center_offset[1],
                            0,0,1,-center_offset[2]+m_z_offset,
                           0,0,0,1);*/

        val = coord_transform*offset_rot*val;
    }

    Rect MazeObject::computeMazeROI(const Camera &cam) {
//         std::vector<geom::Vec> data(4);
//         getViconPoints(data);
//         std::vector<Point32f> p2d = cam.project(data);
// 
//         float x_max = 0, y_max = 0;
//         float x_min = 0, y_min = 0;
//         for (uint i = 0; i < p2d.size(); ++i) {
//             const Point32f &p = p2d[i];
//             x_min = std::min(p.x,x_min);
//             y_min = std::min(p.y,y_min);
//             x_max = std::max(p.x,x_max);
//             y_max = std::max(p.y,y_max);
//         }
// 
//         return Rect(x_min,y_min,x_max-x_min,y_max-y_min);
	return Rect(0,0,0,0);
    }

    MazeSection* MazeObject::whichSection(const utils::Point32f &pos) {
        // if we already have a pre-known position, we only look at the neighbours and itself
        if (cur_part_index > -1) {
            // check whether the given position is still in the same region
            if (parts[cur_part_index]->checkActivated(pos)) {
                return parts[cur_part_index];
            }
            // otherwise, check the neighbours
            const int *neighbours = &neighbourhood[cur_part_index*MAX_SECTION_NEIGBOURS];
            for(uint i = 0; i < MAX_SECTION_NEIGBOURS && i < MAX_SECTION_NEIGBOURS*NUM_MAZE_SECTIONS; ++i) {
                if (neighbours[i] > 0) {
                    if (parts[neighbours[i]]->checkActivated(pos)) {
                        cur_part_index = neighbours[i];
                        return parts[cur_part_index];
                    }
                }
            }
        }
        std::cout << "WARNING: Doing an overall search in 'whichSection()'\n" << std::flush;
        // if the search through the neighbours was not successful, try an overall search
        for(uint i = 0; i < parts.size(); ++i) {
            if (parts[i]->checkActivated(pos)) {
                cur_part_index = i;
                return parts[cur_part_index];
            }
        }
        // if nothing was found, we return a NULL-pointer
        return 0;
    }

    const float MazeObject::sections[4*NUM_MAZE_SECTIONS] = {
         0,0,62.5,22.5           //0
        ,62.5,0,102.5,22.5       //1
        ,0,22.5,22.5,60          //2
        ,22.5,22.5,80,20         //3
        ,102.5,22.5,40,20        //4
        ,142.5,22.5,22.5,60      //5
        ,22.5,42.5,20,27.5       //6
        ,42.5,42.5,40,30         //7
        ,82.5,42.5,20,50         //8
        ,102.5,42.5,40,20        //9
        ,0,82.5,22.5,62.5        //10
        ,22.5,70,20,52.5         //11
        ,42.5,72.5,20,30         //12
        ,62.5,72.5,20,30         //13
        ,102.5,62.5,20,60        //14
        ,122.5,62.5,20,40        //15
        ,142.5,82.5,22.5,62.5    //16
        ,42.5,102.5,20,20        //17
        ,62.5,102.5,20,20        //18
        ,82.5,92.5,20,30         //19
        ,122.5,102.5,20,42.5     //20
        ,22.5,122.5,100,22.5     //21
    };

    const int MazeObject::neighbourhood[MAX_SECTION_NEIGBOURS*NUM_MAZE_SECTIONS] = {
         3,-1,-1,-1     //for 0
        ,3,4,-1,-1      //for 1
        ,3,10,-1,-1     //for 2
        ,2,1,-1,-1      //for 3
        ,1,5,-1,-1      //for 4
        ,4,15,-1,-1     //for 5
        ,7,-1,-1,-1     //for 6
        ,6,8,12,-1      //for 7
        ,7,14,-1,-1     //for 8
        ,15,-1,-1,-1    //for 9
        ,2,11,21,-1     //for 10
        ,10,-1,-1,-1    //for 11
        ,7,13,-1,-1     //for 12
        ,12,18,-1,-1    //for 13
        ,8,20,-1,-1     //for 14
        ,5,9,16,-1      //for 15
        ,15,20,-1,-1    //for 16
        ,18,-1,-1,-1    //for 17
        ,13,17,19,21    //for 18
        ,13,18,-1,-1    //for 19
        ,14,16,-1,-1    //for 20
        ,10,18,-1,-1    //for 21
    };


    const int MazeObject::sectionWalls[5*NUM_MAZE_SECTIONS] = {
                        // segment number
        2,3,15,16,-1,   // 0
        1,2,16,24,21,   // 1
        3,6,15,-1,-1,   // 2
        8,15,16,21,-1,  // 3
        21,23,24,-1,-1, // 4
        1,22,23,24,-1,  // 5
        5,6,7,8,-1,     // 6
        5,8,14,-1,-1,   // 7
        8,21,13,12,-1,  // 8
        8,20,21,23,-1,  // 9
        3,4,-1,-1,-1,   // 10
        4,5,6,7,-1,     // 11
        5,25,14,-1,-1,  // 12
        25,14,13,-1,-1, // 13
        20,11,19,10,-1, // 14
        19,18,22,23,-1, // 15
        0,1,17,22,-1,   // 16
        4,5,25,-1,-1,   // 17
        4,10,25,-1,-1,  // 18
        10,11,12,-1,-1, // 19
        9,18,17,-1,-1,  // 20
        0,4,10,9,-1     // 21
    };

    const int MazeObject::sectionWallCount[NUM_MAZE_SECTIONS] = {
        4,5,3,4,3,4,4,3,4,4,2,4,3,3,4,4,4,3,3,3,3,4
    };
  
    const float MazeObject::points[2*39] = {
        // corners
        0,145-0,           //p0
        165,145-0,         //p1
        0, 145-145,        //p2
        165, 145-145,      //p3
        //
        25, 145-22.5,      //4
        42.5, 145-22.5,    //5
        60, 145-22.5,      //6
        85, 145-22.5,      //7
        102.5, 145-22.5,   //8
        122.5, 145-22.5,   //9
        142.5, 145-25,     //10
        122.5, 145-0,      //11
        122.5, 145-42.5,   //12
        142.5, 145-42.5,   //13
        82.5, 145-52.5,    //14
        102.5, 145-52.5,   //15
        22.5, 145-65,      //16
        145, 145-62.5,     //17
        22.5, 145-75,      //18
        42.5, 145-75,      //19
        65, 145-72.5,      //20
        82.5, 145-72.5,    //21
        42.5, 145-80,      //22
        102.5, 145-82.5,   //23
        122.5, 145-82.5,   //24
        142.5, 145-85,     //25
        165, 145-62.5,     //26
        22.5, 145-102.5,   //27
        102.5, 145-102.5,  //28
        142.5, 145-102.5,  //29
        0, 145-122.5,      //30
        40, 145-122.5,     //31
        62.5, 145-122.5,   //32
        102.5, 145-120,    //33
        125, 145-122.5,    //34
        165, 145-122.5,    //35
        62.5, 145-145,     //36
        42.5, 145-42.5,    //37
        60, 145-42.5       //38
    };

    const int MazeObject::lines[2*26] = {
        // main frame
        0, 1,   //0
        1, 3,   //1
        3, 2,   //2
        2, 0,   //3
        // the rest
        4, 6,   //4
        5, 22,  //5
        16,27,  //6
        18,19,  //7
        27,29,  //8
        11,9,   //9
        7,9,    //10
        8,15,   //11
        14,15,  //12
        14,21,  //13
        20,21,  //14
        30,31,  //15
        32,36,  //16
        10,13,  //17
        13,12,  //18
        12,24,  //19
        23,24,  //20
        23,33,  //21
        17,26,  //22
        25,29,  //23
        34,35,  //24
        37,38   //25
    };

    const geom::Vec MazeObject::hole_positions[NUM_HOLES] = {
      geom::Vec(-74,-42,-11),
      geom::Vec(-52,-21,-11),
      geom::Vec(-27,-38,-11),
      geom::Vec(-18, 38,-11),
      geom::Vec(  2,-63,-11),
      geom::Vec( 13, 29,-11),
      geom::Vec( 53,-21,-11),
      geom::Vec( 75,-42,-11),
      geom::Vec( 75,65,-11),
    };

}
