// ICL Raytracer — OpenCL BVH traversal + shading kernel

// ---- Struct layouts (must match C++ RaytracerTypes.h) ----

typedef struct { float x, y, z, _pad; } RTFloat3;
typedef struct { float x, y, z, w; } RTFloat4;
typedef struct { float cols[4][4]; } RTMat4;

typedef struct {
  RTFloat3 position;
  RTFloat3 normal;
  RTFloat4 color;
} RTVertex;

typedef struct {
  unsigned int v0, v1, v2;
  unsigned int materialIndex;
} RTTriangle;

typedef struct {
  RTFloat4 diffuseColor;
  RTFloat4 specularColor;
  RTFloat4 emission;
  float shininess;
  float reflectivity;
  float _pad[2];
} RTMaterial;

typedef struct {
  RTFloat3 position;
  RTFloat4 ambient;
  RTFloat4 diffuse;
  RTFloat4 specular;
  RTFloat3 spotDirection;
  RTFloat3 attenuation;
  float spotCutoff;
  float spotExponent;
  int on;
  float _pad;
} RTLight;

typedef struct {
  RTFloat3 boundsMin;
  RTFloat3 boundsMax;
  unsigned int rightChildIdx;
  unsigned int triOffset;
  unsigned int triCount;
  unsigned int _pad;
} BVHNode;

typedef struct {
  RTMat4 transform;
  RTMat4 transformInverse;
  int bvhNodeOffset;
  int triIndexOffset;
  int vertexOffset;
  int triangleOffset;
  int materialIndex;
  int numBVHNodes;
  int _pad[2];
} FlatInstance;

// ---- Vector helpers ----

inline float3 f3(RTFloat3 v) { return (float3)(v.x, v.y, v.z); }
inline float3 f3v(RTFloat4 v) { return (float3)(v.x, v.y, v.z); }

inline float3 transformPoint(__global const RTMat4 *m, float3 p) {
  return (float3)(
    m->cols[0][0]*p.x + m->cols[1][0]*p.y + m->cols[2][0]*p.z + m->cols[3][0],
    m->cols[0][1]*p.x + m->cols[1][1]*p.y + m->cols[2][1]*p.z + m->cols[3][1],
    m->cols[0][2]*p.x + m->cols[1][2]*p.y + m->cols[2][2]*p.z + m->cols[3][2]
  );
}

inline float3 transformDir(__global const RTMat4 *m, float3 d) {
  return (float3)(
    m->cols[0][0]*d.x + m->cols[1][0]*d.y + m->cols[2][0]*d.z,
    m->cols[0][1]*d.x + m->cols[1][1]*d.y + m->cols[2][1]*d.z,
    m->cols[0][2]*d.x + m->cols[1][2]*d.y + m->cols[2][2]*d.z
  );
}

inline float3 transformNormal(__global const RTMat4 *invT, float3 n) {
  return (float3)(
    invT->cols[0][0]*n.x + invT->cols[0][1]*n.y + invT->cols[0][2]*n.z,
    invT->cols[1][0]*n.x + invT->cols[1][1]*n.y + invT->cols[1][2]*n.z,
    invT->cols[2][0]*n.x + invT->cols[2][1]*n.y + invT->cols[2][2]*n.z
  );
}

// ---- Fast RNG (xorshift) ----

inline uint rngNext(uint *state) {
  *state ^= *state << 13;
  *state ^= *state >> 17;
  *state ^= *state << 5;
  return *state;
}

inline float rngFloat(uint *state) {
  return (float)(rngNext(state) & 0xFFFFFF) / (float)0x1000000;
}

// ---- Ray-AABB slab test ----

inline bool intersectAABB(float3 origin, float3 invDir, float3 bmin, float3 bmax,
                          float tMin, float tMax) {
  float3 t1 = (bmin - origin) * invDir;
  float3 t2 = (bmax - origin) * invDir;
  float3 tNear = fmin(t1, t2);
  float3 tFar  = fmax(t1, t2);
  float tn = fmax(fmax(tNear.x, tNear.y), tNear.z);
  float tf = fmin(fmin(tFar.x, tFar.y), tFar.z);
  return tn <= tf && tf >= tMin && tn <= tMax;
}

// ---- Moller-Trumbore ray-triangle ----

inline bool intersectTriangle(float3 origin, float3 dir,
                               float3 v0, float3 v1, float3 v2,
                               float tMin, float tMax,
                               float *t, float *u, float *v) {
  float3 e1 = v1 - v0;
  float3 e2 = v2 - v0;
  float3 h = cross(dir, e2);
  float a = dot(e1, h);
  if (a > -1e-8f && a < 1e-8f) return false;
  float f = 1.0f / a;
  float3 s = origin - v0;
  *u = f * dot(s, h);
  if (*u < 0.0f || *u > 1.0f) return false;
  float3 q = cross(s, e1);
  *v = f * dot(dir, q);
  if (*v < 0.0f || *u + *v > 1.0f) return false;
  *t = f * dot(e2, q);
  return *t >= tMin && *t <= tMax;
}

// ---- BVH traversal for one instance ----

inline bool traceBVH(__global const BVHNode *nodes, int nodeOffset, int numNodes,
                     __global const unsigned int *triIndices, int triIndexOffset,
                     __global const RTVertex *vertices, int vertexOffset,
                     __global const RTTriangle *triangles, int triangleOffset,
                     float3 origin, float3 dir, float3 invDir,
                     float tMin, float *tMax,
                     unsigned int *hitTriIdx, float *hitU, float *hitV) {
  if (numNodes == 0) return false;

  unsigned int stack[32];
  int stackPtr = 0;
  stack[stackPtr++] = 0;
  bool anyHit = false;

  while (stackPtr > 0) {
    unsigned int idx = stack[--stackPtr];
    __global const BVHNode *node = &nodes[nodeOffset + idx];

    if (!intersectAABB(origin, invDir, f3(node->boundsMin), f3(node->boundsMax), tMin, *tMax))
      continue;

    if (node->triCount > 0) {
      for (unsigned int i = 0; i < node->triCount; i++) {
        unsigned int ti = triIndices[triIndexOffset + node->triOffset + i];
        __global const RTTriangle *tri = &triangles[triangleOffset + ti];
        float3 va = f3(vertices[vertexOffset + tri->v0].position);
        float3 vb = f3(vertices[vertexOffset + tri->v1].position);
        float3 vc = f3(vertices[vertexOffset + tri->v2].position);
        float t, u, v;
        if (intersectTriangle(origin, dir, va, vb, vc, tMin, *tMax, &t, &u, &v)) {
          *tMax = t;
          *hitTriIdx = ti;
          *hitU = u;
          *hitV = v;
          anyHit = true;
        }
      }
    } else {
      if (stackPtr < 31) stack[stackPtr++] = node->rightChildIdx;
      if (stackPtr < 31) stack[stackPtr++] = idx + 1;
    }
  }
  return anyHit;
}

// ---- Trace scene: find closest hit across all instances ----

inline int traceScene(__global const BVHNode *bvhNodes,
                      __global const unsigned int *triIndices,
                      __global const RTVertex *vertices,
                      __global const RTTriangle *triangles,
                      __global const FlatInstance *instances, int numInstances,
                      float3 origin, float3 dir,
                      unsigned int *outTri, float *outU, float *outV) {
  float closestT = 1e30f;
  int closestInst = -1;

  for (int i = 0; i < numInstances; i++) {
    __global const FlatInstance *inst = &instances[i];
    float3 localOrigin = transformPoint(&inst->transformInverse, origin);
    float3 localDir = transformDir(&inst->transformInverse, dir);
    float dirLen = length(localDir);
    if (dirLen < 1e-10f) continue;
    localDir /= dirLen;
    float3 invDir = 1.0f / select(localDir, (float3)(1e-10f), fabs(localDir) < 1e-10f);

    float tMax = closestT / dirLen;
    unsigned int hitTri; float hu, hv;
    if (traceBVH(bvhNodes, inst->bvhNodeOffset, inst->numBVHNodes,
                 triIndices, inst->triIndexOffset,
                 vertices, inst->vertexOffset,
                 triangles, inst->triangleOffset,
                 localOrigin, localDir, invDir, 0.5f / dirLen, &tMax,
                 &hitTri, &hu, &hv)) {
      float worldT = tMax * dirLen;
      if (worldT < closestT) {
        closestT = worldT;
        closestInst = i;
        *outTri = hitTri;
        *outU = hu;
        *outV = hv;
      }
    }
  }
  return closestInst;
}

// ---- Shadow test ----

inline bool traceShadow(__global const BVHNode *nodes,
                        __global const unsigned int *triIndices,
                        __global const RTVertex *vertices,
                        __global const RTTriangle *triangles,
                        __global const FlatInstance *instances, int numInstances,
                        float3 origin, float3 lightDir, float maxDist) {
  for (int i = 0; i < numInstances; i++) {
    __global const FlatInstance *inst = &instances[i];
    float3 localOrigin = transformPoint(&inst->transformInverse, origin);
    float3 localDir = transformDir(&inst->transformInverse, lightDir);
    float dirLen = length(localDir);
    if (dirLen < 1e-10f) continue;
    localDir /= dirLen;
    float3 invDir = 1.0f / select(localDir, (float3)(1e-10f), fabs(localDir) < 1e-10f);

    float tMax = maxDist / dirLen;
    unsigned int dummy; float du, dv;
    if (traceBVH(nodes, inst->bvhNodeOffset, inst->numBVHNodes,
                 triIndices, inst->triIndexOffset,
                 vertices, inst->vertexOffset,
                 triangles, inst->triangleOffset,
                 localOrigin, localDir, invDir, 0.5f / dirLen, &tMax,
                 &dummy, &du, &dv))
      return true;
  }
  return false;
}

// ---- Interpolate surface hit ----

inline void interpolateHit(__global const RTVertex *vertices, int vertexOffset,
                           __global const RTTriangle *triangles, int triangleOffset,
                           __global const FlatInstance *inst,
                           unsigned int triIdx, float u, float v,
                           float3 *outPos, float3 *outNormal, float3 *outColor) {
  __global const RTTriangle *tri = &triangles[triangleOffset + triIdx];
  __global const RTVertex *v0 = &vertices[vertexOffset + tri->v0];
  __global const RTVertex *v1 = &vertices[vertexOffset + tri->v1];
  __global const RTVertex *v2 = &vertices[vertexOffset + tri->v2];

  float w0 = 1.0f - u - v, w1 = u, w2 = v;

  float3 localNormal = (float3)(
    v0->normal.x*w0 + v1->normal.x*w1 + v2->normal.x*w2,
    v0->normal.y*w0 + v1->normal.y*w1 + v2->normal.y*w2,
    v0->normal.z*w0 + v1->normal.z*w1 + v2->normal.z*w2
  );
  *outNormal = normalize(transformNormal(&inst->transformInverse, localNormal));

  *outColor = (float3)(
    v0->color.x*w0 + v1->color.x*w1 + v2->color.x*w2,
    v0->color.y*w0 + v1->color.y*w1 + v2->color.y*w2,
    v0->color.z*w0 + v1->color.z*w1 + v2->color.z*w2
  );

  float3 localPos = (float3)(
    v0->position.x*w0 + v1->position.x*w1 + v2->position.x*w2,
    v0->position.y*w0 + v1->position.y*w1 + v2->position.y*w2,
    v0->position.z*w0 + v1->position.z*w1 + v2->position.z*w2
  );
  *outPos = transformPoint(&inst->transform, localPos);
}

// ---- Cosine-weighted hemisphere sample ----

inline float3 randomHemisphere(float3 N, uint *rng) {
  float u1 = rngFloat(rng);
  float u2 = rngFloat(rng);
  float r = sqrt(u1);
  float theta = 2.0f * 3.14159265f * u2;
  float x = r * cos(theta);
  float y = r * sin(theta);
  float z = sqrt(fmax(0.0f, 1.0f - u1));

  float3 up = fabs(N.y) < 0.999f ? (float3)(0,1,0) : (float3)(1,0,0);
  float3 tangent = normalize(cross(up, N));
  float3 bitangent = cross(N, tangent);
  return normalize(tangent * x + bitangent * y + N * z);
}

// ---- Direct lighting (Blinn-Phong) ----

inline float3 directLight(float3 hitPos, float3 N, float3 dir, float3 baseColor,
                           __global const RTMaterial *mat,
                           __global const RTLight *lights, int numLights,
                           __global const BVHNode *bvhNodes,
                           __global const unsigned int *triIndices,
                           __global const RTVertex *vertices,
                           __global const RTTriangle *triangles,
                           __global const FlatInstance *instances, int numInstances) {
  float3 color = (float3)(0, 0, 0);
  for (int li = 0; li < numLights; li++) {
    if (!lights[li].on) continue;
    float3 lightPos = f3(lights[li].position);
    float3 L = lightPos - hitPos;
    float dist = length(L);
    if (dist < 1e-6f) continue;
    L /= dist;

    float NdotL = fmax(0.0f, dot(N, L));
    if (NdotL <= 0) continue;

    if (traceShadow(bvhNodes, triIndices, vertices, triangles,
                    instances, numInstances, hitPos + N * 1.0f, L, dist))
      continue;

    float atten = 1.0f / (lights[li].attenuation.x + lights[li].attenuation.y * dist +
                          lights[li].attenuation.z * dist * dist);

    float3 diffuse = f3v(lights[li].diffuse) * baseColor * NdotL * atten;

    float3 V = normalize(-dir);
    float3 H = normalize(L + V);
    float spec = pow(fmax(0.0f, dot(N, H)), mat->shininess);
    float3 specular = f3v(lights[li].specular) * f3v(mat->specularColor) * spec * atten;

    float3 ambient = f3v(lights[li].ambient) * baseColor;

    color += diffuse + specular + ambient;
  }
  return color;
}

// ---- Main raytrace kernel (direct lighting + reflections) ----

__kernel void raytrace(
    __global const BVHNode *bvhNodes,
    __global const unsigned int *triIndices,
    __global const RTVertex *vertices,
    __global const RTTriangle *triangles,
    __global const FlatInstance *instances,
    __global const RTLight *lights,
    __global const RTMaterial *materials,
    __global unsigned char *outR,
    __global unsigned char *outG,
    __global unsigned char *outB,
    __global int *objectIds,
    float camPosX, float camPosY, float camPosZ,
    float qi00, float qi01, float qi02,
    float qi10, float qi11, float qi12,
    float qi20, float qi21, float qi22,
    int width, int height,
    int numInstances, int numLights,
    float bgR, float bgG, float bgB)
{
  int px = get_global_id(0);
  int py = get_global_id(1);
  if (px >= width || py >= height) return;

  float3 camPos = (float3)(camPosX, camPosY, camPosZ);
  float fpx = (float)px + 0.5f;
  float fpy = (float)py + 0.5f;
  float3 dir = normalize((float3)(
    qi00*fpx + qi10*fpy + qi20,
    qi01*fpx + qi11*fpy + qi21,
    qi02*fpx + qi12*fpy + qi22
  ));

  float3 origin = camPos;
  float3 color = (float3)(0, 0, 0);
  float3 throughput = (float3)(1, 1, 1);
  int primaryInstance = -1;

  // Iterative ray bouncing for reflections (max 4)
  for (int bounce = 0; bounce < 4; bounce++) {
    unsigned int hitTri; float hu, hv;
    int instIdx = traceScene(bvhNodes, triIndices, vertices, triangles,
                             instances, numInstances, origin, dir,
                             &hitTri, &hu, &hv);

    if (bounce == 0) primaryInstance = instIdx;

    if (instIdx < 0) {
      color += throughput * (float3)(bgR, bgG, bgB);
      break;
    }

    __global const FlatInstance *inst = &instances[instIdx];
    __global const RTMaterial *mat = &materials[inst->materialIndex];

    float3 hitPos, N, baseColor;
    interpolateHit(vertices, inst->vertexOffset, triangles, inst->triangleOffset,
                   inst, hitTri, hu, hv, &hitPos, &N, &baseColor);

    if (dot(N, dir) > 0) N = -N;

    // Emission
    color += throughput * f3v(mat->emission);

    // Direct lighting
    float3 direct = directLight(hitPos, N, dir, baseColor, mat,
                                lights, numLights, bvhNodes, triIndices,
                                vertices, triangles, instances, numInstances);

    float refl = mat->reflectivity;
    color += throughput * direct * (1.0f - refl);

    if (refl < 0.01f) break;

    // Reflect
    throughput *= refl;
    dir = dir - N * (2.0f * dot(dir, N));
    dir = normalize(dir);
    origin = hitPos + N * 1.0f;
  }

  int idx = px + py * width;
  objectIds[idx] = primaryInstance;
  color = clamp(color, 0.0f, 1.0f);
  outR[idx] = (unsigned char)(color.x * 255);
  outG[idx] = (unsigned char)(color.y * 255);
  outB[idx] = (unsigned char)(color.z * 255);
}

// ---- Path tracing kernel ----

__kernel void pathTraceKernel(
    __global const BVHNode *bvhNodes,
    __global const unsigned int *triIndices,
    __global const RTVertex *vertices,
    __global const RTTriangle *triangles,
    __global const FlatInstance *instances,
    __global const RTLight *lights,
    __global const RTMaterial *materials,
    __global float *accumR,
    __global float *accumG,
    __global float *accumB,
    __global unsigned char *outR,
    __global unsigned char *outG,
    __global unsigned char *outB,
    __global int *objectIds,
    float camPosX, float camPosY, float camPosZ,
    float qi00, float qi01, float qi02,
    float qi10, float qi11, float qi12,
    float qi20, float qi21, float qi22,
    int width, int height,
    int numInstances, int numLights,
    float bgR, float bgG, float bgB,
    int frameNumber)
{
  int px = get_global_id(0);
  int py = get_global_id(1);
  if (px >= width || py >= height) return;

  int idx = px + py * width;
  uint rng = (uint)(px * 1973 + py * 9277 + frameNumber * 26699) | 1u;

  float3 camPos = (float3)(camPosX, camPosY, camPosZ);

  // Jittered pixel position
  float fpx = (float)px + rngFloat(&rng);
  float fpy = (float)py + rngFloat(&rng);
  float3 dir = normalize((float3)(
    qi00*fpx + qi10*fpy + qi20,
    qi01*fpx + qi11*fpy + qi21,
    qi02*fpx + qi12*fpy + qi22
  ));

  float3 origin = camPos;
  float3 color = (float3)(0, 0, 0);
  float3 throughput = (float3)(1, 1, 1);
  int primaryInstance = -1;

  for (int bounce = 0; bounce < 5; bounce++) {
    unsigned int hitTri; float hu, hv;
    int instIdx = traceScene(bvhNodes, triIndices, vertices, triangles,
                             instances, numInstances, origin, dir,
                             &hitTri, &hu, &hv);

    if (bounce == 0) primaryInstance = instIdx;

    if (instIdx < 0) {
      // Sky
      float t = 0.5f * (dir.y + 1.0f);
      float3 sky = (float3)(0.05f, 0.05f, 0.08f) * (1-t) + (float3)(0.1f, 0.12f, 0.2f) * t;
      color += throughput * sky;
      break;
    }

    __global const FlatInstance *inst = &instances[instIdx];
    __global const RTMaterial *mat = &materials[inst->materialIndex];

    float3 hitPos, N, baseColor;
    interpolateHit(vertices, inst->vertexOffset, triangles, inst->triangleOffset,
                   inst, hitTri, hu, hv, &hitPos, &N, &baseColor);

    if (dot(N, dir) > 0) N = -N;

    // Emission
    color += throughput * f3v(mat->emission);

    // Direct lighting (next event estimation)
    for (int li = 0; li < numLights; li++) {
      if (!lights[li].on) continue;
      float3 lightPos = f3(lights[li].position);
      float3 L = lightPos - hitPos;
      float dist = length(L);
      if (dist < 1e-6f) continue;
      L /= dist;
      float NdotL = fmax(0.0f, dot(N, L));
      if (NdotL <= 0) continue;
      if (traceShadow(bvhNodes, triIndices, vertices, triangles,
                      instances, numInstances, hitPos + N * 1.0f, L, dist))
        continue;
      float atten = 1.0f / (lights[li].attenuation.x + lights[li].attenuation.y * dist +
                            lights[li].attenuation.z * dist * dist);
      color += throughput * f3v(lights[li].diffuse) * baseColor * NdotL * atten;
    }

    // Mirror reflection or diffuse bounce
    float refl = mat->reflectivity;
    if (refl > 0.01f && rngFloat(&rng) < refl) {
      // Specular reflection
      dir = dir - N * (2.0f * dot(dir, N));
      dir = normalize(dir);
    } else {
      // Diffuse bounce — cosine-weighted hemisphere
      dir = randomHemisphere(N, &rng);
      throughput *= baseColor;
    }
    origin = hitPos + N * 1.0f;

    // Russian roulette after bounce 2
    if (bounce > 1) {
      float p = fmax(throughput.x, fmax(throughput.y, throughput.z));
      if (rngFloat(&rng) > p) break;
      throughput /= p;
    }
  }

  // Running average accumulation
  float weight = 1.0f / (float)frameNumber;
  accumR[idx] += (color.x - accumR[idx]) * weight;
  accumG[idx] += (color.y - accumG[idx]) * weight;
  accumB[idx] += (color.z - accumB[idx]) * weight;

  // Object ID on first frame only
  if (frameNumber == 1) objectIds[idx] = primaryInstance;

  outR[idx] = (unsigned char)(clamp(accumR[idx], 0.0f, 1.0f) * 255);
  outG[idx] = (unsigned char)(clamp(accumG[idx], 0.0f, 1.0f) * 255);
  outB[idx] = (unsigned char)(clamp(accumB[idx], 0.0f, 1.0f) * 255);
}
