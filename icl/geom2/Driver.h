// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  class Node;

  /// Abstract attachable behaviour bound to a single Node.
  /** A Driver is an "internal" that computes its host node's transform or
      geometry each frame from some source (an animation clock, an attractor,
      or — in the physics2 module — a Bullet body). geom2 stays free of any
      simulation backend: a Driver only ever talks to its node() through the
      public Node API.

      Lifecycle (all on the UI / render thread):
        - onAttach()  : called once, right after the driver is added to a node;
                        node() is valid.
        - sync(dt,a)  : called every render frame by Scene2::sync(), in pre-order
                        (parent before children). `dt` is the wall-clock delta in
                        seconds; `alpha` is the interpolation fraction in [0,1]
                        used by sources that publish discrete states (physics);
                        purely time-driven drivers (animators) ignore it.
        - onDetach()  : called once, when the driver is removed / its node dies.

      Attach via Node::addDriver<T>(args...) (world-free drivers) or, for
      physics drivers, via the owning PhysicsWorld factory. The node owns the
      driver (shared_ptr); destroying the node detaches it. */
  class ICLGeom2_API Driver {
  public:
    virtual ~Driver();

    /// The node this driver is attached to (nullptr before attachment)
    Node *node() const { return m_node; }

    virtual void onAttach() {}
    virtual void onDetach() {}

    /// Per-frame update on the UI thread (see class docs for dt / alpha)
    virtual void sync(double dt, double alpha) { (void)dt; (void)alpha; }

  protected:
    Driver() = default;

  private:
    Node *m_node = nullptr;
    friend class Node;  // Node sets m_node on addDriver()
  };

} // namespace icl::geom2
