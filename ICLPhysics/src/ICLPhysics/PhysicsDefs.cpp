#include <ICLPhysics/PhysicsDefs.h>

#include <ICLMarkers/QuadDetector.h>

float icl::physics::ICL_UNIT_TO_METER = 0.001f;
float icl::physics::METER_TO_BULLET_UNIT = 0.1f;

void ensure_correct_recursive_linkage_against_ICLMarkers(){
  icl::markers::QuadDetector qd;
}
