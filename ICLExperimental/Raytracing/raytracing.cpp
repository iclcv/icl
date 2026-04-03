// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Time.h>

#include <cmath>
#include <vector>
#include <algorithm>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;

struct Vec3 {
  float x, y, z;
  Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
  Vec3 operator+(const Vec3 &v) const { return {x+v.x, y+v.y, z+v.z}; }
  Vec3 operator-(const Vec3 &v) const { return {x-v.x, y-v.y, z-v.z}; }
  Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
  float dot(const Vec3 &v) const { return x*v.x + y*v.y + z*v.z; }
  float length() const { return std::sqrt(dot(*this)); }
  Vec3 normalized() const { float l = length(); return l > 0 ? *this * (1.0f/l) : Vec3(); }
};

struct Ray {
  Vec3 origin, dir;
};

struct Material {
  Vec3 color;
  float diffuse = 0.8f;
  float specular = 0.3f;
  float shininess = 32.0f;
  float reflectivity = 0.0f;
};

struct HitRecord {
  float t = 1e30f;
  Vec3 point, normal;
  Material material;
  bool hit = false;
};

struct Sphere {
  Vec3 center;
  float radius;
  Material material;

  bool intersect(const Ray &ray, HitRecord &rec) const {
    Vec3 oc = ray.origin - center;
    float b = oc.dot(ray.dir);
    float c = oc.dot(oc) - radius * radius;
    float disc = b * b - c;
    if (disc < 0) return false;
    float t = -b - std::sqrt(disc);
    if (t < 0.001f) {
      t = -b + std::sqrt(disc);
      if (t < 0.001f) return false;
    }
    if (t < rec.t) {
      rec.t = t;
      rec.point = ray.origin + ray.dir * t;
      rec.normal = (rec.point - center) * (1.0f / radius);
      rec.material = material;
      rec.hit = true;
      return true;
    }
    return false;
  }
};

// Checkerboard ground plane at y = -1
static bool intersectPlane(const Ray &ray, HitRecord &rec) {
  const float planeY = -1.0f;
  if (std::abs(ray.dir.y) < 1e-6f) return false;
  float t = (planeY - ray.origin.y) / ray.dir.y;
  if (t < 0.001f || t >= rec.t) return false;
  rec.t = t;
  rec.point = ray.origin + ray.dir * t;
  rec.normal = {0, 1, 0};
  int cx = (int)std::floor(rec.point.x);
  int cz = (int)std::floor(rec.point.z);
  bool white = ((cx + cz) & 1) == 0;
  rec.material.color = white ? Vec3(0.9f, 0.9f, 0.9f) : Vec3(0.2f, 0.2f, 0.2f);
  rec.material.diffuse = 0.7f;
  rec.material.specular = 0.1f;
  rec.material.reflectivity = 0.15f;
  rec.hit = true;
  return true;
}

struct Light {
  Vec3 position;
  Vec3 color;
  float intensity;
};

static std::vector<Sphere> spheres;
static std::vector<Light> lights;

static void setupScene() {
  spheres = {
    {{0, 0, -3}, 1.0f, {{1.0f, 0.2f, 0.2f}, 0.8f, 0.5f, 64.0f, 0.3f}},
    {{-2.2f, -0.3f, -4}, 0.7f, {{0.2f, 0.8f, 0.2f}, 0.8f, 0.4f, 32.0f, 0.1f}},
    {{2.0f, -0.5f, -3.5f}, 0.5f, {{0.2f, 0.3f, 1.0f}, 0.8f, 0.6f, 64.0f, 0.4f}},
    {{-0.5f, -0.7f, -1.8f}, 0.3f, {{1.0f, 0.8f, 0.1f}, 0.9f, 0.7f, 128.0f, 0.5f}},
  };
  lights = {
    {{-5, 5, -1}, {1, 1, 1}, 1.0f},
    {{3, 3, -2}, {0.6f, 0.6f, 1.0f}, 0.5f},
  };
}

static HitRecord traceRay(const Ray &ray) {
  HitRecord rec;
  for (auto &s : spheres) s.intersect(ray, rec);
  intersectPlane(ray, rec);
  return rec;
}

static Vec3 shade(const Ray &ray, int depth = 0) {
  HitRecord rec = traceRay(ray);
  if (!rec.hit) {
    // Sky gradient
    float t = 0.5f * (ray.dir.y + 1.0f);
    return Vec3(1, 1, 1) * (1 - t) + Vec3(0.5f, 0.7f, 1.0f) * t;
  }

  Vec3 result(0, 0, 0);
  Vec3 ambient = rec.material.color * 0.1f;
  result = result + ambient;

  for (auto &light : lights) {
    Vec3 toLight = light.position - rec.point;
    float dist = toLight.length();
    Vec3 L = toLight * (1.0f / dist);

    // Shadow test
    Ray shadowRay{rec.point + rec.normal * 0.001f, L};
    HitRecord shadowRec = traceRay(shadowRay);
    if (shadowRec.hit && shadowRec.t < dist) continue;

    // Diffuse
    float diff = std::max(0.0f, rec.normal.dot(L));
    Vec3 diffuse = rec.material.color * (diff * rec.material.diffuse * light.intensity);

    // Specular (Blinn-Phong)
    Vec3 V = (ray.dir * -1.0f).normalized();
    Vec3 H = (L + V).normalized();
    float spec = std::pow(std::max(0.0f, rec.normal.dot(H)), rec.material.shininess);
    Vec3 specular = light.color * (spec * rec.material.specular * light.intensity);

    result = result + diffuse + specular;
  }

  // Reflection
  if (depth < 3 && rec.material.reflectivity > 0.01f) {
    Vec3 refl = ray.dir - rec.normal * (2.0f * ray.dir.dot(rec.normal));
    Ray reflRay{rec.point + rec.normal * 0.001f, refl.normalized()};
    Vec3 reflColor = shade(reflRay, depth + 1);
    float r = rec.material.reflectivity;
    result = result * (1 - r) + reflColor * r;
  }

  // Clamp
  result.x = std::min(1.0f, result.x);
  result.y = std::min(1.0f, result.y);
  result.z = std::min(1.0f, result.z);
  return result;
}

GUI gui;

static void render(Img8u &img) {
  int w = img.getWidth(), h = img.getHeight();
  float aspect = (float)w / h;
  float fov = 1.0f; // ~53 degrees half-angle

  icl8u *R = img.getData(0);
  icl8u *G = img.getData(1);
  icl8u *B = img.getData(2);

  #pragma omp parallel for schedule(dynamic, 4)
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      float u = (2.0f * x / w - 1.0f) * aspect * fov;
      float v = (1.0f - 2.0f * y / h) * fov;
      Ray ray{{0, 0, 2}, Vec3(u, v, -1).normalized()};
      Vec3 c = shade(ray);
      int idx = x + y * w;
      R[idx] = (icl8u)(c.x * 255);
      G[idx] = (icl8u)(c.y * 255);
      B[idx] = (icl8u)(c.z * 255);
    }
  }
}

void init() {
  setupScene();
  gui << Canvas().handle("canvas").minSize(32, 24)
      << Show();
}

void run() {
  static Img8u img(Size(640, 480), formatRGB);

  Time t = Time::now();
  render(img);
  float ms = (float)(Time::now() - t).toMilliSeconds();

  DrawHandle draw = gui["canvas"];
  draw = img;
  draw->render();

  static char buf[64];
  snprintf(buf, sizeof(buf), "%.1f ms (%.0f fps)", ms, 1000.0f / ms);
  draw->text(buf, 10, 20, 12);
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
