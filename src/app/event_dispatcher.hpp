#pragma once
#include <any>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <typeindex>

// A handle that automatically unsubscribes when it goes out of scope.
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

class EventDispatcher {
public:
  static EventDispatcher &instance() {
    static EventDispatcher dispatcher;
    return dispatcher;
  }

  template <typename TEvent>
  [[nodiscard]] ScopedConnection
  subscribe(std::function<void(const TEvent &)> callback) {
    auto type = std::type_index(typeid(TEvent));
    auto &subscribers = m_subscribers[type];

    // Using a list to prevent iterator invalidation on add/remove.
    auto it = subscribers.emplace(
        subscribers.end(), [cb = std::move(callback)](const std::any &event) {
          cb(std::any_cast<const TEvent &>(event));
        });

    // Return a ScopedConnection that knows how to remove this specific
    // subscriber.
    return ScopedConnection(
        [this, type, it]() { m_subscribers[type].erase(it); });
  }

  template <typename TEvent> void publish(const TEvent &event) {
    auto type = std::type_index(typeid(TEvent));
    if (m_subscribers.contains(type)) {
      // Iterate over a copy in case a callback modifies the subscriber list.
      auto callbacks = m_subscribers.at(type);
      for (const auto &callback : callbacks) {
        callback(event);
      }
    }
  }

private:
  EventDispatcher() = default;
  std::map<std::type_index, std::list<std::function<void(const std::any &)>>>
      m_subscribers;
};