// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/WSImageOutput.h>
#include <icl/io/ImageCompressor.h>
#include <icl/io/CompressionRegistry.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Macros.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QByteArray>
#include <QtCore/QMetaObject>
#include <QtNetwork/QHostAddress>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include <atomic>
#include <mutex>

namespace icl::io {
  using namespace icl::utils;
  using namespace icl::core;

  // Qt's QWebSocketServer / QWebSocket require an event loop, which in turn
  // requires a QCoreApplication instance somewhere in the process. Apps
  // built on ICLApp already construct one (QApplication). For headless
  // tools (`icl-pipe -no-gui`, library users embedding ICL without Qt main
  // loops), we lazily install a process-wide QCoreApplication on first
  // WS-class construction. We never call exec() — child QThreads run
  // their own local event loops.
  static void ensureQCoreApplication() {
    if (QCoreApplication::instance() != nullptr) return;
    static int    s_argc = 1;
    static char   s_arg0[] = "icl-ws";
    static char  *s_argv[] = {s_arg0, nullptr};
    static QCoreApplication s_app(s_argc, s_argv);
    (void)s_app;
  }

  // ----------------------------------------------------------------- Impl --
  // QObject helper that owns the QWebSocketServer + client list. Lives on
  // a dedicated QThread so I/O never blocks the caller of send(). No
  // Q_OBJECT macro because we don't declare custom signals/slots — Qt6's
  // function-pointer + lambda connect() works just fine on a plain
  // QObject. Keeps the file moc-free.
  // -----------------------------------------------------------------------
  class WSImageOutputServer : public QObject {
  public:
    QWebSocketServer *server = nullptr;
    QList<QWebSocket*> clients;
    qint64 maxMessageSizeBytes = 256LL * 1024 * 1024;

    // counters (atomics so getters can read without locking)
    std::atomic<qint64> bytesSent{0};
    std::atomic<qint64> framesSent{0};

    // Setup runs on the WS thread once `start()` is invoked there.
    void start(QHostAddress addr, quint16 port) {
      server = new QWebSocketServer(QStringLiteral("icl-ws-output"),
                                    QWebSocketServer::NonSecureMode, this);
      // headroom for raw 4K RGB and similar — caller can override via prop
      server->setMaxPendingConnections(64);
      QObject::connect(server, &QWebSocketServer::newConnection, this, [this]{
        while (server->hasPendingConnections()) {
          QWebSocket *c = server->nextPendingConnection();
          c->setMaxAllowedIncomingMessageSize(maxMessageSizeBytes);
          c->setMaxAllowedIncomingFrameSize(maxMessageSizeBytes);
          // Drop the client on any error / disconnect
          QObject::connect(c, &QWebSocket::disconnected, this, [this, c]{
            clients.removeAll(c);
            c->deleteLater();
          });
          clients.append(c);
        }
      });
      if (!server->listen(addr, port)) {
        ERROR_LOG("WSImageOutput: failed to bind " << addr.toString().toStdString()
                  << ":" << port << " (" << server->errorString().toStdString() << ")");
      }
    }

    void stop() {
      for (auto *c : clients) {
        c->close();
        c->deleteLater();
      }
      clients.clear();
      if (server) { server->close(); server->deleteLater(); server = nullptr; }
    }

    // Broadcast: called on the WS thread (via QueuedConnection from send()).
    void broadcast(QByteArray bytes) {
      const qint64 n = bytes.size();
      // Iterate a snapshot — disconnect signals can mutate `clients` reentrantly.
      const auto snapshot = clients;
      int delivered = 0;
      for (auto *c : snapshot) {
        if (c->state() != QAbstractSocket::ConnectedState) continue;
        const qint64 written = c->sendBinaryMessage(bytes);
        if (written == n) ++delivered;
        // (write failures are surfaced via the QWebSocket::error signal +
        //  the disconnected handler above will clean up.)
      }
      if (delivered > 0) {
        bytesSent += static_cast<qint64>(delivered) * n;
        ++framesSent;
      }
    }
  };

  // ------------------------------------------------------------- pimpl --
  struct WSImageOutput::Data {
    ImageCompressor compressor;
    QThread *thread = nullptr;
    WSImageOutputServer *server = nullptr;
    QString bindAddress;
    quint16 requestedPort = 0;
    std::atomic<int> actualPort{-1};

    // Cached property snapshot — read on the caller thread before the
    // QueuedConnection hop, so we don't need to touch m_applyMutex
    // from the WS thread.
    qint64 maxMessageSizeBytes = 256LL * 1024 * 1024;

    Data(const std::string &bind, int port)
      : bindAddress(QString::fromStdString(bind)),
        requestedPort(static_cast<quint16>(port)) {
      thread = new QThread;
      server = new WSImageOutputServer;
      server->moveToThread(thread);
      thread->start();
      // Schedule bind on the WS thread, then publish the actual port.
      QHostAddress addr;
      if (!addr.setAddress(bindAddress)) addr = QHostAddress::Any;
      QMetaObject::invokeMethod(server, [this, addr]{
        server->maxMessageSizeBytes = maxMessageSizeBytes;
        server->start(addr, requestedPort);
        actualPort = server->server ? server->server->serverPort() : -1;
      }, Qt::BlockingQueuedConnection);
    }

    ~Data() {
      if (server && thread) {
        QMetaObject::invokeMethod(server, [this]{ server->stop(); },
                                  Qt::BlockingQueuedConnection);
      }
      if (thread) {
        thread->quit();
        thread->wait();
        delete thread;
      }
      // server has parent-less child sockets that get cleaned in stop();
      // server itself was moved-to-thread but not parented, so delete here.
      delete server;
    }
  };

  // -------------------------------------------------------- public API --
  WSImageOutput::WSImageOutput(int port, const std::string &bindAddress)
    : m_data(nullptr) {
    ensureQCoreApplication();
    setConfigurableID(str("ws:")+bindAddress+":"+str(port));
    // No `compression` / `quality` here — they're exposed as a child
    // Configurable under the `compression.` prefix once `m_data` is
    // built (see below). The user sees `compression.mode` (a menu of
    // every registered codec) plus the active codec's own tunables
    // (`compression.quality` / `compression.level` / …) which appear
    // and disappear with the active codec selection.
    addProperty("max message size MB", "range", "[1,4096]:1", "256", 0,
                "Per-frame WebSocket message size cap. Default 256 MB.");
    addProperty("bind address", "info", "", bindAddress, 0,
                "Interface the server is bound to.");
    addProperty("port", "info", "", str(port), 0,
                "Bound TCP port (resolved at construction time).");
    addProperty("clients", "info", "", "0", 200, "Live connected client count.");
    addProperty("bytes sent", "info", "", "0", 200, "Lifetime bytes broadcast.");
    addProperty("frames sent", "info", "", "0", 200, "Lifetime frames broadcast.");
    Configurable::registerCallback([this](const Property &p){ onPropertyChange(p); });

    m_data = new Data(bindAddress, port);
    // The compressor is fully initialized in its ctor (codec, mode prop,
    // active plugin) — safe because no ImageCompressor is constructed
    // during static init (FileWriter/FileGrabber are factory-based).
    addChildConfigurable(&m_data->compressor, "compression");
    if (m_data->actualPort < 0) {
      // bind failed — leave m_data alive (with isNull-like state we can't
      // express cleanly without a flag), but flip the property and warn.
      ERROR_LOG("WSImageOutput: server failed to bind, output is non-functional");
    } else {
      setPropertyValue("port", str(static_cast<int>(m_data->actualPort)));
    }
  }

  WSImageOutput::~WSImageOutput() {
    delete m_data;
  }

  int WSImageOutput::actualPort() const {
    return m_data ? static_cast<int>(m_data->actualPort) : -1;
  }

  int WSImageOutput::connectedClients() const {
    if (!m_data || !m_data->server) return 0;
    int n = 0;
    QMetaObject::invokeMethod(m_data->server, [this, &n]{
      n = m_data->server->clients.size();
    }, Qt::BlockingQueuedConnection);
    return n;
  }

  void WSImageOutput::send(const core::Image &image) {
    if (!m_data || !m_data->server || image.isNull()) return;

    // The compressor is a child Configurable owned by us — its codec
    // selection and per-codec params were already applied via the
    // property mechanism (see addChildConfigurable in the ctor).
    const ImageCompressor::CompressedData cd =
      m_data->compressor.compress(image, false);

    // Copy into a QByteArray (owns its bytes for the cross-thread hop).
    QByteArray bytes(reinterpret_cast<const char*>(cd.bytes),
                     static_cast<int>(cd.len));

    QMetaObject::invokeMethod(m_data->server,
      [s = m_data->server, b = std::move(bytes)]{ s->broadcast(b); },
      Qt::QueuedConnection);

    // Refresh live-info properties (cheap, off the WS thread).
    setPropertyValue("clients", str(static_cast<int>(m_data->server->clients.size())));
    setPropertyValue("bytes sent", str(static_cast<long long>(m_data->server->bytesSent)));
    setPropertyValue("frames sent", str(static_cast<long long>(m_data->server->framesSent)));
  }

  void WSImageOutput::onPropertyChange(const Property &p) {
    // `compression.*` properties live on the inner ImageCompressor child
    // and are handled there — we only see local properties here.
    if (p.name == "max message size MB") {
      const qint64 mb = parse<int>(p.value);
      m_data->maxMessageSizeBytes = mb * 1024 * 1024;
      // Apply to existing+future clients on the WS thread
      if (m_data->server) {
        QMetaObject::invokeMethod(m_data->server, [this]{
          m_data->server->maxMessageSizeBytes = m_data->maxMessageSizeBytes;
          for (auto *c : m_data->server->clients) {
            c->setMaxAllowedIncomingMessageSize(m_data->maxMessageSizeBytes);
            c->setMaxAllowedIncomingFrameSize(m_data->maxMessageSizeBytes);
          }
        }, Qt::QueuedConnection);
      }
    }
  }

} // namespace icl::io

// ----- registration with GenericImageOutput -----------------------------
#include <icl/io/GenericImageOutput.h>
REGISTER_IMAGE_OUTPUT(ws, "ws",
  ([](const std::string &params) -> icl::io::ImageOutputFn {
    // params form: "PORT" (bind 0.0.0.0) or "BIND:PORT"
    std::string bind = "0.0.0.0";
    std::string portStr = params;
    const auto colon = params.find(':');
    if (colon != std::string::npos) {
      bind = params.substr(0, colon);
      portStr = params.substr(colon + 1);
    }
    auto impl = std::make_shared<icl::io::WSImageOutput>(
        icl::utils::parse<int>(portStr), bind);
    return [impl](const icl::core::Image &img) { impl->send(img); };
  }),
  "PORT or BIND:PORT~WebSocket server (broadcasts ImageCompressor envelopes to all clients)")
