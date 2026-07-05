// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PaperDriver.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/StateBuffer.h>
#include <icl/viz3d/Node.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/Primitive.h>
#include <icl/cv3d/ViewRay.h>
#include <icl/cv3d/Camera.h>
#include <icl/math/transform/HomogeneousMath.h>
#include <icl/math/transform/StraightLine2D.h>
#include <icl/math/la/FixedMatrix.h>
#include <icl/utils/Macros.h>
#include <icl/utils/prop/Constraints.h>

#include <BulletSoftBody/btSoftBody.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

namespace icl::physics2 {
  using utils::Point32f;
  using utils::Size;
  using math::linear_interpolate;
  using math::bilinear_interpolate;

  // A4 reference dimensions (mm) used to turn normalized paper coords into the
  // physical link rest-lengths / triangle areas (faithful to the legacy paper).
  static constexpr float PAPER_W = 210.f, PAPER_H = 297.f;

  // --- transplanted pure helpers (were icl::physics::GeometricTools) ---------
  namespace {
    bool line_segment_intersect(const Point32f &a, const Point32f &b, const Point32f &c,
                                const Point32f &d, Point32f *dst = 0,
                                float *dstr = 0, float *dsts = 0) {
      const float x1 = (a.y-c.y)*(d.x-c.x)-(a.x-c.x)*(d.y-c.y);
      const float x2 = (b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x);
      if (x1 == 0 && x2 == 0) return false;   // collinear
      if (x2 == 0) return false;              // parallel
      const float x3 = (a.y-c.y)*(b.x-a.x)-(a.x-c.x)*(b.y-a.y);
      const float x4 = (b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x);
      if (x4 == 0) return false;
      const float r = x1 / x2, s = x3 / x4;
      if (dstr) *dstr = 1.0f - r;
      if (dsts) *dsts = 1.0f - s;
      if (r >= 0 && r <= 1 && s >= 0 && s <= 1) {
        if (dst) *dst = a + (b - a) * r;
        return true;
      }
      return false;
    }

    inline float pit_sign(const Point32f &p1, const Point32f &p2, const Point32f &p3) {
      return (p1.x-p3.x)*(p2.y-p3.y) - (p2.x-p3.x)*(p1.y-p3.y);
    }
    bool point_in_triangle(const Point32f &p, const Point32f &v1,
                           const Point32f &v2, const Point32f &v3) {
      bool b1 = pit_sign(p, v1, v2) <= 0, b2 = pit_sign(p, v2, v3) <= 0,
           b3 = pit_sign(p, v3, v1) <= 0;
      return (b1 == b2) && (b2 == b3);
    }

    /// True iff segments ab and cd *properly* cross (intersection strictly
    /// interior to both) — endpoint-touching / collinear cases return false, so a
    /// bending link merely sharing a crease node is not counted as crossing.
    inline bool segments_cross(const Point32f &a, const Point32f &b,
                               const Point32f &c, const Point32f &d) {
      const float d1 = pit_sign(a, c, d), d2 = pit_sign(b, c, d);
      const float d3 = pit_sign(c, a, b), d4 = pit_sign(d, a, b);
      return (d1 * d2 < 0.f) && (d3 * d4 < 0.f);
    }

    /// Per-link fold metadata, owned via btSoftBody::Link::m_tag (heap).
    struct LinkState {
      bool isFirstOrder, isFold, hasMemorizedRestDist, isOriginal;
      LinkState(bool fo = true, bool f = false, bool m = false, bool o = false)
        : isFirstOrder(fo), isFold(f), hasMemorizedRestDist(m), isOriginal(o) {}
      LinkState *p() const { return new LinkState(*this); }
      static bool is_first_order(void *t) { return static_cast<LinkState *>(t)->isFirstOrder; }
      static bool is_fold(void *t) { return static_cast<LinkState *>(t)->isFold; }
      static bool has_memorized_rest_dist(void *t) { return static_cast<LinkState *>(t)->hasMemorizedRestDist; }
    };
    inline void free_link_state(void *p) { delete static_cast<LinkState *>(p); }

    template<class T> inline void free_bullet_inst(T &) {}
    template<> inline void free_bullet_inst<btSoftBody::Link>(btSoftBody::Link &l) {
      free_link_state(l.m_tag);
    }
    template<class T>
    void remove_indices(btAlignedObjectArray<T> &array, std::vector<int> &indices) {
      std::sort(indices.begin(), indices.end());
      btAlignedObjectArray<T> tmp;
      tmp.resize(array.size() - indices.size());
      const int *is = indices.data(), *isEnd = is + indices.size();
      for (int si = 0, di = 0; di < tmp.size(); ++si) {
        if (is < isEnd && si == *is) { free_bullet_inst<T>(array[si]); ++is; }
        else tmp[di++] = array[si];
      }
      array = tmp;
    }

    template<class T> inline void shift_back_3(T &a, T &b, T &c) { T t = a; a = b; b = c; c = t; }
  }

  struct PaperDriver::Data {
    PhysicsWorld &world;
    Units units;
    Size cells;
    Vec corners[4];
    bool enableSelfCollision;
    float initialStiffness;     // <=0 => derive bending stiffness from the creases
    float maxLinkDist;          // bending-constraint range (paper units)
    float linkStiffness = 1e-5f; // crease (fold-link) softness
    bool smoothNormals = true;
    btSoftBody *body = nullptr;
    bool added = false;

    std::vector<Point32f> texCoords;       // paper coord of each node
    std::vector<Point32f> projectedPoints; // working buffer for fold hit-tests
    int numPointsBeforeUpdate = 0;
    int structureVersion = 0;              // bumped when a fold changes topology
    int builtVersion = -1;                 // UI-thread: version last built into the mesh
    mutable int lastUsedFaceIdx = -1;
    // persistent kinematic grab (sim-thread state)
    bool grabActive = false;
    std::vector<int> grabNodes;
    std::vector<Vec> grabOffsets;       // each node's world offset from the grab centre
    std::vector<btScalar> grabOrigIm;   // original inverse masses (restored on release)
    std::map<std::pair<int, int>, float> rlMap;
    std::vector<PaperDriver::Crease> creases;   // geometry primitives (paper space)
    PaperStateBuffer buffer;

    Data(PhysicsWorld &w) : world(w) {}

    void clearMemorizedRestDistances() { rlMap.clear(); }
    void addMemorizedRestDistance(int a, int b, float rl) { rlMap[{a, b}] = rl; }
    float getMemorizedRestDistance(int a, int b) {
      auto it = rlMap.find({a, b});
      return it == rlMap.end() ? 0 : it->second;
    }
  };

  // --- sim-side primitives operating on Data (transplanted from PhysicsPaper3) -
  namespace {
    int node_index(const btSoftBody *s, const btSoftBody::Node *n) {
      return static_cast<int>(n - &s->m_nodes[0]);
    }

    void addTriangleI(PaperDriver::Data &d, int a, int b, int c) {
      btSoftBody *s = d.body;
      s->appendFace(a, b, c);
      btSoftBody::Face &f = s->m_faces[s->m_faces.size() - 1];
      Point32f t0 = d.texCoords[a].transform(PAPER_W, PAPER_H);
      Point32f t1 = d.texCoords[b].transform(PAPER_W, PAPER_H);
      Point32f t2 = d.texCoords[c].transform(PAPER_W, PAPER_H);
      float la = t0.distanceTo(t1), lb = t1.distanceTo(t2), lc = t2.distanceTo(t0);
      float st = 0.5f * (la + lb + lc);
      f.m_ra = std::sqrt(std::max(0.f, st * (st - la) * (st - lb) * (st - lc)));
    }

    void addLinkI(PaperDriver::Data &d, int a, int b, float stiffness, const LinkState &state,
                  bool checkExist = true) {
      if (a == b) return;
      if (a > b) std::swap(a, b);
      btSoftBody *s = d.body;
      int n = s->m_links.size();
      // checkExist=false skips Bullet's O(#links) checkLink scan — only safe when
      // the caller has already deduped (createBendingConstraints does, via a set).
      s->appendLink(a, b, 0, checkExist);
      if (s->m_links.size() > n) {
        btSoftBody::Link &l = s->m_links[s->m_links.size() - 1];
        l.m_bbending = !state.isFirstOrder;
        const Point32f &ta = d.texCoords[a];
        const Point32f &tb = d.texCoords[b];
        if (state.hasMemorizedRestDist) {
          float rd = d.getMemorizedRestDistance(a, b);
          if (rd != 0) l.m_rl = rd;
          else {
            btVector3 dd = s->m_nodes[a].m_x - s->m_nodes[b].m_x;
            l.m_rl = std::sqrt(utils::sqr(dd[0]) + utils::sqr(dd[1]) + utils::sqr(dd[2]));
          }
        } else {
          l.m_rl = d.units.toBullet(std::sqrt(utils::sqr((ta.x-tb.x)*PAPER_W) +
                                              utils::sqr((ta.y-tb.y)*PAPER_H)));
        }
        l.m_c1 = l.m_rl * l.m_rl;
        btSoftBody::Material *m = s->appendMaterial();
        m->m_kLST = stiffness;
        l.m_material = m;
        l.m_c0 = (l.m_n[0]->m_im + l.m_n[1]->m_im) / m->m_kLST;
        l.m_tag = state.p();
        // (the logical crease is recorded once per fold in splitInPaperCoords)
      } else if (state.isFold) {
        // link already existed: upgrade it to a fold if it isn't one yet
        const btSoftBody::Node *na = &s->m_nodes[a], *nb = &s->m_nodes[b];
        for (int i = 0; i < s->m_links.size(); ++i) {
          btSoftBody::Link &l = s->m_links[i];
          if (((l.m_n[0] == na && l.m_n[1] == nb) || (l.m_n[0] == nb && l.m_n[1] == na))) {
            l.m_material->m_kLST = stiffness;
            if (!LinkState::is_fold(l.m_tag)) {
              free_link_state(l.m_tag);
              l.m_tag = state.p();
            }
            break;
          }
        }
      }
      s->m_bUpdateRtCst = false;
    }

    struct cmp_point_32f {
      const Point32f &a;
      cmp_point_32f(const Point32f &a) : a(a) {}
      bool operator()(const Point32f &b) const {
        return (utils::sqr(a.x-b.x) + utils::sqr(a.y-b.y)) < 0.000001f;
      }
    };

    void addVertexOrReuseOldOne(PaperDriver::Data &d, Point32f &t, btVector3 &v, int &idx) {
      auto it = std::find_if(d.texCoords.begin() + d.numPointsBeforeUpdate,
                             d.texCoords.end(), cmp_point_32f(t));
      btSoftBody *s = d.body;
      if (it != d.texCoords.end()) {
        t = *it;
        idx = static_cast<int>(it - d.texCoords.begin());
        v = s->m_nodes[idx].m_x;
      } else {
        idx = static_cast<int>(d.texCoords.size());
        d.texCoords.push_back(t);
        s->appendNode(v, 0.01);   // Bullet re-points existing link/face node ptrs
      }
    }

    bool hitLink(PaperDriver::Data &d, btSoftBody::Link *l, const Point32f &a, const Point32f &b) {
      int ia = node_index(d.body, l->m_n[0]), ib = node_index(d.body, l->m_n[1]);
      return line_segment_intersect(a, b, d.projectedPoints[ia], d.projectedPoints[ib]);
    }
    bool hitTriangle(PaperDriver::Data &d, btSoftBody::Face *f, const Point32f &a, const Point32f &b) {
      int ia = node_index(d.body, f->m_n[0]), ib = node_index(d.body, f->m_n[1]),
          ic = node_index(d.body, f->m_n[2]);
      Point32f ta = d.projectedPoints[ia], tb = d.projectedPoints[ib], tc = d.projectedPoints[ic];
      return line_segment_intersect(a, b, ta, tb) ||
             line_segment_intersect(a, b, tb, tc) ||
             line_segment_intersect(a, b, tc, ta);
    }

    bool replaceTriangle(PaperDriver::Data &d, btSoftBody::Face *f,
                         const Point32f &lineA, const Point32f &lineB) {
      btSoftBody *s = d.body;
      int idx_a = node_index(s, f->m_n[0]), idx_b = node_index(s, f->m_n[1]), idx_c = node_index(s, f->m_n[2]);
      btVector3 a = s->m_nodes[idx_a].m_x, b = s->m_nodes[idx_b].m_x, c = s->m_nodes[idx_c].m_x;
      Point32f ta = d.projectedPoints[idx_a], tb = d.projectedPoints[idx_b], tc = d.projectedPoints[idx_c];

      float rA = 0, rB = 0, rC = 0;
      bool iA = line_segment_intersect(lineA, lineB, ta, tb, 0, 0, &rA);
      bool iB = line_segment_intersect(lineA, lineB, tb, tc, 0, 0, &rB);
      bool iC = line_segment_intersect(lineA, lineB, tc, ta, 0, 0, &rC);
      rA = 1.f - rA; rB = 1.f - rB; rC = 1.f - rC;

      if ((!!iA + !!iB + !!iC) != 2) {
        addLinkI(d, idx_a, idx_b, 1, LinkState());
        addLinkI(d, idx_b, idx_c, 1, LinkState());
        addLinkI(d, idx_c, idx_a, 1, LinkState());
        return false;
      }
      if (iA && iB) {
        // keep
      } else if (iB && iC) {
        shift_back_3(idx_a, idx_b, idx_c); shift_back_3(ta, tb, tc);
        shift_back_3(a, b, c); shift_back_3(rA, rB, rC); shift_back_3(iA, iB, iC);
      } else {
        for (int i = 0; i < 2; ++i) {
          shift_back_3(idx_a, idx_b, idx_c); shift_back_3(ta, tb, tc);
          shift_back_3(a, b, c); shift_back_3(rA, rB, rC); shift_back_3(iA, iB, iC);
        }
      }

      Point32f tta = linear_interpolate(d.texCoords[idx_a], d.texCoords[idx_b], rA);
      Point32f ttb = linear_interpolate(d.texCoords[idx_b], d.texCoords[idx_c], rB);
      const float MIN_VEC_DIST = 0.002f;
      bool too_close_tta_ta = d.texCoords[idx_a].distanceTo(tta) < MIN_VEC_DIST;
      bool too_close_tta_tb = d.texCoords[idx_b].distanceTo(tta) < MIN_VEC_DIST;
      bool too_close_tta_tc = d.texCoords[idx_c].distanceTo(tta) < MIN_VEC_DIST;
      bool too_close_ttb_ta = d.texCoords[idx_a].distanceTo(ttb) < MIN_VEC_DIST;
      bool too_close_ttb_tb = d.texCoords[idx_b].distanceTo(ttb) < MIN_VEC_DIST;
      bool too_close_ttb_tc = d.texCoords[idx_c].distanceTo(ttb) < MIN_VEC_DIST;

      bool split = true;
      utils::Point *newSoftLink = 0;
      const float ls = d.linkStiffness;

      if (too_close_tta_ta) {
        if (too_close_ttb_ta || too_close_ttb_tb || too_close_ttb_tc) {
          split = false;
          if (too_close_ttb_tb) newSoftLink = new utils::Point(idx_a, idx_b);
          else if (too_close_ttb_tc) newSoftLink = new utils::Point(idx_a, idx_c);
        } else {
          int idx_new = -1;
          btVector3 vNew = linear_interpolate(b, c, rB);
          addVertexOrReuseOldOne(d, ttb, vNew, idx_new);
          addLinkI(d, idx_a, idx_b, 1, LinkState());
          addLinkI(d, idx_b, idx_new, 1, LinkState());
          addLinkI(d, idx_new, idx_a, ls, LinkState(true, true));
          addLinkI(d, idx_new, idx_c, 1, LinkState());
          addLinkI(d, idx_c, idx_a, 1, LinkState());
          addTriangleI(d, idx_a, idx_b, idx_new);
          addTriangleI(d, idx_a, idx_new, idx_c);
        }
      } else if (too_close_ttb_tc) {
        if (too_close_tta_ta || too_close_tta_tb || too_close_tta_tc) {
          split = false;
          if (too_close_ttb_ta) newSoftLink = new utils::Point(idx_a, idx_c);
          else if (too_close_ttb_tb) newSoftLink = new utils::Point(idx_b, idx_c);
        } else {
          int idx_new = -1;
          btVector3 vNew = linear_interpolate(a, b, rA);
          addVertexOrReuseOldOne(d, tta, vNew, idx_new);
          addLinkI(d, idx_a, idx_new, 1, LinkState());
          addLinkI(d, idx_new, idx_c, ls, LinkState(true, true));
          addLinkI(d, idx_c, idx_a, 1, LinkState());
          addLinkI(d, idx_new, idx_b, 1, LinkState());
          addLinkI(d, idx_b, idx_c, 1, LinkState());
          addTriangleI(d, idx_a, idx_new, idx_c);
          addTriangleI(d, idx_new, idx_b, idx_c);
        }
      } else if (!too_close_tta_ta && !too_close_tta_tb && !too_close_tta_tc &&
                 !too_close_ttb_ta && !too_close_ttb_tb && !too_close_ttb_tc) {
        int idx_new_a = -1, idx_new_b = -1;
        btVector3 vNew_a = linear_interpolate(a, b, rA);
        btVector3 vNew_b = linear_interpolate(b, c, rB);
        addVertexOrReuseOldOne(d, tta, vNew_a, idx_new_a);
        addVertexOrReuseOldOne(d, ttb, vNew_b, idx_new_b);
        addLinkI(d, idx_new_a, idx_b, 1, LinkState());
        addLinkI(d, idx_b, idx_new_b, 1, LinkState());
        addLinkI(d, idx_new_b, idx_new_a, ls, LinkState(true, true));
        addLinkI(d, idx_a, idx_new_a, 1, LinkState());
        addLinkI(d, idx_new_b, idx_a, 1, LinkState());
        addLinkI(d, idx_new_b, idx_c, 1, LinkState());
        addLinkI(d, idx_c, idx_a, 1, LinkState());
        addTriangleI(d, idx_new_a, idx_b, idx_new_b);
        addTriangleI(d, idx_a, idx_new_a, idx_new_b);
        addTriangleI(d, idx_a, idx_new_b, idx_c);
      } else {
        split = false;
      }

      if (!split) {
        int k = newSoftLink ? newSoftLink->x : -1;
        int l = newSoftLink ? newSoftLink->y : -1;
        delete newSoftLink;
        if (k > l) std::swap(k, l);
        bool b_a = (std::min(idx_a, idx_b) == k && std::max(idx_a, idx_b) == l);
        bool b_b = (std::min(idx_b, idx_c) == k && std::max(idx_b, idx_c) == l);
        bool b_c = (std::min(idx_c, idx_a) == k && std::max(idx_c, idx_a) == l);
        addLinkI(d, idx_a, idx_b, b_a ? ls : 1, b_a ? LinkState(true, true) : LinkState());
        addLinkI(d, idx_b, idx_c, b_b ? ls : 1, b_b ? LinkState(true, true) : LinkState());
        addLinkI(d, idx_c, idx_a, b_c ? ls : 1, b_c ? LinkState(true, true) : LinkState());
        return false;
      }
      return true;
    }

    void updateNodeAreas(PaperDriver::Data &d) {
      btSoftBody *s = d.body;
      std::vector<int> counts(s->m_nodes.size(), 0);
      for (int i = 0; i < s->m_nodes.size(); ++i) s->m_nodes[i].m_area = 0;
      for (int i = 0; i < s->m_faces.size(); ++i) {
        btSoftBody::Face &f = s->m_faces[i];
        for (int j = 0; j < 3; ++j) {
          counts[node_index(s, f.m_n[j])]++;
          f.m_n[j]->m_area += std::fabs(f.m_ra);
        }
      }
      for (int i = 0; i < s->m_nodes.size(); ++i)
        s->m_nodes[i].m_area /= (counts[i] ? counts[i] : 1);
      s->m_fdbvt.clear();
      if (s->m_cfg.collisions & btSoftBody::fCollision::VF_SS) s->initializeFaceTree();
      s->m_bUpdateRtCst = false;
    }

    void createBendingConstraints(PaperDriver::Data &d, float maxDistance, float fixedStiffness) {
      btSoftBody *s = d.body;
      btAlignedObjectArray<btSoftBody::Link> &ls = s->m_links;
      std::vector<int> rmLinks;
      d.clearMemorizedRestDistances();
      for (int i = 0; i < ls.size(); ++i) {
        if (!LinkState::is_first_order(ls[i].m_tag)) {
          if (LinkState::has_memorized_rest_dist(ls[i].m_tag)) {
            d.addMemorizedRestDistance(node_index(s, ls[i].m_n[0]),
                                       node_index(s, ls[i].m_n[1]), ls[i].m_rl);
          }
          rmLinks.push_back(i);
        }
      }
      remove_indices(ls, rmLinks);

      // Dedup bending links against the existing (first-order) links in O(log)
      // via a set, instead of appendLink's O(#links) checkLink scan — which turned
      // the all-pairs build into O(n^4) and hung the demo at startup. The set also
      // dedups the j>i loop's own pairs.
      std::set<std::pair<int, int>> existing;
      for (int i = 0; i < ls.size(); ++i)
        existing.insert({std::min(node_index(s, ls[i].m_n[0]), node_index(s, ls[i].m_n[1])),
                         std::max(node_index(s, ls[i].m_n[0]), node_index(s, ls[i].m_n[1]))});

      // Extend each crease segment ~1.5cm past its (edge-clipped) endpoints for the
      // crossing test only. A crease ends exactly on the paper boundary, so a
      // bending link near the edge can cross it right at that endpoint, where the
      // strict sign test degenerates (≈0) and the link is wrongly kept stiff.
      // Pushing the endpoints outside the sheet turns those into clean interior
      // crossings. Display still uses the un-extended d.creases.
      std::vector<std::pair<Point32f, Point32f>> extCreases;
      extCreases.reserve(d.creases.size());
      for (const auto &cr : d.creases) {
        Point32f dir = cr.b - cr.a;
        const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 1e-6f) { extCreases.emplace_back(cr.a, cr.b); continue; }
        dir = dir * (0.05f / len);   // ~5% of paper span (>1cm on either axis)
        extCreases.emplace_back(cr.a - dir, cr.b + dir);
      }

      const float maxD2 = maxDistance * maxDistance;
      const std::vector<Point32f> &tex = d.texCoords;
      const int num = static_cast<int>(tex.size());
      for (int i = 0; i < num; ++i) {
        for (int j = i + 1; j < num; ++j) {
          float dd = utils::sqr(tex[i].x - tex[j].x) + utils::sqr(tex[i].y - tex[j].y);
          if (dd >= maxD2) continue;
          if (!existing.insert({i, j}).second) continue;   // already a link
          // Reduce links that cross a crease to the (weakest) crossed crease's
          // stiffness — an exact segment-cross test against the crease primitives
          // (no rasterization, so no missed long crossings).
          float stiffness = (fixedStiffness > 0) ? fixedStiffness : 1.f;
          bool memorized = false;
          if (fixedStiffness <= 0) {
            for (size_t k = 0; k < d.creases.size(); ++k)
              if (segments_cross(tex[i], tex[j], extCreases[k].first, extCreases[k].second)) {
                if (d.creases[k].stiffness < stiffness) stiffness = d.creases[k].stiffness;
                memorized = memorized || d.creases[k].memorized;
              }
          }
          addLinkI(d, i, j, stiffness, LinkState(false, false, memorized),
                   /*checkExist*/ false);
        }
      }
      updateNodeAreas(d);
      s->randomizeConstraints();
      if (d.enableSelfCollision) s->generateClusters(0);
    }

    /// Collect the triangle list (flat index triples) for the render side.
    void publishTopology(PaperDriver::Data &d) {
      btSoftBody *s = d.body;
      std::vector<int> faces(s->m_faces.size() * 3);
      for (int f = 0; f < s->m_faces.size(); ++f) {
        const btSoftBody::Face &face = s->m_faces[f];
        faces[3*f+0] = node_index(s, face.m_n[0]);
        faces[3*f+1] = node_index(s, face.m_n[1]);
        faces[3*f+2] = node_index(s, face.m_n[2]);
      }
      d.buffer.publishTopology(std::move(faces), d.structureVersion);
    }

    /// World position of a paper coord (barycentric); falls back to the nearest
    /// node rather than throwing if no triangle contains the coord.
    Vec interpolatePositionI(PaperDriver::Data &d, Point32f p) {
      if (p.x <= 0) p.x = 1e-7f; if (p.y <= 0) p.y = 1e-7f;
      if (p.x >= 1) p.x = 1.f - 1e-7f; if (p.y >= 1) p.y = 1.f - 1e-7f;
      const btSoftBody *s = d.body;
      int hit_idx = -1, ia = 0, ib = 0, ic = 0;
      auto test = [&](int fi) {
        const btSoftBody::Face &f = s->m_faces[fi];
        ia = node_index(s, f.m_n[0]); ib = node_index(s, f.m_n[1]); ic = node_index(s, f.m_n[2]);
        return point_in_triangle(p, d.texCoords[ia], d.texCoords[ib], d.texCoords[ic]);
      };
      if (d.lastUsedFaceIdx >= 0 && d.lastUsedFaceIdx < s->m_faces.size() && test(d.lastUsedFaceIdx))
        hit_idx = d.lastUsedFaceIdx;
      if (hit_idx == -1)
        for (int i = 0; i < s->m_faces.size(); ++i) if (test(i)) { hit_idx = i; break; }
      if (hit_idx == -1) {
        // fallback: nearest node by paper distance
        int best = 0; float bd = 1e30f;
        for (size_t i = 0; i < d.texCoords.size(); ++i) {
          float dd = p.distanceTo(d.texCoords[i]);
          if (dd < bd) { bd = dd; best = (int)i; }
        }
        return d.units.toIclVec(s->m_nodes[best].m_x);
      }
      d.lastUsedFaceIdx = hit_idx;
      const btSoftBody::Face &f = s->m_faces[hit_idx];
      const Vec a = d.units.toIclVec(f.m_n[0]->m_x);
      const Vec b = d.units.toIclVec(f.m_n[1]->m_x);
      const Vec c = d.units.toIclVec(f.m_n[2]->m_x);
      const Point32f &ta = d.texCoords[ia], &tb = d.texCoords[ib], &tc = d.texCoords[ic];
      math::FixedMatrix<float, 2, 2> M(tc.x - ta.x, tb.x - ta.x, tc.y - ta.y, tb.y - ta.y);
      math::FixedColVector<float,2> ff = M.inv() * math::FixedColVector<float,2>(p.x - ta.x, p.y - ta.y);
      Vec res = a + (c - a) * ff[0] + (b - a) * ff[1];
      res[3] = 1;
      return res;
    }

    Point32f hitI(PaperDriver::Data &d, const geom::ViewRay &ray) {
      const btSoftBody *s = d.body;
      Point32f best(-1, -1);
      float bestDist = 1e30f;
      for (int i = 0; i < s->m_faces.size(); ++i) {
        const btSoftBody::Face &f = s->m_faces[i];
        geom::Vec a = d.units.toIclVec(f.m_n[0]->m_x);
        geom::Vec b = d.units.toIclVec(f.m_n[1]->m_x);
        geom::Vec c = d.units.toIclVec(f.m_n[2]->m_x);
        geom::Vec pW; Point32f coords;
        if (ray.getIntersectionWithTriangle(a, b, c, &pW, &coords) == geom::ViewRay::foundIntersection) {
          const Point32f &ta = d.texCoords[node_index(s, f.m_n[0])];
          const Point32f &tb = d.texCoords[node_index(s, f.m_n[1])];
          const Point32f &tc = d.texCoords[node_index(s, f.m_n[2])];
          Point32f pPaper = ta + (tb - ta) * coords.x + (tc - ta) * coords.y;
          float dist = (pW[0]-ray.offset[0])*(pW[0]-ray.offset[0]) +
                       (pW[1]-ray.offset[1])*(pW[1]-ray.offset[1]) +
                       (pW[2]-ray.offset[2])*(pW[2]-ray.offset[2]);
          if (dist < bestDist) { bestDist = dist; best = pPaper; }
        }
      }
      return best;
    }

    void splitInPaperCoords(PaperDriver::Data &d, Point32f a, Point32f b, bool extend) {
      Point32f creaseA = a, creaseB = b;   // logical crease (paper space), pre-elongation
      Point32f e = (b - a) * 100;   // elongate to avoid grazing the endpoints
      b += e; a -= e;
      if (extend) {
        std::vector<Point32f> wheres;
        Point32f edges[5] = {Point32f(0,0), Point32f(1,0), Point32f(1,1), Point32f(0,1), Point32f(0,0)};
        Point32f where;
        for (int i = 0; i < 4; ++i) {
          if (line_segment_intersect(a, b, edges[i], edges[i+1], &where)) wheres.push_back(where);
          if (wheres.size() >= 2) break;
        }
        if (wheres.size() == 2) {
          creaseA = wheres[0]; creaseB = wheres[1];   // edge-clipped crease for display + crossing
          a = wheres[0]; b = wheres[1];
          Point32f e2 = (b - a); b += e2; a -= e2;
        }
      }
      // record the crease as a geometry primitive (the source of truth for the
      // bending reduction below + the 2D paper view)
      d.creases.push_back({creaseA, creaseB, d.linkStiffness, false});
      btSoftBody *s = d.body;
      std::vector<int> delLinks, delTriangles;
      d.projectedPoints = d.texCoords;   // hit-test in paper space
      d.numPointsBeforeUpdate = s->m_nodes.size();
      for (int i = 0; i < s->m_links.size(); ++i)
        if (hitLink(d, &s->m_links[i], a, b)) delLinks.push_back(i);
      remove_indices(s->m_links, delLinks);
      int n_faces = s->m_faces.size();
      for (int i = 0; i < n_faces; ++i)
        if (hitTriangle(d, &s->m_faces[i], a, b))
          if (replaceTriangle(d, &s->m_faces[i], a, b)) delTriangles.push_back(i);
      remove_indices(s->m_faces, delTriangles);
      d.lastUsedFaceIdx = -1;
      createBendingConstraints(d, d.maxLinkDist, d.initialStiffness);
    }
  }

  // ---------------------------------------------------------------------------
  PaperDriver::PaperDriver(PhysicsWorld &world, const Size &cells, const Vec *corners,
                           bool enableSelfCollision, float initialStiffness, float maxLinkDist)
    : m_data(std::make_unique<Data>(world)) {
    m_data->cells = cells;
    m_data->enableSelfCollision = enableSelfCollision;
    m_data->initialStiffness = initialStiffness;
    m_data->maxLinkDist = maxLinkDist;

    const float rx = PAPER_W / 2, ry = PAPER_H / 2, z = 40;
    const Vec def[4] = { Vec(-rx,-ry,z,1), Vec(rx,-ry,z,1), Vec(-rx,ry,z,1), Vec(rx,ry,z,1) };
    for (int i = 0; i < 4; ++i) m_data->corners[i] = corners ? corners[i] : def[i];

    using namespace utils;
    addProperty("fold softness", prop::Range{.min=0.00001f, .max=1.f, .step=0.001f}, m_data->linkStiffness,
                "crease (fold-link) stiffness — lower = sharper, floppier creases");
    addProperty("bend range", prop::Range{.min=0.05f, .max=2.5f, .step=0.05f}, maxLinkDist,
                "bending-constraint range in paper units (higher = stiffer/papery; "
                ">~1.4 links every node = max stiffness, more cost)");
    addProperty("self collision", prop::Flag{}, enableSelfCollision,
                "cluster self-collision (stops the paper passing through itself)");
    addProperty("smooth normals", prop::Flag{}, true,
                "interpolate vertex normals for smooth shading (off = faceted)");

    registerCallback([this](const Configurable::Property &p) {
      if (p.name == "smooth normals") {
        m_data->smoothNormals = (bool)prop(p.name).value;   // render-side only
        if (auto *mesh = dynamic_cast<viz3d::MeshNode *>(node()))
          mesh->setSmoothShading(m_data->smoothNormals);
      } else {
        // fold softness / bend range / self collision: re-derive the bending graph
        // on the sim thread (changes links only, not the mesh topology).
        m_data->linkStiffness = (float)prop("fold softness").value;
        m_data->maxLinkDist = (float)prop("bend range").value;
        bool sc = (bool)prop("self collision").value;
        PaperDriver::Data *d = m_data.get();
        m_data->world.enqueue([d, sc]() {
          if (!d->body) return;
          d->enableSelfCollision = sc;
          d->body->m_cfg.collisions = sc
              ? (btSoftBody::fCollision::CL_SS | btSoftBody::fCollision::SDF_RS | btSoftBody::fCollision::CL_SELF)
              : (int)btSoftBody::fCollision::SDF_RS;
          createBendingConstraints(*d, d->maxLinkDist, d->initialStiffness);
        });
      }
    });
  }

  PaperDriver::~PaperDriver() {
    if (m_data->body) {
      // free per-link LinkState tags before the soft body goes away
      for (int i = 0; i < m_data->body->m_links.size(); ++i)
        free_link_state(m_data->body->m_links[i].m_tag);
      delete m_data->body;
    }
  }

  void PaperDriver::onAttach() { buildBody(); }

  void PaperDriver::buildBody() {
    auto *mesh = dynamic_cast<viz3d::MeshNode *>(node());
    if (!mesh) { ERROR_LOG("PaperDriver must be attached to a viz3d::MeshNode"); return; }
    if (m_data->world.isDeformable()) {
      ERROR_LOG("PaperDriver needs a SoftRigid world (cluster self-collision + per-link "
                "constants); construct the scene with SoftBodyMode::SoftRigid");
      return;
    }
    Data &d = *m_data;
    d.units = d.world.getUnits();
    const Units &u = d.units;
    const int nx = d.cells.width, ny = d.cells.height;
    const float dx = 1.f / (nx - 1), dy = 1.f / (ny - 1);

    std::vector<btVector3> nodes;
    d.texCoords.clear();
    // corner grid
    for (int y = 0; y < ny; ++y)
      for (int x = 0; x < nx; ++x) {
        Point32f p(x * dx, y * dy);
        d.texCoords.push_back(p);
        nodes.push_back(u.toBulletVec(bilinear_interpolate(d.corners, p.x, p.y)));
      }
    // per-cell centre vertices (the dual mesh)
    for (int y = 1; y < ny; ++y)
      for (int x = 1; x < nx; ++x) {
        Point32f q((x - .5f) * dx, (y - .5f) * dy);
        d.texCoords.push_back(q);
        nodes.push_back(u.toBulletVec(bilinear_interpolate(d.corners, q.x, q.y)));
      }

    auto *info = d.world.getSoftBodyWorldInfo();
    btSoftBody *s = new btSoftBody(info, (int)nodes.size(), nodes.data(), 0);
    s->setTotalMass(nodes.size() * 0.01, false);
    s->m_cfg.kLF = 0; s->m_cfg.kDG = 0;
    s->m_cfg.kMT = 0.7; s->m_cfg.kDP = 0.1; s->m_cfg.kDF = 0.4;
    s->getCollisionShape()->setMargin(u.toBullet(2));
    s->appendMaterial();
    s->m_materials[0]->m_kLST = 1.0; s->m_materials[0]->m_kAST = 1.0; s->m_materials[0]->m_kVST = 1.0;
    if (d.enableSelfCollision)
      s->m_cfg.collisions = btSoftBody::fCollision::CL_SS | btSoftBody::fCollision::SDF_RS |
                            btSoftBody::fCollision::CL_SELF;
    d.body = s;

    // triangles + first-order links (per cell: centre vertex fans out to 4 corners)
    const int o = nx * ny;
    auto IDX = [nx](int x, int y) { return x + nx * y; };
    auto IDXO = [nx, o](int x, int y) { return o + x + (nx - 1) * y; };
    for (int y = 1; y < ny; ++y)
      for (int x = 1; x < nx; ++x) {
        int a = IDX(x-1, y-1), b = IDX(x, y-1), c = IDX(x-1, y), dd = IDX(x, y), e = IDXO(x-1, y-1);
        addTriangleI(d, e, a, b); addTriangleI(d, e, b, dd);
        addTriangleI(d, e, dd, c); addTriangleI(d, e, c, a);
        LinkState orig; orig.isOriginal = true;
        addLinkI(d, a, b, 1, orig); addLinkI(d, a, c, 1, orig);
        addLinkI(d, a, e, 1, orig); addLinkI(d, b, e, 1, orig);
        addLinkI(d, c, e, 1, orig); addLinkI(d, dd, e, 1, orig);
        if (x == nx-1 || y == ny-1) { addLinkI(d, b, dd, 1, orig); addLinkI(d, c, dd, 1, orig); }
      }
    d.lastUsedFaceIdx = -1;
    createBendingConstraints(d, d.maxLinkDist, d.initialStiffness);

    d.world.addSoftBody(s);
    d.added = true;
    d.world.addCapture(this, [this]() {
      Data &dd = *m_data;
      btSoftBody *bb = dd.body;
      // anti-explosion safety net (legacy impulse contacts can pump energy at rest)
      const btScalar vmax = dd.units.toBullet(15000.f), vmax2 = vmax * vmax;
      for (int i = 0; i < bb->m_nodes.size(); i++) {
        btScalar v2 = bb->m_nodes[i].m_v.length2();
        if (v2 > vmax2) bb->m_nodes[i].m_v *= vmax / btSqrt(v2);
      }
      std::vector<Vec> pos(bb->m_nodes.size());
      for (int i = 0; i < bb->m_nodes.size(); i++) pos[i] = dd.units.toIclVec(bb->m_nodes[i].m_x);
      dd.buffer.publishPositions(std::move(pos), dd.structureVersion);
    });

    // initial mesh topology (vertices from nodes, triangles from faces)
    const viz3d::GeomColor col(0.92f, 0.90f, 0.86f, 1.0f);
    mesh->clearGeometry();
    for (int i = 0; i < s->m_nodes.size(); ++i) mesh->addVertex(u.toIclVec(s->m_nodes[i].m_x), col);
    for (int f = 0; f < s->m_faces.size(); ++f) {
      const btSoftBody::Face &face = s->m_faces[f];
      int a = node_index(s, face.m_n[0]), b = node_index(s, face.m_n[1]), c = node_index(s, face.m_n[2]);
      mesh->addTriangle(a, b, c, a, b, c);
    }
    mesh->createAutoNormals(true);
    mesh->setSmoothShading(m_data->smoothNormals);
    d.structureVersion = 0;
    d.builtVersion = 0;
    publishTopology(d);
  }

  void PaperDriver::onDetach() {
    if (m_data->added) {
      m_data->world.removeCapture(this);
      m_data->world.removeSoftBody(m_data->body);
      m_data->added = false;
    }
    if (m_data->body) {
      for (int i = 0; i < m_data->body->m_links.size(); ++i)
        free_link_state(m_data->body->m_links[i].m_tag);
      delete m_data->body;
      m_data->body = nullptr;
    }
  }

  void PaperDriver::sync(double /*dt*/, double /*alpha*/) {
    auto *mesh = dynamic_cast<viz3d::MeshNode *>(node());
    if (!mesh) return;
    std::vector<Vec> pos;
    int ver = 0;
    if (!m_data->buffer.samplePositions(pos, ver)) return;
    if (ver != m_data->builtVersion) {
      // topology changed (a fold grew nodes/faces) — rebuild the mesh
      std::vector<int> faces; int tver = -1;
      m_data->buffer.sampleTopology(faces, tver);
      if (tver != ver) return;   // topology not yet caught up; wait one frame
      const viz3d::GeomColor col(0.92f, 0.90f, 0.86f, 1.0f);
      mesh->clearGeometry();
      for (const auto &p : pos) mesh->addVertex(p, col);
      for (size_t f = 0; f + 2 < faces.size(); f += 3)
        mesh->addTriangle(faces[f], faces[f+1], faces[f+2], faces[f], faces[f+1], faces[f+2]);
      mesh->createAutoNormals(true);
      mesh->setSmoothShading(m_data->smoothNormals);
      m_data->builtVersion = ver;
    } else {
      auto &v = mesh->getVertices();
      if (v.size() == pos.size()) {
        std::copy(pos.begin(), pos.end(), v.begin());
        mesh->createAutoNormals(true);
      }
    }
  }

  std::vector<std::pair<Vec, Vec>> PaperDriver::getCreaseSegments() const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    std::vector<std::pair<Vec, Vec>> out;
    btSoftBody *s = m_data->body;
    if (!s) return out;
    const Units &u = m_data->units;
    for (int i = 0; i < s->m_links.size(); ++i) {
      if (!LinkState::is_fold(s->m_links[i].m_tag)) continue;
      out.emplace_back(u.toIclVec(s->m_links[i].m_n[0]->m_x),
                       u.toIclVec(s->m_links[i].m_n[1]->m_x));
    }
    return out;
  }

  PaperDriver::DebugGeometry PaperDriver::getDebugGeometry() const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    DebugGeometry g;
    btSoftBody *s = m_data->body;
    if (!s) return g;
    const Units &u = m_data->units;
    // Bending links that cross a crease are reduced (by the fold-map) to at most
    // the crease stiffness; those are effectively a hinge, so hide them from the
    // 2nd-order overlay. Threshold = the crease (fold-link) stiffness.
    const float creaseThresh = m_data->linkStiffness;
    for (int i = 0; i < s->m_links.size(); ++i) {
      const btSoftBody::Link &l = s->m_links[i];
      auto seg = std::make_pair(u.toIclVec(l.m_n[0]->m_x), u.toIclVec(l.m_n[1]->m_x));
      if (LinkState::is_fold(l.m_tag))             g.creases.push_back(seg);
      else if (LinkState::is_first_order(l.m_tag)) g.firstOrder.push_back(seg);
      else if (!l.m_material || l.m_material->m_kLST > creaseThresh)
        g.secondOrder.push_back(seg);            // skip crease-reduced bending links
    }
    for (int i = 0; i < s->m_faces.size(); ++i) {
      const btSoftBody::Face &f = s->m_faces[i];
      Vec a = u.toIclVec(f.m_n[0]->m_x), b = u.toIclVec(f.m_n[1]->m_x), c = u.toIclVec(f.m_n[2]->m_x);
      g.faces.emplace_back(a, b);
      g.faces.emplace_back(b, c);
      g.faces.emplace_back(c, a);
    }
    return g;
  }

  std::shared_ptr<PaperDriver::LinkCoords>
  PaperDriver::projectScreenLine(const geom::ViewRay &rayA, const geom::ViewRay &rayB) const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    btSoftBody *s = m_data->body;
    if (!s) return {};
    // The crease is where the *cutting plane* (eye + both endpoint rays) meets the
    // paper. Plane through the eye C with normal N = dirA x dirB.
    const Vec &C = rayA.offset;
    const Vec &da = rayA.direction, &db = rayB.direction;
    const Vec N(da[1]*db[2]-da[2]*db[1], da[2]*db[0]-da[0]*db[2], da[0]*db[1]-da[1]*db[0], 0);
    const float nlen = std::sqrt(N[0]*N[0] + N[1]*N[1] + N[2]*N[2]);
    if (nlen < 1e-9f) return {};   // rays (nearly) parallel: no plane
    auto sdist = [&](const Vec &V) {
      return (N[0]*(V[0]-C[0]) + N[1]*(V[1]-C[1]) + N[2]*(V[2]-C[2])) / nlen;
    };
    // collect crease points (paper coords) where plane crosses face edges
    std::vector<Point32f> pts;
    for (int i = 0; i < s->m_faces.size(); ++i) {
      const btSoftBody::Face &f = s->m_faces[i];
      const int idx[3] = {node_index(s, f.m_n[0]), node_index(s, f.m_n[1]), node_index(s, f.m_n[2])};
      const float sd[3] = {sdist(m_data->units.toIclVec(f.m_n[0]->m_x)),
                           sdist(m_data->units.toIclVec(f.m_n[1]->m_x)),
                           sdist(m_data->units.toIclVec(f.m_n[2]->m_x))};
      for (int e = 0; e < 3; ++e) {
        const int p = e, q = (e + 1) % 3;
        if ((sd[p] < 0.f) == (sd[q] < 0.f)) continue;   // same side: no crossing
        const float t = sd[p] / (sd[p] - sd[q]);
        const Point32f &tp = m_data->texCoords[idx[p]], &tq = m_data->texCoords[idx[q]];
        pts.push_back(tp + (tq - tp) * t);
      }
    }
    if (pts.size() < 2) return {};
    // the two farthest-apart crossings define the crease line (paper coords)
    int bi = 0, bj = 1; float bd = -1.f;
    for (size_t i = 0; i < pts.size(); ++i)
      for (size_t j = i + 1; j < pts.size(); ++j) {
        const float d = pts[i].distanceTo(pts[j]);
        if (d > bd) { bd = d; bi = (int)i; bj = (int)j; }
      }
    if (bd < 1e-5f) return {};
    return std::make_shared<LinkCoords>(pts[bi], pts[bj]);
  }

  btSoftBody *PaperDriver::softBody() const { return m_data->body; }
  int PaperDriver::getNumNodes() const { return m_data->body ? m_data->body->m_nodes.size() : 0; }
  std::vector<PaperDriver::Crease> PaperDriver::getCreases() const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    return m_data->creases;
  }

  void PaperDriver::foldAlongLine(const Point32f &a, const Point32f &b, bool autoExtendToEdges) {
    Data *d = m_data.get();
    d->world.enqueue([d, a, b, autoExtendToEdges]() {
      if (!d->body) return;
      splitInPaperCoords(*d, a, b, autoExtendToEdges);
      d->structureVersion++;        // topology grew
      publishTopology(*d);
    });
  }

  void PaperDriver::dragPoint(const Point32f &coords, const Vec &target, float strength, float radius) {
    Data *d = m_data.get();
    d->world.enqueue([d, coords, target, strength, radius]() {
      if (!d->body) return;
      btSoftBody *s = d->body;
      Vec pW = interpolatePositionI(*d, coords);
      btVector3 offset = d->units.toBulletVec(Vec(target[0]-pW[0], target[1]-pW[1], target[2]-pW[2], 0));
      std::vector<int> close; std::vector<float> dist;
      for (int i = 0; i < s->m_nodes.size(); ++i) {
        float dd = coords.distanceTo(d->texCoords[i]);
        if (dd < 3 * radius) { close.push_back(i); dist.push_back(dd); }
      }
      if (close.size() == 1) {
        s->m_nodes[close[0]].m_v = offset * strength;
      } else if (close.size()) {
        float N = 1.0f / (std::sqrt(2 * M_PI) * radius);
        for (size_t i = 0; i < close.size(); ++i) {
          float alpha = N * std::exp(-dist[i] * dist[i] / (radius * radius)) * strength;
          s->m_nodes[close[i]].m_v = offset * alpha;
        }
      }
    });
  }

  void PaperDriver::beginGrab(const Point32f &paperCoords, float radius) {
    Data *d = m_data.get();
    d->world.enqueue([d, paperCoords, radius]() {
      if (!d->body) return;
      btSoftBody *s = d->body;
      // nearest node to the grab point — always grabbed, even if radius is tiny
      int center = 0; float bd = 1e30f;
      for (size_t i = 0; i < d->texCoords.size(); ++i) {
        float dd = paperCoords.distanceTo(d->texCoords[i]);
        if (dd < bd) { bd = dd; center = (int)i; }
      }
      Vec centerW = d->units.toIclVec(s->m_nodes[center].m_x);
      d->grabNodes.clear(); d->grabOffsets.clear(); d->grabOrigIm.clear();
      for (size_t i = 0; i < d->texCoords.size(); ++i) {
        if ((int)i != center && paperCoords.distanceTo(d->texCoords[i]) > radius) continue;
        Vec w = d->units.toIclVec(s->m_nodes[i].m_x);
        d->grabNodes.push_back((int)i);
        d->grabOffsets.push_back(Vec(w[0]-centerW[0], w[1]-centerW[1], w[2]-centerW[2], 0));
        d->grabOrigIm.push_back(s->m_nodes[i].m_im);
        s->m_nodes[i].m_im = 0;                 // pin (kinematic) so it holds
        s->m_nodes[i].m_v = btVector3(0, 0, 0);
      }
      d->grabActive = true;
    });
  }

  void PaperDriver::updateGrab(const Vec &worldTarget) {
    Data *d = m_data.get();
    d->world.enqueue([d, worldTarget]() {
      if (!d->grabActive || !d->body) return;
      btSoftBody *s = d->body;
      for (size_t k = 0; k < d->grabNodes.size(); ++k) {
        const Vec &o = d->grabOffsets[k];
        btVector3 p = d->units.toBulletVec(Vec(worldTarget[0]+o[0], worldTarget[1]+o[1],
                                               worldTarget[2]+o[2], 1));
        btSoftBody::Node &n = s->m_nodes[d->grabNodes[k]];
        n.m_x = p; n.m_q = p; n.m_v = btVector3(0, 0, 0);
      }
    });
  }

  void PaperDriver::endGrab() {
    Data *d = m_data.get();
    d->world.enqueue([d]() {
      if (!d->grabActive || !d->body) return;
      btSoftBody *s = d->body;
      for (size_t k = 0; k < d->grabNodes.size(); ++k)
        s->m_nodes[d->grabNodes[k]].m_im = d->grabOrigIm[k];   // unpin
      d->grabActive = false;
      d->grabNodes.clear(); d->grabOffsets.clear(); d->grabOrigIm.clear();
    });
  }

  void PaperDriver::wholeSheetMove(const Vec &deltaIcl) {
    Data *d = m_data.get();
    d->world.enqueue([d, deltaIcl]() {
      if (!d->body) return;
      btSoftBody *s = d->body;
      btVector3 off = d->units.toBulletVec(Vec(deltaIcl[0], deltaIcl[1], deltaIcl[2], 0));
      for (int i = 0; i < s->m_nodes.size(); ++i) {
        s->m_nodes[i].m_x += off;
        s->m_nodes[i].m_q = s->m_nodes[i].m_x;
        s->m_nodes[i].m_v = btVector3(0, 0, 0);
      }
      s->updateNormals(); s->updateBounds();
    });
  }

  void PaperDriver::reset() {
    Data *d = m_data.get();
    d->world.enqueue([d]() {
      if (!d->body) return;
      btSoftBody *s = d->body;
      for (int i = 0; i < s->m_nodes.size(); ++i) {
        btVector3 bp = d->units.toBulletVec(bilinear_interpolate(d->corners, d->texCoords[i].x, d->texCoords[i].y));
        s->m_nodes[i].m_x = bp; s->m_nodes[i].m_q = bp;
        s->m_nodes[i].m_v = btVector3(0, 0, 0); s->m_nodes[i].m_f = btVector3(0, 0, 0);
      }
      s->updateNormals(); s->updateBounds();
    });
  }

  PaperDriver::Point32f PaperDriver::hit(const geom::ViewRay &ray) const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    if (!m_data->body) return Point32f(-1, -1);
    return hitI(*m_data, ray);
  }

  Vec PaperDriver::interpolatePosition(const Point32f &paperCoords) const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    if (!m_data->body) return Vec(0, 0, 0, 1);
    return interpolatePositionI(*m_data, paperCoords);
  }

  std::shared_ptr<PaperDriver::LinkCoords>
  PaperDriver::getLinkCoords(const Point32f &pix, const geom::Camera &cam) const {
    std::scoped_lock<PhysicsWorld> lock(m_data->world);
    if (!m_data->body) return {};
    geom::ViewRay v = cam.getViewRay(pix);
    Point32f p = hitI(*m_data, v);
    const btSoftBody *s = m_data->body;
    int bestIA = -1, bestIB = -1; float bestD = -1;
    for (int i = 0; i < s->m_links.size(); ++i) {
      if (!LinkState::is_fold(s->m_links[i].m_tag)) continue;
      int ia = node_index(s, s->m_links[i].m_n[0]), ib = node_index(s, s->m_links[i].m_n[1]);
      Point32f la = m_data->texCoords[ia], lb = m_data->texCoords[ib];
      math::StraightLine2D line(la, lb - la);
      float dd = line.distance(p);
      if (dd < 0.005f && (bestIA < 0 || dd < bestD)) { bestIA = ia; bestIB = ib; bestD = dd; }
    }
    if (bestIA != -1)
      return std::make_shared<LinkCoords>(m_data->texCoords[bestIA], m_data->texCoords[bestIB]);
    return {};
  }

  void PaperDriver::adaptFoldStiffness(const LinkCoords &coords, float stiffness, bool memorize) {
    Data *d = m_data.get();
    Point32f a = coords.first, b = coords.second;
    d->world.enqueue([d, a, b, stiffness, memorize]() {
      if (!d->body) return;
      btSoftBody *s = d->body;
      math::StraightLine2D ab(a, b - a);
      // update the crease primitive(s) along the picked line (the source of truth)
      for (auto &cr : d->creases)
        if (ab.distance(cr.a) < 0.05f && ab.distance(cr.b) < 0.05f) {
          cr.stiffness = stiffness; cr.memorized = memorize;
        }
      // mirror the stiffness onto the realized fold links, and flag rest-length memory
      for (int i = 0; i < s->m_links.size(); ++i) {
        if (!LinkState::is_fold(s->m_links[i].m_tag)) continue;
        int ia = node_index(s, s->m_links[i].m_n[0]), ib = node_index(s, s->m_links[i].m_n[1]);
        Point32f la = d->texCoords[ia], lb = d->texCoords[ib];
        if (ab.distance(la) < 0.05f && ab.distance(lb) < 0.05f) {
          s->m_links[i].m_material->m_kLST = stiffness;
          static_cast<LinkState *>(s->m_links[i].m_tag)->hasMemorizedRestDist = memorize;
        }
      }
      createBendingConstraints(*d, d->maxLinkDist, d->initialStiffness);
    });
  }

} // namespace icl::physics2
