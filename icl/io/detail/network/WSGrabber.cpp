// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/network/WSGrabber.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/ImageCompressor.h>
#include <icl/core/Image.h>
#include <icl/core/ImgBase.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Time.h>
#include <icl/utils/Macros.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QByteArray>
#include <QtCore/QMetaObject>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QDateTime>
#include <QtWebSockets/QWebSocket>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

namespace icl::io {
  using namespace icl::utils;
  using namespace icl::core;

  // See WSImageOutput.cpp for rationale: WS classes need a QCoreApplication
  // in headless contexts (e.g. `icl-pipe -no-gui`).
  static void ensureQCoreApplication() {
    if (QCoreApplication::instance() != nullptr) return;
    static int    s_argc = 1;
    static char   s_arg0[] = "icl-ws";
    static char  *s_argv[] = {s_arg0, nullptr};
    static QCoreApplication s_app(s_argc, s_argv);
    (void)s_app;
  }

  // ----------------------------------------------------------------- Impl --
  // Owns a QWebSocket on a private QThread, runs the reconnect state
  // machine, and pushes uncompressed `ImageBase*` (well — compressed
  // bytes; uncompress lazily on the consumer thread) into a bounded
  // queue. No Q_OBJECT macro: function-pointer connect() + lambdas
  // suffice in Qt6.
  // -----------------------------------------------------------------------
  enum class ConnState { Disconnected, Connecting, Connected };

  static const char *connStateName(ConnState s) {
    switch (s) {
      case ConnState::Disconnected: return "disconnected";
      case ConnState::Connecting:   return "connecting";
      case ConnState::Connected:    return "connected";
    }
    return "unknown";
  }

  class WSGrabberClient : public QObject {
  public:
    QWebSocket *ws = nullptr;
    QUrl url;

    int backoffInitialMs = 250;
    int backoffMaxMs = 5000;
    int currentBackoffMs = 250;

    // Frame queue: payload + receive timestamp for latency calculation.
    struct Frame {
      QByteArray bytes;
      qint64 recvUsec;
    };
    std::deque<Frame> queue;
    int queueCap = 2;

    // For the consumer side
    std::mutex qMutex;
    std::condition_variable qCv;

    // counters / state (atomic so getters can read freely)
    std::atomic<ConnState> state{ConnState::Disconnected};
    std::atomic<int>       reconnectAttempts{0};
    std::atomic<qint64>    framesReceived{0};
    std::atomic<qint64>    framesDropped{0};
    std::atomic<qint64>    bytesReceived{0};
    std::atomic<qint64>    lastConnectedUsec{0};
    std::atomic<qint64>    lastLatencyMs{0};

    // Set true by the destructor to short-circuit any pending reconnect timer
    std::atomic<bool>      shuttingDown{false};

    void start(const QUrl &u) {
      url = u;
      ws = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
      ws->setMaxAllowedIncomingMessageSize(256LL * 1024 * 1024);
      ws->setMaxAllowedIncomingFrameSize(256LL * 1024 * 1024);

      QObject::connect(ws, &QWebSocket::connected, this, [this]{
        state = ConnState::Connected;
        currentBackoffMs = backoffInitialMs;
        lastConnectedUsec = QDateTime::currentMSecsSinceEpoch() * 1000;
      });
      QObject::connect(ws, &QWebSocket::disconnected, this, [this]{
        state = ConnState::Disconnected;
        scheduleReconnect();
      });
      QObject::connect(ws, &QWebSocket::errorOccurred, this,
                       [this](QAbstractSocket::SocketError){
        // errorOccurred fires before disconnected during a failed connect;
        // state may already be Connecting. The disconnect handler is the
        // single point that schedules reconnect.
        if (state != ConnState::Connected) {
          state = ConnState::Disconnected;
          scheduleReconnect();
        }
      });
      QObject::connect(ws, &QWebSocket::binaryMessageReceived, this,
                       [this](const QByteArray &msg){
        const qint64 nowUs = QDateTime::currentMSecsSinceEpoch() * 1000;
        bytesReceived += msg.size();
        ++framesReceived;
        {
          std::scoped_lock lk(qMutex);
          // drop-oldest policy
          while (static_cast<int>(queue.size()) >= queueCap) {
            queue.pop_front();
            ++framesDropped;
          }
          queue.push_back({msg, nowUs});
        }
        qCv.notify_one();
      });

      tryConnect();
    }

    void tryConnect() {
      if (shuttingDown) return;
      state = ConnState::Connecting;
      ++reconnectAttempts;
      ws->open(url);
    }

    void scheduleReconnect() {
      if (shuttingDown) return;
      const int delay = currentBackoffMs;
      currentBackoffMs = std::min(currentBackoffMs * 2, backoffMaxMs);
      QTimer::singleShot(delay, this, [this]{ tryConnect(); });
    }

    void stop() {
      shuttingDown = true;
      if (ws) {
        ws->close();
        ws->deleteLater();
        ws = nullptr;
      }
      // Wake any blocked consumer
      qCv.notify_all();
    }
  };

  // ------------------------------------------------------------- pimpl --
  struct WSGrabber::Data {
    ImageCompressor compressor;          // consumer-thread side
    QThread *thread = nullptr;
    WSGrabberClient *client = nullptr;
    QUrl url;

    // Consumer-side knobs (mirrored from properties; read on consumer thread)
    std::atomic<int>  blockTimeoutMs{1000};
    std::atomic<bool> replayLastOnTimeout{true};

    // Last successfully decoded frame, kept for replay on timeout. The
    // ImageCompressor's internal buffer would be overwritten by the next
    // call to uncompress(), so we keep our own deep-copy here.
    Image  lastFrame;

    Data(const std::string &u) : url(QString::fromStdString(u)) {
      // (URL is normalized in WSGrabber::WSGrabber before reaching here)
      thread = new QThread;
      client = new WSGrabberClient;
      client->moveToThread(thread);
      thread->start();
      QMetaObject::invokeMethod(client, [this]{ client->start(url); },
                                Qt::QueuedConnection);
    }

    ~Data() {
      if (client && thread) {
        QMetaObject::invokeMethod(client, [this]{ client->stop(); },
                                  Qt::BlockingQueuedConnection);
      }
      if (thread) {
        thread->quit();
        thread->wait();
        delete thread;
      }
      delete client;
    }
  };

  // -------------------------------------------------------- public API --

  // Accept any of:
  //   "9999"               → ws://localhost:9999
  //   "host:9999"          → ws://host:9999
  //   "ws://host:9999"     → unchanged
  //   "ws://host:9999/p"   → unchanged (path preserved for v2 multi-stream)
  // Fail loudly on inputs that look like a hostname with no port — those
  // are almost always typos (the user meant a port).
  static std::string normalizeWSUrl(const std::string &raw) {
    if (raw.find("://") != std::string::npos) return raw;
    // bare digits → port on localhost
    if (!raw.empty() &&
        raw.find_first_not_of("0123456789") == std::string::npos) {
      return std::string("ws://localhost:") + raw;
    }
    // contains ':' → assume host:port (preserve unchanged after prefix)
    if (raw.find(':') != std::string::npos) {
      return std::string("ws://") + raw;
    }
    // bare host with no port and no scheme — almost always a mistake
    throw ICLException("WSGrabber: ambiguous URL '" + raw + "'. "
                       "Expected one of: PORT (digits → localhost:PORT), "
                       "HOST:PORT, or ws://HOST:PORT");
  }

  WSGrabber::WSGrabber(const std::string &url) : m_data(nullptr) {
    ensureQCoreApplication();
    const std::string normalized = normalizeWSUrl(url);
    setConfigurableID(str("ws:")+normalized);
    addProperty("connection state",            prop::Info{}, "disconnected", 200);
    addProperty("reconnect attempts",          prop::Info{}, "0",            200);
    addProperty("last connected",              prop::Info{}, "never",        200);
    addProperty("reconnect backoff initial ms",prop::Range{.min=10, .max=60000, .step=1}, 250, 0,
                "Initial delay before retrying after a disconnect.");
    addProperty("reconnect backoff max ms",    prop::Range{.min=10, .max=300000, .step=1}, 5000, 0,
                "Cap on the exponential backoff between reconnect attempts.");
    addProperty("block timeout ms",            prop::Range{.min=0, .max=60000, .step=1}, 1000, 0,
                "Max time acquireImage() blocks for a fresh frame "
                "before falling back to the last known frame.");
    addProperty("replay last on timeout",      prop::Flag{}, "true",        0,
                "If true, return the last successfully received frame "
                "when block timeout expires; if false, return null.");
    addProperty("queue size",                  prop::Range{.min=1, .max=64, .step=1}, 2,  0,
                "Max frames buffered. Drop-oldest policy when exceeded.");
    addProperty("frames dropped",              prop::Info{}, "0",            200);
    addProperty("frames received",             prop::Info{}, "0",            200);
    addProperty("bytes received",              prop::Info{}, "0",            200);

    registerCallback([this](const Property &p){
      if (!m_data || !m_data->client) return;
      if (p.name == "block timeout ms") {
        m_data->blockTimeoutMs = p.as<int>();
      } else if (p.name == "replay last on timeout") {
        m_data->replayLastOnTimeout = (p.as<std::string>() == "true");
      } else if (p.name == "queue size") {
        const int q = p.as<int>();
        QMetaObject::invokeMethod(m_data->client, [c = m_data->client, q]{
          std::scoped_lock lk(c->qMutex);
          c->queueCap = std::max(1, q);
        }, Qt::QueuedConnection);
      } else if (p.name == "reconnect backoff initial ms") {
        const int v = p.as<int>();
        m_data->client->backoffInitialMs = v;
      } else if (p.name == "reconnect backoff max ms") {
        const int v = p.as<int>();
        m_data->client->backoffMaxMs = v;
      }
    });

    m_data = new Data(normalized);
  }

  WSGrabber::~WSGrabber() {
    delete m_data;
  }

  const std::vector<GrabberDeviceDescription>&
  WSGrabber::getDeviceList([[maybe_unused]] bool rescan) {
    static std::vector<GrabberDeviceDescription> empty;
    return empty;
  }

  const core::ImgBase *WSGrabber::acquireImage() {
    if (!m_data || !m_data->client) return nullptr;
    auto *c = m_data->client;

    // Refresh live-info properties cheaply — these are atomics, no locking.
    // Silent updates: called from acquireImage() under m_grabMutex; firing
    // callbacks here would deadlock against a GUI thread changing another
    // property (qt::Prop holds execMutex, wants m_grabMutex).  GUI polls
    // these Info properties via VolatileUpdater.
    setPropertyValueSilent("connection state",
        str(connStateName(c->state.load())) +
        (c->state == ConnState::Disconnected
           ? str(" (next retry in ~") + str(c->currentBackoffMs) + "ms)"
           : str("")));
    setPropertyValueSilent("reconnect attempts", str(static_cast<long long>(c->reconnectAttempts)));
    setPropertyValueSilent("frames received",    str(static_cast<long long>(c->framesReceived)));
    setPropertyValueSilent("frames dropped",     str(static_cast<long long>(c->framesDropped)));
    setPropertyValueSilent("bytes received",     str(static_cast<long long>(c->bytesReceived)));
    if (c->lastConnectedUsec > 0) {
      setPropertyValueSilent("last connected",
          Time(static_cast<Time::value_type>(c->lastConnectedUsec))
            .toStringFormated("%H:%M:%S"));
    }

    // Wait for a fresh frame, with the timeout configured by the user.
    QByteArray bytes;
    qint64 recvUsec = 0;
    {
      std::unique_lock lk(c->qMutex);
      const int timeoutMs = m_data->blockTimeoutMs;
      const bool got = c->qCv.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                                       [c]{ return !c->queue.empty(); });
      if (got) {
        auto f = c->queue.front();
        c->queue.pop_front();
        bytes = std::move(f.bytes);
        recvUsec = f.recvUsec;
      }
      // else: timeout — handled below
    }

    if (bytes.isEmpty()) {
      // No fresh frame — replay last if allowed.
      if (m_data->replayLastOnTimeout && !m_data->lastFrame.isNull()) {
        return m_data->lastFrame.ptr();
      }
      return nullptr;
    }

    const qint64 nowUs = QDateTime::currentMSecsSinceEpoch() * 1000;
    c->lastLatencyMs = (nowUs - recvUsec) / 1000;

    Image decoded;
    try {
      decoded = m_data->compressor.uncompress(
        reinterpret_cast<const icl8u*>(bytes.constData()),
        static_cast<int>(bytes.size()));
    } catch (const ICLException &e) {
      ERROR_LOG("WSGrabber: failed to decode frame: " << e.what());
    }
    if (decoded.isNull()) {
      // Bad frame; keep last known if any.
      return m_data->replayLastOnTimeout && !m_data->lastFrame.isNull()
             ? m_data->lastFrame.ptr() : nullptr;
    }

    // ImageCompressor returns an Image whose backing buffer it also keeps
    // a reference to (for next decompress() reuse). Deep-copy here so the
    // pointer we return to the consumer stays valid past the next
    // acquireImage() call.
    m_data->lastFrame = Image(decoded.ptr()->deepCopy());
    return m_data->lastFrame.ptr();
  }

  // ----- registration with GenericGrabber ---------------------------------

  REGISTER_CONFIGURABLE(WSGrabber,
                        return new WSGrabber("ws://localhost:9090"));

  static Grabber* createWSGrabber(const std::string &param) {
    // Pass straight through — WSGrabber::WSGrabber handles all the
    // accepted shorthand forms (PORT / HOST:PORT / ws://HOST:PORT).
    return new WSGrabber(param);
  }

  static const std::vector<GrabberDeviceDescription>&
  getWSDeviceList(std::string filter, bool /*rescan*/) {
    static std::vector<GrabberDeviceDescription> deviceList;
    deviceList.clear();
    if (filter.size()) {
      deviceList.emplace_back(GrabberDeviceDescription(
        "ws", filter, "WebSocket-based network grabber"));
    }
    return deviceList;
  }

  REGISTER_GRABBER(ws, createWSGrabber, getWSDeviceList,
                   "ws:ws\\://host\\:port (URL of the publishing server) "
                   ":WebSocket-based network grabber")

} // namespace icl::io
