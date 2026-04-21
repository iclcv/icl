# Apps ported from Quick.h/Common.h → Quick2.h/Common2.h

## Trivial (header swap only)

- filter/apps/color-segmentation.cpp
- filter/apps/filter-array.cpp
- filter/apps/local-thresh.cpp
- filter/apps/rectify-image.cpp
- geom/apps/compute-relative-camera-transform.cpp
- geom/apps/depth-camera-simulator.cpp
- geom/apps/fix-kinect-calibration.cpp
- geom/apps/kinect-recorder.cpp
- geom/apps/point-cloud-creator.cpp
- geom/apps/point-cloud-define-world-frame.cpp
- geom/apps/point-cloud-pipe.cpp
- geom/apps/point-cloud-primitive-filter.cpp
- geom/apps/point-cloud-tests.cpp
- geom/apps/point-cloud-viewer.cpp
- geom/apps/rotate-image-3D.cpp
- geom/apps/show-extrinsic-calibration-grid.cpp
- geom/apps/show-scene.cpp
- geom/apps/simple-point-cloud-viewer.cpp
- io/apps/camera-param-io.cpp
- io/apps/convert.cpp
- io/apps/k2.cpp
- io/apps/multi-viewer.cpp
- io/apps/pipe.cpp
- io/apps/reset-bus.cpp
- io/apps/video-player.cpp
- markers/apps/camera-calibration-CameraCalibrationUtils.cpp
- markers/apps/camera-calibration-planar.cpp
- markers/apps/camera-calibration.cpp
- markers/apps/create-marker-grid-svg.cpp
- markers/apps/create-marker.cpp
- markers/apps/marker-detection.cpp
- qt/apps/camera-config.cpp
- qt/apps/color-picker.cpp
- qt/apps/image-compare.cpp
- qt/apps/viewer.cpp

## Code changes required (cvt/cvt8u/load<T> removal, ImgQ→Image)

- io/apps/create.cpp — `ImgBase *image = new ImgQ(create(...))` → `Image image = create(...)`, `.ptr()` for FixedConverter
- qt/apps/xv.cpp — `ImgQ o = ones(...)*100` → `Image o = ones(...)*100`, `.ptr()` for widget
- qt/apps/create-button-icon.cpp — `load<icl8u>()` → `icl::qt::load().as8u()`
- cv/apps/crop.cpp — `ImgQ image = qt::load(f[i])` → `Image image = load(f[i])`
- cv/apps/region-inspector.cpp — `cvt8u(icl::qt::levels(cvt(...)))` → `icl::qt::levels(Image(...)).as8u()` (name conflict with local `levels` variable)
- cv/apps/surf-detector.cpp — `load<icl8u>()` → `icl::qt::load().as8u()` (name conflict with local `load` function)
- cv/apps/lens-undistortion-calibration.cpp — `cvt()` → `Image()`
- cv/apps/lens-undistortion-calibration-UndistortionUtil.cpp — header swap only (Quick.h → Quick2.h)
- cv/apps/lens-undistortion-calibration-opencv.cpp — header swap only
- geom/apps/surf-based-object-tracking.cpp — `load<icl8u>()` → `icl::qt::load().as8u()`

## Also ported (library code, not apps/demos)

- qt/GUI.cpp — header swap only (Quick.h → Quick2.h)
- geom/SceneMouseHandler.cpp — header swap only
- geom/Segmentation3D.cpp — header swap only
- cv/TemplateTracker.cpp — header swap only
- physics/PhysicsPaper.cpp — header swap only
- physics/PhysicsPaper3.cpp — header swap only
