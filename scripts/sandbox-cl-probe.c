// Minimal OpenCL kernel-compile probe for the mscc sandbox investigation.
// Builds a trivial kernel — this is what triggers the Metal frontend to write
// its `opencl_c.pcm` module cache under /var/folders/.../com.apple.metalfe/,
// the write that fails with EPERM inside the sandbox. RUN FROM THE HOST under
// scripts/sandbox-cl-smoke.sh, which harvests the resulting sandbox denials.
//
// Build (host or sandbox): cc -x c sandbox-cl-probe.c -framework OpenCL -o probe
#include <OpenCL/opencl.h>
#include <stdio.h>

int main(void) {
  cl_platform_id plat = 0;
  if (clGetPlatformIDs(1, &plat, 0) != CL_SUCCESS) { printf("NO PLATFORM\n"); return 2; }
  cl_device_id dev = 0;
  if (clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 1, &dev, 0) != CL_SUCCESS) {
    if (clGetDeviceIDs(plat, CL_DEVICE_TYPE_DEFAULT, 1, &dev, 0) != CL_SUCCESS) {
      printf("NO DEVICE\n"); return 2;
    }
  }
  cl_int err = 0;
  cl_context ctx = clCreateContext(0, 1, &dev, 0, 0, &err);
  if (!ctx) { printf("NO CONTEXT err=%d\n", err); return 2; }
  const char *src = "__kernel void k(__global float *a){ a[get_global_id(0)] = 1.0f; }";
  cl_program prog = clCreateProgramWithSource(ctx, 1, &src, 0, &err);
  err = clBuildProgram(prog, 1, &dev, 0, 0, 0);
  if (err == CL_SUCCESS) { printf("OK: kernel built (opencl_c cache reachable)\n"); return 0; }
  char buf[8192] = {0};
  clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, sizeof(buf), buf, 0);
  printf("BUILD FAILED err=%d:\n%s\n", err, buf);
  return 1;
}
