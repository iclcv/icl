# Quick2 Porting Progress

## Demos

| File | Status | Notes |
|------|--------|-------|
| core/demos/canvas.cpp | :wastebasket: Removed | Deleted along with AbstractCanvas.h/.cpp (unused outside demo) |
| core/demos/colorspace.cpp | :white_check_mark: Done | Header swap + simplified `cc(image, gui["fmt"])` via DataStore string→parse fallback |
| ~~core~~/filter/demos/pseudo-color.cpp | :white_check_mark: Done | Moved to filter, PseudoColorConverter→PseudoColorOp (UnaryOp), Quick2 rewrite |
| cv/demos/corner-detection-css.cpp | :white_check_mark: Done | Header swap |
| cv/demos/flood-filler.cpp | :white_check_mark: Done | Header swap |
| cv/demos/heart-rate-detector.cpp | :white_check_mark: Done | Header swap |
| cv/demos/hough-line.cpp | :white_check_mark: Done | `load<icl8u>()` -> `icl::qt::load().as8u()` |
| cv/demos/mean-shift.cpp | :white_check_mark: Done | Header swap |
| cv/demos/orb-feature-detection.cpp | :white_check_mark: Done | Header swap |
| cv/demos/region-curvature.cpp | :white_check_mark: Done | Header swap |
| cv/demos/region-detection.cpp | :white_check_mark: Done | Header swap |
| cv/demos/signature-extraction.cpp | :x: TODO | Heavy ImgQ pixel access — needs DrawTarget rewrite |
| cv/demos/simple-blob-searcher.cpp | :white_check_mark: Done | Header swap |
| cv/demos/template-matching.cpp | :white_check_mark: Done | `cvt8u()` -> `.as8u()` |
| cv/demos/vector-tracker.cpp | :white_check_mark: Done | Header swap |
| filter/demos/affine-op.cpp | :white_check_mark: Done | `cvt8u()` -> `.as8u()` |
| filter/demos/bilateral-filter-op.cpp | :white_check_mark: Done | Header swap |
| filter/demos/canny-op.cpp | :white_check_mark: Done | Header swap |
| filter/demos/convolution-op.cpp | :white_check_mark: Done | Header swap |
| filter/demos/dither-op.cpp | :white_check_mark: Done | Header swap |
| filter/demos/fft.cpp | :white_check_mark: Done | Header swap |
| filter/demos/gabor-op.cpp | :white_check_mark: Done | Header swap |
| filter/demos/temporal-smoothing.cpp | :white_check_mark: Done | Header swap |
| filter/demos/warp-op.cpp | :white_check_mark: Done | `load()` -> `load().as32f()` |
| geom/demos/animated-grid.cpp | :white_check_mark: Done | Header swap |
| geom/demos/camera.cpp | :white_check_mark: Done | Header swap |
| geom/demos/cycles-overlay-viewer.cpp | :white_check_mark: Done | Header swap |
| geom/demos/cycles-renderer-test.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| geom/demos/cycles-scene-viewer.cpp | :white_check_mark: Done | Header swap |
| geom/demos/generic-texture-coords.cpp | :white_check_mark: Done | Header swap |
| geom/demos/gl-renderer-test.cpp | :white_check_mark: Done | Header swap |
| geom/demos/kinect-depth-image-segmentation.cpp | :white_check_mark: Done | Header swap |
| geom/demos/kinect-euclidean-blob-segmentation.cpp | :white_check_mark: Done | Header swap |
| geom/demos/kinect-normals.cpp | :white_check_mark: Done | Header swap |
| geom/demos/kinect-pointcloud.cpp | :white_check_mark: Done | Header swap |
| geom/demos/kinect-segmentation.cpp | :white_check_mark: Done | Header swap |
| geom/demos/offscreen-rendering.cpp | :white_check_mark: Done | Header swap |
| geom/demos/plot-widget-3D.cpp | :white_check_mark: Done | Header swap |
| geom/demos/ray-cast-octree.cpp | :white_check_mark: Done | Header swap |
| geom/demos/rgbd-mapping.cpp | :white_check_mark: Done | Header swap |
| geom/demos/scene-graph.cpp | :white_check_mark: Done | Header swap |
| geom/demos/scene-object.cpp | :white_check_mark: Done | Header swap |
| geom/demos/scene-shadows.cpp | :white_check_mark: Done | Header swap |
| geom/demos/simplex-2D.cpp | :white_check_mark: Done | Header swap |
| geom/demos/simplex-3D.cpp | :white_check_mark: Done | Header swap |
| geom/demos/superquadric.cpp | :white_check_mark: Done | Header swap |
| geom/demos/swiss-ranger.cpp | :white_check_mark: Done | `cvt()` -> `Image().as32f()` |
| geom/demos/texture-cube.cpp | :white_check_mark: Done | `cvt8u()` -> `.as8u()` (name conflict) |
| geom2/demos/cycles-overlay-viewer.cpp | :white_check_mark: Done | Header swap |
| geom2/demos/cycles-renderer-test.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| geom2/demos/cycles-scene-viewer.cpp | :white_check_mark: Done | Header swap |
| geom2/demos/geom2-hello.cpp | :white_check_mark: Done | Header swap |
| geom2/demos/scene-to-pointcloud.cpp | :white_check_mark: Done | Header swap |
| io/demos/depth_img_endcoding_test.cpp | :white_check_mark: Done | Header swap |
| io/demos/png_write_test.cpp | :white_check_mark: Done | Header swap |
| io/demos/undistortion.cpp | :white_check_mark: Done | Header swap |
| markers/demos/multi-cam-marker-demo.cpp | :white_check_mark: Done | Header swap |
| markers/demos/simple-marker-demo.cpp | :white_check_mark: Done | Header swap |
| math/demos/k-means.cpp | :white_check_mark: Done | Header swap |
| math/demos/llm-1D.cpp | :white_check_mark: Done | Header swap |
| math/demos/llm-2D.cpp | :x: TODO | Heavy ImgQ pixel access — needs DrawTarget rewrite |
| math/demos/octree.cpp | :white_check_mark: Done | Header swap |
| math/demos/polynomial-regression.cpp | :x: TODO | Heavy ImgQ pixel access — needs DrawTarget rewrite |
| math/demos/quad-tree.cpp | :white_check_mark: Done | Header swap |
| physics/demos/phyisics-car.cpp | :white_check_mark: Done | Header swap |
| physics/demos/phyisics-constraints.cpp | :white_check_mark: Done | Header swap |
| physics/demos/physics-maze.cpp | :white_check_mark: Done | Header swap |
| physics/demos/physics-maze-MazeObject.cpp | :white_check_mark: Done | `Img32f` -> `Image` |
| physics/demos/physics-paper.cpp | :white_check_mark: Done | `load<icl8u>()` -> `icl::qt::load().as8u()` |
| physics/demos/physics-paper-SceneMultiCamCapturer.cpp | :heavy_minus_sign: N/A | Helper file, no Quick.h/Common.h |
| physics/demos/physics-paper3.cpp | :white_check_mark: Done | `load<icl8u>()` -> `icl::qt::load().as8u()` |
| physics/demos/physics-scene.cpp | :white_check_mark: Done | Header swap |
| qt/demos/chromaticity-space.cpp | :white_check_mark: Done | Header swap |
| qt/demos/configurable-gui.cpp | :white_check_mark: Done | Header swap |
| qt/demos/define-rects.cpp | :white_check_mark: Done | Header swap |
| qt/demos/game-of-life.cpp | :white_check_mark: Done | Header swap |
| qt/demos/interactive-filter.cpp | :white_check_mark: Done | Header swap |
| qt/demos/mandelbrot.cpp | :white_check_mark: Done | Header swap |
| qt/demos/onscreen-button.cpp | :white_check_mark: Done | `ImgQ` -> `Image`, `cvt()` removed |
| qt/demos/plot-component.cpp | :white_check_mark: Done | Header swap |

## Apps

| File | Status | Notes |
|------|--------|-------|
| cv/apps/crop.cpp | :white_check_mark: Done | `ImgQ` -> `Image` |
| cv/apps/lens-undistortion-calibration.cpp | :white_check_mark: Done | `cvt()` -> `Image()` |
| cv/apps/lens-undistortion-calibration-UndistortionUtil.cpp | :white_check_mark: Done | Header swap |
| cv/apps/lens-undistortion-calibration-opencv.cpp | :white_check_mark: Done | Header swap |
| cv/apps/region-inspector.cpp | :white_check_mark: Done | `cvt8u()` -> `.as8u()` (name conflict) |
| cv/apps/surf-detector.cpp | :white_check_mark: Done | `load<icl8u>()` -> `icl::qt::load().as8u()` |
| filter/apps/color-segmentation.cpp | :white_check_mark: Done | Header swap |
| filter/apps/filter-array.cpp | :white_check_mark: Done | Header swap |
| filter/apps/local-thresh.cpp | :white_check_mark: Done | Header swap |
| filter/apps/rectify-image.cpp | :white_check_mark: Done | Header swap |
| geom/apps/compute-relative-camera-transform.cpp | :white_check_mark: Done | Header swap |
| geom/apps/depth-camera-simulator.cpp | :white_check_mark: Done | Header swap |
| geom/apps/fix-kinect-calibration.cpp | :white_check_mark: Done | Header swap |
| geom/apps/icp3d-test.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| geom/apps/kinect-recorder.cpp | :white_check_mark: Done | Header swap |
| geom/apps/point-cloud-creator.cpp | :white_check_mark: Done | Header swap |
| geom/apps/point-cloud-define-world-frame.cpp | :white_check_mark: Done | Header swap |
| geom/apps/point-cloud-pipe.cpp | :white_check_mark: Done | Header swap |
| geom/apps/point-cloud-primitive-filter.cpp | :white_check_mark: Done | Header swap |
| geom/apps/point-cloud-tests.cpp | :white_check_mark: Done | Header swap |
| geom/apps/point-cloud-viewer.cpp | :white_check_mark: Done | Header swap |
| geom/apps/rotate-image-3D.cpp | :white_check_mark: Done | Header swap |
| geom/apps/show-extrinsic-calibration-grid.cpp | :white_check_mark: Done | Header swap |
| geom/apps/show-scene.cpp | :white_check_mark: Done | Header swap |
| geom/apps/simple-point-cloud-viewer.cpp | :white_check_mark: Done | Header swap |
| geom/apps/surf-based-object-tracking.cpp | :white_check_mark: Done | `load<icl8u>()` -> `icl::qt::load().as8u()` |
| io/apps/camera-param-io.cpp | :white_check_mark: Done | Header swap |
| io/apps/convert.cpp | :white_check_mark: Done | Header swap |
| io/apps/create.cpp | :white_check_mark: Done | Rewritten with `fixed_convert()` + `save()` |
| io/apps/dcclearisochannels.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| io/apps/dcdeviceinfo.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| io/apps/jpg2cpp.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| io/apps/k2.cpp | :white_check_mark: Done | Header swap |
| io/apps/multi-viewer.cpp | :white_check_mark: Done | Header swap |
| io/apps/pipe.cpp | :white_check_mark: Done | Header swap |
| io/apps/reset-bus.cpp | :white_check_mark: Done | Header swap |
| io/apps/reset-dc-bus.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| io/apps/video-player.cpp | :white_check_mark: Done | Header swap |
| markers/apps/camera-calibration.cpp | :white_check_mark: Done | Header swap |
| markers/apps/camera-calibration-CameraCalibrationUtils.cpp | :white_check_mark: Done | Header swap |
| markers/apps/camera-calibration-planar.cpp | :white_check_mark: Done | Header swap |
| markers/apps/camera-calibration-planar-GridIndicatorObject.cpp | :heavy_minus_sign: N/A | Helper file, no Quick.h/Common.h |
| markers/apps/camera-calibration-planar-PlanarCalibrationTools.cpp | :heavy_minus_sign: N/A | Helper file, no Quick.h/Common.h |
| markers/apps/create-marker.cpp | :white_check_mark: Done | Header swap |
| markers/apps/create-marker-grid-svg.cpp | :white_check_mark: Done | Header swap |
| markers/apps/marker-detection.cpp | :white_check_mark: Done | Header swap |
| qt/apps/camera-config.cpp | :white_check_mark: Done | Header swap |
| qt/apps/color-picker.cpp | :white_check_mark: Done | Header swap |
| qt/apps/create-button-icon.cpp | :white_check_mark: Done | `load<icl8u>()` -> `icl::qt::load().as8u()` |
| qt/apps/gui-assignment-info.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| qt/apps/image-compare.cpp | :white_check_mark: Done | Header swap |
| qt/apps/viewer.cpp | :white_check_mark: Done | Header swap |
| qt/apps/xv.cpp | :white_check_mark: Done | `ImgQ` -> `Image` |
| utils/apps/configurable-info.cpp | :white_check_mark: Checked | No Quick.h/Common.h — no changes needed |

## Examples

| File | Status | Notes |
|------|--------|-------|
| qt/examples/quick.cpp | :white_check_mark: Done | `ImgQ` -> `Image` |
| qt/examples/model-fitting.cpp | :white_check_mark: Done | Header swap |
| core/examples/img.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| math/examples/levenberg-marquardt.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| utils/examples/opencl_example.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| utils/examples/any.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| utils/examples/progarg.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| utils/examples/config-file.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |
| utils/examples/regex-find.cpp | :heavy_minus_sign: N/A | No Quick.h/Common.h |

## Library Code

| File | Status | Notes |
|------|--------|-------|
| qt/GUI.cpp | :white_check_mark: Done | Header swap |
| geom/SceneMouseHandler.cpp | :white_check_mark: Done | Header swap |
| geom/Segmentation3D.cpp | :white_check_mark: Done | Header swap |
| cv/TemplateTracker.cpp | :white_check_mark: Done | Header swap |
| physics/PhysicsPaper.cpp | :white_check_mark: Done | Header swap |
| physics/PhysicsPaper3.cpp | :white_check_mark: Done | Header swap |
| geom/Scene.cpp | :x: TODO | Uses ImgQ |
| markers/FiducialDetectorPluginICL1.cpp | :x: TODO | Uses ImgQ |
| qt/DrawWidget.h | :x: TODO | Uses ImgQ |

## Summary

| Category | Done | TODO | N/A | Total |
|----------|------|------|-----|-------|
| Demos | 68 | 5 | 3 | 76 |
| Apps | 42 | 0 | 7 | 49 |
| Examples | 2 | 0 | 7 | 9 |
| Library | 6 | 3 | 0 | 9 |
| **Total** | **118** | **8** | **17** | **143** |
