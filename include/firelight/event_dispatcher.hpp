#pragma once
#include <any>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <typeindex>

// A handle that automatically unsubscribes when it goes out of scope
class ScopedConnection {
public:
  ScopedConnection() = default;
  explicit ScopedConnection(std::function<void()> disconnect)
      : m_disconnect(std::move(disconnect)) {}
  ~ScopedConnection() {
    if (m_disconnect) {
      m_disconnect();
    }
  }
  ScopedConnection(ScopedConnection &&other) noexcept
      : m_disconnect(std::move(other.m_disconnect)) {}

  ScopedConnection &operator=(ScopedConnection &&other) noexcept {
    if (this != &other) {
      if (m_disconnect) {
        m_disconnect();
      }

      m_disconnect = std::move(other.m_disconnect);
    }
    return *this;
  }

private:
  std::function<void()> m_disconnect;
};

// Threading: subscribe()/publish() are called from many threads (render, SDL
// input, GUI) and are mutex-guarded. Callbacks run synchronously on the
// publishing thread, so a subscriber must not assume its own thread affinity (a
// render-thread publish runs GUI subscribers there)
//
// Owned objects should take an EventDispatcher& by injection; instance() is the
// process-wide default, kept for QML-constructed types that can't receive
// constructor arguments
class EventDispatcher {
public:
  EventDispatcher() = default;

  static EventDispatcher &instance() {
    static EventDispatcher dispatcher;
    return dispatcher;
  }

  template <typename TEvent>
  [[nodiscard]] ScopedConnection
  subscribe(std::function<void(const TEvent &)> callback) {
    auto type = std::type_index(typeid(TEvent));

    std::list<std::function<void(const std::any &)>>::iterator it;
    {
      std::lock_guard lock(m_mutex);
      auto &subscribers = m_subscribers[type];
      // Using a list to prevent iterator invalidation on add/remove
      it = subscribers.emplace(
          subscribers.end(), [cb = std::move(callback)](const std::any &event) {
            cb(std::any_cast<const TEvent &>(event));
          });
    }

    return ScopedConnection([this, type, it]() {
      std::lock_guard lock(m_mutex);
      m_subscribers[type].erase(it);
    });
  }

  template <typename TEvent> void publish(const TEvent &event) {
    auto type = std::type_index(typeid(TEvent));
    // Snapshot the callbacks under the lock, then invoke them outside it: a
    // callback may (un)subscribe or re-publish, which would otherwise deadlock
    std::list<std::function<void(const std::any &)>> callbacks;
    {
      std::lock_guard lock(m_mutex);
      if (const auto it = m_subscribers.find(type); it != m_subscribers.end()) {
        callbacks = it->second;
      }
    }
    for (const auto &callback : callbacks) {
      callback(event);
    }
  }

private:
  std::mutex m_mutex;
  std::map<std::type_index, std::list<std::function<void(const std::any &)>>>
      m_subscribers;
};
