// Headless Qt GL probe — the exact path ICL's GLSceneCapture(ownContext=true) uses:
// QGuiApplication (cocoa platform) + QOffscreenSurface + QOpenGLContext, render one
// FBO pixel offscreen (no window). Confirms ICL's real offscreen-GL path works under
// the sandbox and enumerates the Qt-cocoa-init mach set. Used by scripts/sandbox-gl-smoke.sh --qt.
#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QSurfaceFormat>
#include <cstdio>

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);

  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setVersion(3, 2);
  fmt.setProfile(QSurfaceFormat::CoreProfile);

  QOffscreenSurface surf;
  surf.setFormat(fmt);
  surf.create();
  if (!surf.isValid()) { printf("FAIL QOffscreenSurface invalid\n"); return 2; }

  QOpenGLContext ctx;
  ctx.setFormat(fmt);
  if (!ctx.create())          { printf("FAIL QOpenGLContext::create()\n"); return 3; }
  if (!ctx.makeCurrent(&surf)){ printf("FAIL makeCurrent()\n"); return 4; }

  QOpenGLFunctions *f = ctx.functions();
  const GLubyte *ver = f->glGetString(GL_VERSION);
  printf("OK Qt offscreen context: GL_VERSION=%s\n", ver ? (const char *)ver : "(null)");

  GLuint fbo = 0, tex = 0;
  f->glGenFramebuffers(1, &fbo); f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  f->glGenTextures(1, &tex);     f->glBindTexture(GL_TEXTURE_2D, tex);
  f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
  f->glViewport(0, 0, 8, 8);
  f->glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
  f->glClear(GL_COLOR_BUFFER_BIT);
  unsigned char px[4] = {0};
  f->glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
  printf("OK pixel=%u,%u,%u,%u (expect ~64,128,191,255)\n", px[0], px[1], px[2], px[3]);
  return 0;
}
