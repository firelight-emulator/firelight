// TODO: NEEDS REVIEW
#include "focus_arbiter.hpp"

#include "candidate_collector.hpp"
#include "gui/focus_info.hpp"

#include <QCoreApplication>
#include <spdlog/spdlog.h>

namespace firelight::gui {

namespace {
/**
 * Whether one item is the other, or holds it
 */
bool holds(const QQuickItem *outer, QQuickItem *inner) {
  return outer != nullptr && inner != nullptr && (outer == inner || outer->isAncestorOf(inner));
}

/**
 * Whether an item is somewhere focus can usefully be
 */
bool isUsable(const QQuickItem *item) { return item != nullptr && item->isVisible() && item->isEnabled(); }

/**
 * An item named for the log: its type and object name, or "null"
 */
std::string describe(const QQuickItem *item) {
  if (item == nullptr) {
    return "null";
  }

  return std::string(item->metaObject()->className()) + "(" + item->objectName().toStdString() + ")";
}
} // namespace

//****************
// FocusSurfaces
//****************

FocusSurfaces &FocusSurfaces::instance() {
  static FocusSurfaces surfaces;
  return surfaces;
}

void FocusSurfaces::add(FocusInfo *info) {
  if (info == nullptr || m_surfaces.contains(info)) {
    return;
  }

  m_surfaces.append(info);
  emit changed();
}

void FocusSurfaces::remove(FocusInfo *info) {
  if (m_surfaces.removeAll(info) > 0) {
    emit changed();
  }
}

//****************
// FocusArbiter
//****************

FocusArbiter::FocusArbiter(QObject *parent) : QObject(parent) {
  connect(&FocusSurfaces::instance(), &FocusSurfaces::changed, this, &FocusArbiter::adoptSurfaces);
  adoptSurfaces();
}

FocusArbiter::~FocusArbiter() {
  for (auto &watch : m_watches) {
    for (const auto &connection : watch.connections) {
      disconnect(connection);
    }
  }
}

void FocusArbiter::setLander(Landing lander) { m_lander = std::move(lander); }

void FocusArbiter::setParker(Landing parker) { m_parker = std::move(parker); }

QQuickItem *FocusArbiter::getItemOf(const FocusInfo *surface) {
  return surface != nullptr ? qobject_cast<QQuickItem *>(surface->parent()) : nullptr;
}

QQuickItem *FocusArbiter::getEntryOf(const FocusInfo *surface) {
  if (surface == nullptr) {
    return nullptr;
  }

  auto *entry = surface->getEntry();

  return entry != nullptr ? entry : getItemOf(surface);
}

QQuickWindow *FocusArbiter::getWindowOf(const FocusInfo *surface) {
  const auto *item = getItemOf(surface);

  return item != nullptr ? item->window() : nullptr;
}

FocusInfo *FocusArbiter::getOwnerInfo(QQuickWindow *window) const {
  if (window == nullptr) {
    return nullptr;
  }

  FocusInfo *best = nullptr;

  for (auto *info : FocusSurfaces::instance().getAll()) {
    if (!info->isSurface() || !info->isSurfaceActive() || getWindowOf(info) != window) {
      continue;
    }

    if (best == nullptr || info->getLayer() > best->getLayer() ||
        (info->getLayer() == best->getLayer() && m_activation.value(info) > m_activation.value(best))) {
      best = info;
    }
  }

  return best;
}

QQuickItem *FocusArbiter::getOwner(QQuickWindow *window) const { return getItemOf(getOwnerInfo(window)); }

bool FocusArbiter::isAllowed(const FocusInfo *owner, QQuickItem *item) {
  if (!isUsable(item)) {
    return false;
  }

  if (holds(getEntryOf(owner), item)) {
    return true;
  }

  if (!holds(getItemOf(owner), item)) {
    return false;
  }

  for (const auto *region : owner->getExclusiveItems()) {
    if (holds(region, item)) {
      return false;
    }
  }

  // TODO
  // Focus resting on something the cursor cannot sit on (a scope, a layout) is not placed, and the
  // surface has said where it enters; only a real target inside the owner is left where it is
  return CandidateCollector::candidateFor(item) != nullptr;
}

void FocusArbiter::request(QQuickWindow *window) {
  if (window == nullptr || m_pending.contains(window)) {
    return;
  }

  m_pending.insert(window);

  QMetaObject::invokeMethod(
      this,
      [this, window = QPointer<QQuickWindow>(window)] {
        m_pending.remove(window);

        if (!window.isNull()) {
          enforceNow(window);
        }
      },
      Qt::QueuedConnection);
}

void FocusArbiter::requestForEntry(QQuickWindow *window) {
  if (window == nullptr) {
    return;
  }

  m_entryMoved.insert(window);
  request(window);
}

void FocusArbiter::enforceNow(QQuickWindow *window) {
  if (window == nullptr || m_isEnforcing) {
    return;
  }

  auto &watch = m_watches[window];
  auto *info = getOwnerInfo(window);
  auto *owner = getItemOf(info);
  auto *entry = getEntryOf(info);
  auto *focused = window->activeFocusItem();
  const auto isEntryMoved = m_entryMoved.remove(window);

  follow(window, watch, owner, entry, focused);

  if (watch.lastOwner != owner) {
    watch.lastOwner = owner;
    emit ownerChanged(window);
  }

  // TODO
  // A window that has lost the keyboard has nothing to enforce; Qt hands focus back on its own
  auto *content = window->contentItem();

  if (content == nullptr || !content->hasFocus()) {
    spdlog::debug("[focus] pass: window has no keyboard focus, nothing to do (focused={})", describe(focused));
    return;
  }

  if (info != nullptr && isUsable(owner) && isUsable(entry) && m_lander) {
    const auto isSatisfied = isEntryMoved ? holds(entry, focused) : isAllowed(info, focused);

    spdlog::debug("[focus] pass: owner={} entry={} focused={} entryMoved={} satisfied={}", describe(owner),
                  describe(entry), describe(focused), isEntryMoved, isSatisfied);

    if (!isSatisfied) {
      m_isEnforcing = true;
      m_lander(entry);
      m_isEnforcing = false;

      focused = window->activeFocusItem();
      follow(window, watch, owner, entry, focused);
      spdlog::debug("[focus] pass: landed, focused={}", describe(focused));
    }
  } else {
    spdlog::debug("[focus] pass: no landing possible: owner={} usable={} entry={} usable={} focused={}",
                  describe(owner), isUsable(owner), describe(entry), isUsable(entry), describe(focused));
  }

  if (focused != nullptr && m_parker) {
    m_isEnforcing = true;
    m_parker(focused);
    m_isEnforcing = false;
  }
}

void FocusArbiter::follow(QQuickWindow *window, Watch &watch, QQuickItem *owner, QQuickItem *entry,
                          QQuickItem *focused) {
  if (watch.owner == owner && watch.entry == entry && watch.focused == focused) {
    return;
  }

  for (const auto &connection : watch.connections) {
    disconnect(connection);
  }

  watch.connections.clear();
  watch.owner = owner;
  watch.entry = entry;
  watch.focused = focused;

  for (auto *item : {owner, entry, focused}) {
    if (item == nullptr) {
      continue;
    }

    const auto ask = [this, window] { request(window); };
    watch.connections.append(connect(item, &QQuickItem::visibleChanged, this, ask));
    watch.connections.append(connect(item, &QQuickItem::enabledChanged, this, ask));
  }
}

void FocusArbiter::adoptSurfaces() {
  const auto &all = FocusSurfaces::instance().getAll();

  for (auto it = m_activation.begin(); it != m_activation.end();) {
    if (all.contains(it.key())) {
      ++it;
    } else {
      m_observed.remove(it.key());
      it = m_activation.erase(it);
    }
  }

  for (auto *info : all) {
    if (!m_observed.contains(info)) {
      observeSurface(info);
    }
  }

  // TODO
  // A surface coming or going can change every owner, and a surface that has just gone has no
  // window left to name
  for (auto it = m_watches.begin(); it != m_watches.end(); ++it) {
    request(it.key());
  }

  for (auto *info : all) {
    request(getWindowOf(info));
  }
}

void FocusArbiter::observeSurface(FocusInfo *info) {
  m_observed.insert(info);

  if (info->isSurfaceActive()) {
    m_activation.insert(info, ++m_activationCounter);
  } else {
    m_activation.insert(info, 0);
  }

  connect(info, &FocusInfo::surfaceActiveChanged, this, [this, info] {
    if (info->isSurfaceActive()) {
      m_activation.insert(info, ++m_activationCounter);
    }

    request(getWindowOf(info));
  });

  connect(info, &FocusInfo::entryChanged, this, [this, info] { requestForEntry(getWindowOf(info)); });
  connect(info, &FocusInfo::layerChanged, this, [this, info] { request(getWindowOf(info)); });
  connect(info, &FocusInfo::exclusiveChanged, this, [this, info] { request(getWindowOf(info)); });
  connect(info, &QObject::destroyed, this, [this, info] {
    m_observed.remove(info);
    m_activation.remove(info);
  });
}

} // namespace firelight::gui
