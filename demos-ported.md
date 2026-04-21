# Demos ported from Quick.h/Common.h → Quick2.h/Common2.h

## Trivial (header swap only)

- core/demos/colorspace.cpp
- cv/demos/corner-detection-css.cpp
- cv/demos/flood-filler.cpp
- cv/demos/heart-rate-detector.cpp
- cv/demos/mean-shift.cpp
- cv/demos/orb-feature-detection.cpp
- cv/demos/region-curvature.cpp
- cv/demos/region-detection.cpp
- cv/demos/simple-blob-searcher.cpp
- cv/demos/vector-tracker.cpp
- filter/demos/bilateral-filter-op.cpp
- filter/demos/canny-op.cpp
- filter/demos/convolution-op.cpp
- filter/demos/dither-op.cpp
- filter/demos/fft.cpp
- filter/demos/gabor-op.cpp
- filter/demos/temporal-smoothing.cpp
- geom/demos/animated-grid.cpp
- geom/demos/camera.cpp
- geom/demos/cycles-overlay-viewer.cpp
- geom/demos/cycles-scene-viewer.cpp
- geom/demos/generic-texture-coords.cpp
- geom/demos/gl-renderer-test.cpp
- geom/demos/kinect-depth-image-segmentation.cpp
- geom/demos/kinect-euclidean-blob-segmentation.cpp
- geom/demos/kinect-normals.cpp
- geom/demos/kinect-pointcloud.cpp
- geom/demos/kinect-segmentation.cpp
- geom/demos/offscreen-rendering.cpp
- geom/demos/plot-widget-3D.cpp
- geom/demos/ray-cast-octree.cpp
- geom/demos/rgbd-mapping.cpp
- geom/demos/scene-graph.cpp
- geom/demos/scene-object.cpp
- geom/demos/scene-shadows.cpp
- geom/demos/simplex-2D.cpp
- geom/demos/simplex-3D.cpp
- geom/demos/superquadric.cpp
- geom2/demos/cycles-overlay-viewer.cpp
- geom2/demos/cycles-scene-viewer.cpp
- geom2/demos/geom2-hello.cpp
- geom2/demos/scene-to-pointcloud.cpp
- io/demos/depth_img_endcoding_test.cpp
- io/demos/undistortion.cpp
- markers/demos/multi-cam-marker-demo.cpp
- math/demos/k-means.cpp
- math/demos/llm-1D.cpp
- math/demos/octree.cpp
- math/demos/quad-tree.cpp
- physics/demos/phyisics-car.cpp
- physics/demos/phyisics-constraints.cpp
- physics/demos/physics-maze.cpp
- physics/demos/physics-scene.cpp
- qt/demos/chromaticity-space.cpp
- qt/demos/configurable-gui.cpp
- qt/demos/define-rects.cpp
- qt/demos/game-of-life.cpp
- qt/demos/interactive-filter.cpp
- qt/demos/mandelbrot.cpp
- qt/demos/plot-component.cpp

## Code changes required (cvt/cvt8u/load<T> removal, ImgQ→Image)

- qt/examples/quick.cpp — `ImgQ` → `Image`
- qt/demos/onscreen-button.cpp — `ImgQ` → `Image`, `cvt()` removed, `.ptr()` for widget API
- io/demos/png_write_test.cpp — header swap only (Quick.h → Quick2.h)
- markers/demos/simple-marker-demo.cpp — header swap only (Common.h → Common2.h)
- cv/demos/hough-line.cpp — `load<icl8u>()` → `icl::qt::load().as8u()` (name conflict with local `load` variable)
- cv/demos/template-matching.cpp — `cvt8u(ones(...)*255)` → `(ones(...)*255).as8u()`
- filter/demos/affine-op.cpp — `cvt8u(scale(create(...)))` → `scale(create(...)).as8u()`
- filter/demos/warp-op.cpp — `load(pa("-w"))` → `load(pa("-w")).as32f()` (WarpOp needs Img32f)
- geom/demos/swiss-ranger.cpp — `cvt(moBuf)` → `Image(*moBuf).as32f()`
- geom/demos/texture-cube.cpp — `cvt8u(icl::qt::scale(...))` → `icl::qt::scale(...).as8u()` (name conflict with SceneObject::scale)
- physics/demos/physics-maze-MazeObject.cpp — `Img32f topImage` → `Image topImage` (member + header change)
- physics/demos/physics-paper.cpp — `load<icl8u>()` → `icl::qt::load().as8u()`
- physics/demos/physics-paper3.cpp — `load<icl8u>()` → `icl::qt::load().as8u()`
- qt/examples/model-fitting.cpp — header swap only

## NOT ported (use ImgQ with heavy pixel access)

- core/demos/canvas.cpp
- core/demos/pseudo-color.cpp
- cv/demos/signature-extraction.cpp
- math/demos/llm-2D.cpp
- math/demos/polynomial-regression.cpp
