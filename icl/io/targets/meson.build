# ICL IO - demos, apps, examples
#
# Note: ICL IO has no demos. The historic three (depth_img_endcoding_test,
# png_write_test, undistortion) were retired in Session 45 — the depth
# 11-bit roundtrip moved to tests/test-quick-io.cpp, png writing is
# already covered by Quick2.IO.save.load.png, and undistortion is
# accessible from every GenericGrabber-based app via the `@udist=file.xml`
# qualifier (see GenericGrabber.cpp ~line 273).

# ---- Apps ----
if _build_apps
  # Apps that do NOT need Qt
  executable('icl-jpg2cpp',
    files('../apps/jpg2cpp.cpp'),
    dependencies: [icl_io_dep],
    install: true,
  )

  # Apps needing libdc
  # icl-dcdeviceinfo retired Session 45 — `icl-pipe -i dc 0@info`
  # (or any GenericGrabber-driven app with the same qualifier) prints the
  # same property table on demand.
  if libdc_dep.found()
    executable('icl-dcclearisochannels',
      files('../apps/dcclearisochannels.cpp'),
      dependencies: [icl_io_dep],
      install: true,
    )
    executable('icl-reset-dc-bus',
      files('../apps/reset-dc-bus.cpp'),
      dependencies: [icl_io_dep],
      install: true,
    )
  endif

  # Apps needing Qt
  if qt_found
    _io_qt_apps = {
      'camera-param-io': [],
      'convert': [],
      'create': [],
      'multi-viewer': [],
      'pipe': [],
      'reset-bus': [],
      'video-player': [],
    }
    foreach name, extra_deps : _io_qt_apps
      executable('icl-' + name,
        files('../apps' / name + '.cpp'),
        dependencies: [icl_qt_dep, icl_io_dep] + extra_deps,
        install: true,
      )
    endforeach

    # icl-k2 retired Session 45 — bypassed Kinect2Grabber by talking to
    # libfreenect2 directly. Use `icl-pipe -i kinect2 0` (or any
    # GenericGrabber-driven app) instead, which goes through ICL's own
    # Kinect2Grabber backend.
  endif
endif
