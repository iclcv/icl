# ICL Physics - demos, apps, examples

# ---- Demos ----
if _build_demos
  # Single-file demos
  executable('physics-car-demo',
    files('../demos/phyisics-car.cpp'),
    dependencies: [icl_physics_dep],
    install: false,
  )

  executable('physics-constraints-demo',
    files('../demos/phyisics-constraints.cpp'),
    dependencies: [icl_physics_dep],
    install: false,
  )

  executable('physics-paper3-demo',
    files('../demos/physics-paper3.cpp'),
    dependencies: [icl_physics_dep],
    install: false,
  )

  executable('physics-scene-demo',
    files('../demos/physics-scene.cpp'),
    dependencies: [icl_physics_dep],
    install: false,
  )

  # physics-maze is multi-file
  executable('physics-maze-demo',
    files(
      '../demos/physics-maze.cpp',
      '../demos/physics-maze-MazeObject.cpp',
    ),
    dependencies: [icl_physics_dep],
    install: false,
  )

  # physics-paper is multi-file
  executable('physics-paper-demo',
    files(
      '../demos/physics-paper.cpp',
      '../demos/physics-paper-SceneMultiCamCapturer.cpp',
    ),
    dependencies: [icl_physics_dep],
    install: false,
  )
endif
