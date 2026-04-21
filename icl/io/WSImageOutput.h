// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Configurable.h>
#include <icl/io/ImageOutput.h>
#include <string>

namespace icl::io {
  /// WebSocket-based image output (server side)
  /** Binds a `QWebSocketServer` on the given address/port and broadcasts
      every `send()`-ed image to every connected WebSocket client. The
      payload is an `ImageCompressor` envelope (default mode `none` —
      raw bytes); switch via the `compression` property when bandwidth
      matters. Counterpart on the receiving side: `WSGrabber`.

      \section HIST History
      `WSImageOutput` (Session 46) replaced the retired `SharedMemoryPublisher`
      (Session 47). For same-machine pipelines the loopback path costs ~100 µs
      of latency vs the previous shared-memory mechanism — invisible at
      typical 30 fps image-processing workloads. In exchange the WS pair
      brings auto-reconnect resilience, cross-host as a free bonus, and
      ~1274 fewer lines of QSharedMemory/QSystemSemaphore plumbing to maintain.

      \section URL  URL form (via GenericImageOutput / `-o ws ...`)
      \code
        -o ws PORT                    # bind 0.0.0.0:PORT, broadcast
        -o ws BIND:PORT               # bind a specific interface
      \endcode

      \section THR Threading
      Owns a private `QThread` running the `QWebSocketServer`. `send()`
      may be called from any thread; the payload is compressed on the
      caller thread and the broadcast is marshalled to the WS thread via
      `Qt::QueuedConnection`. A slow/dead client never blocks the
      others — it gets dropped on send-error and removed from the list.

      \section CFG Properties (Configurable)
        - `compression`         menu (none/raw/rlen/jpeg/png/1611)
        - `quality`             range, passed to ImageCompressor
        - `max message size MB` range, default 256
        - `bind address`        info, e.g. `0.0.0.0`
        - `port`                info (the actually bound port)
        - `clients`             info (live count)
        - `bytes sent`          info (lifetime)
        - `frames sent`         info (lifetime)
   */
  class ICLIO_API WSImageOutput : public ImageOutput,
                                  public utils::Configurable {
    // Note: WSImageOutput inherits Configurable directly to expose its
    // own properties (port, clients, bytes sent, …); the active codec's
    // tunables surface separately as a child Configurable under the
    // `compression.` prefix (the inner `m_data->compressor` —
    // an ImageCompressor — is itself a Configurable). See WSImageOutput.cpp.
    /// pimpl
    struct Data;
    Data *m_data;

    public:

    /// Construct + bind. Pass port=0 to let the OS pick a free port
    WSImageOutput(int port = 9090,
                  const std::string &bindAddress = "0.0.0.0");

    /// Destructor (closes the server, drops every client)
    ~WSImageOutput();

    /// ImageOutput contract — broadcast to all connected clients
    virtual void send(const core::Image &image) override;

    /// The actually bound port (useful when port=0 was requested)
    int actualPort() const;

    /// Live client count (also exposed via `clients` property)
    int connectedClients() const;

    /// True if construction failed to bind (port already in use, etc.)
    bool isNull() const { return m_data == nullptr; }

    explicit operator bool() const { return !isNull(); }

    private:
    /// Configurable hook for property → backend state
    void onPropertyChange(const utils::Configurable::Property &p);
  };
} // namespace icl::io
