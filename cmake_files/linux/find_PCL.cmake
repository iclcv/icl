# todo: find a way to make this more dynamic
set(PCL_VER 1.6)
icl_simple_check_external_package(PCL "pcl-${PCL_VER}/pcl/point_types.h;pcl-${PCL_VER}/pcl/io/pcd_io.h;pcl-${PCL_VER}/pcl/io/file_io.h" "pcl_common;pcl_io" TRUE)
