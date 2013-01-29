# todo: find a way to make this more dynamic
set(PCL_VER 1.6)
icl_check_external_package(PCL "pcl/point_types.h;pcl/io/pcd_io.h;pcl/io/file_io.h" "pcl_octree;pcl_kdtree;pcl_common;pcl_io" lib include/pcl-${PCL_VER} TRUE TRUE)

# necessary for using pcl from jochen-spickerhof ppa (pcl-1.6-all)
if(HAVE_PCL_COND)
  include_directories(${ICL_XDEP_PCL_PATH}/include/openni)
endif()

