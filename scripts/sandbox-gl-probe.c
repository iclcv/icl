// Headless GL probe — create an OFFSCREEN OpenGL context (no window) via CGL,
// render one cleared FBO pixel, read it back. This is the minimal stand-in for
// ICL's GLSceneCapture(ownContext=true) / QOffscreenSurface path.
//
// Exit 0 + "OK pixel=64,128,191,255" => headless GL works under the active sandbox.
// Used by scripts/sandbox-gl-smoke.sh; safe to delete (regenerated/ignored).
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <stdio.h>

int main(void) {
  CGLPixelFormatAttribute attrs[] = {
    kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
    kCGLPFAColorSize,     (CGLPixelFormatAttribute)24,
    kCGLPFAAccelerated,
    (CGLPixelFormatAttribute)0
  };
  CGLPixelFormatObj pix = 0; GLint n = 0;
  CGLError e = CGLChoosePixelFormat(attrs, &pix, &n);
  if (e || !pix) { printf("FAIL CGLChoosePixelFormat err=%d (%s)\n", e, CGLErrorString(e)); return 2; }
  CGLContextObj ctx = 0;
  e = CGLCreateContext(pix, 0, &ctx);
  if (e || !ctx) { printf("FAIL CGLCreateContext err=%d (%s)\n", e, CGLErrorString(e)); return 3; }
  CGLSetCurrentContext(ctx);

  const GLubyte *ver = glGetString(GL_VERSION), *ren = glGetString(GL_RENDERER);
  printf("OK context: GL_VERSION=%s | GL_RENDERER=%s\n",
         ver ? (const char*)ver : "(null)", ren ? (const char*)ren : "(null)");

  GLuint fbo = 0, tex = 0;
  glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glGenTextures(1, &tex);     glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("FAIL framebuffer incomplete\n"); return 4;
  }
  glViewport(0, 0, 8, 8);
  glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  unsigned char px[4] = {0};
  glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
  printf("OK pixel=%u,%u,%u,%u (expect ~64,128,191,255)\n", px[0], px[1], px[2], px[3]);
  return 0;
}
