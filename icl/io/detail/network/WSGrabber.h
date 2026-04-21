// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/io/Grabber.h>
#include <string>
#include <vector>

namespace icl::io {
  /// WebSocket-based grabber (client side, with auto-reconnect)
  /** Connects to a remote `WSImageOutput` (or any WebSocket publisher
      that emits `ImageCompressor`-encoded binary frames) and exposes the
      received frames through the standard `Grabber::grabImage()` API.

      \section HIST History
      `WSGrabber` (Session 46) replaced the retired `SharedMemoryGrabber`
      (Session 47). See `WSImageOutput.h` for the rationale.

      \section URL  URL form (via GenericGrabber / `-i ws ...`)
      \code
        -i ws ws://host:port           # connect to a publisher
      \endcode
      Server-mode (`server:PORT` — bind & accept one push client) is
      planned for v2 and not yet implemented.

      \section RES  Resilience
      The client owns a private state machine (`Disconnected → Connecting
      → Connected → Disconnected …`). On a server vanish or transport
      error the grabber transparently retries with exponential backoff
      (`reconnect backoff initial ms`, doubled per failed attempt up to
      `reconnect backoff max ms`). The application loop never observes a
      "dead" state — `acquireImage()` blocks for up to `block timeout ms`
      waiting for a fresh frame, then optionally returns the last-known
      frame (`replay last on timeout`). Default: block 1000 ms, then
      replay.

      \section CFG Properties (Configurable)
        - `connection state`              info (live)
        - `reconnect attempts`            info (lifetime)
        - `last connected`                info (timestamp string)
        - `reconnect backoff initial ms`  range (default 250)
        - `reconnect backoff max ms`      range (default 5000)
        - `block timeout ms`              range (default 1000)
        - `replay last on timeout`        flag  (default true)
        - `queue size`                    range (default 2; drop-oldest)
        - `frames dropped`                info (queue overflow count)
        - `frames received`               info (lifetime)
        - `bytes received`                info (lifetime)
   */
  class ICLIO_API WSGrabber : public Grabber {
    /// pimpl
    struct Data;
    Data *m_data;

    public:

    /// url form: `ws://host:port`
    explicit WSGrabber(const std::string &url);

    /// Destructor — closes the socket, joins the WS thread
    ~WSGrabber();

    /// device discovery — currently empty (WS endpoints can't be
    /// auto-enumerated; user must specify the URL).
    static const std::vector<GrabberDeviceDescription> &
    getDeviceList(bool rescan);

    /// Pop the latest frame from the queue, blocking up to
    /// `block timeout ms` for a fresh frame; on timeout, optionally
    /// replays the last-known frame.
    virtual const core::ImgBase *acquireImage() override;
  };
} // namespace icl::io
