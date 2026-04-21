  // includes
  
  
  GUI gui;
  Scene scene;
  CyclesRenderer cycles;

  init(){ 
    pa_init("...");
   
    Size size = pa("-size").as<Size>();
    scene.load(pa("-scene"));
    
    // either scene has a camera already else
    scene.addCamera(Camera::lookAt(size, ...));
    cycles.init(&scene);

    gui << Canvas3D["canvas"](size) << ... << Show();
    gui["canvas"].install(new MouseHandler(handleMouse));
  }

  void run(){
    Image img = cycles.render();
    gui["canvas"] = img; // this will trigger a redraw of the Canvas3D
    // this will also trigger the GL callback for the reference view, which can share the same camera as the renderer
  }

  int main(int argc, char **argv) {
    return ICLApp(argc, argv, "...", init, run).exec();
  }