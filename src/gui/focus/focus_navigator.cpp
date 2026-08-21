#include "focus_navigator.hpp"

#include "candidate_collector.hpp"
#include "gui/focus_info.hpp"

#include <QDateTime>
#include <QQuickWindow>
#include <algorithm>

namespace firelight::gui {

namespace {
/**
 * The edge flag standing for a direction
 */
FocusInfo::Edge edgeFor(const Direction direction) {
  switch (direction) {
  case Direction::Up:
    return FocusInfo::Up;
  case Direction::Down:
    return FocusInfo::Down;
  case Direction::Left:
    return FocusInfo::Left;
  case Direction::Right:
    return FocusInfo::Right;
  }

  return FocusInfo::NoEdges;
}

/**
 * The direction that undoes one
 */
Direction opposite(const Direction direction) {
  switch (direction) {
  case Direction::Up:
    return Direction::Down;
  case Direction::Down:
    return Direction::Up;
  case Direction::Left:
    return Direction::Right;
  case Direction::Right:
    return Direction::Left;
  }

  return direction;
}

/**
 * Whether one item is the other, or holds it
 */
bool holds(const QQuickItem *outer, QQuickItem *inner) { return outer == inner || outer->isAncestorOf(inner); }

/**
 * The whole of what is on screen, which is as far as a search can reach when nothing narrows it
 */
QQuickItem *rootOf(QQuickItem *item) {
  if (item->window() != nullptr) {
    return item->window()->contentItem();
  }

  auto *top = item;

  while (top->parentItem() != nullptr) {
    top = top->parentItem();
  }

  return top;
}
} // namespace

bool RepeatGovernor::allows(const Direction direction, const bool isAutoRepeat, const qint64 nowMs) {
  const auto isSustained = isAutoRepeat && m_hasMoved && direction == m_lastDirection;

  if (isSustained && nowMs - m_lastMoveMs < INTERVAL_MS) {
    return false;
  }

  m_lastDirection = direction;
  m_lastMoveMs = nowMs;
  m_hasMoved = true;

  return true;
}

void RepeatGovernor::reset() { m_hasMoved = false; }

std::pair<Direction, bool> FocusNavigator::directionFor(const int key) {
  switch (key) {
  case Qt::Key_Up:
    return {Direction::Up, true};
  case Qt::Key_Down:
    return {Direction::Down, true};
  case Qt::Key_Left:
    return {Direction::Left, true};
  case Qt::Key_Right:
    return {Direction::Right, true};
  default:
    return {Direction::Down, false};
  }
}

bool FocusNavigator::isHeldBack(QQuickItem *from, QQuickItem *to, const Direction direction, const bool isAutoRepeat) {
  if (!isAutoRepeat || from == nullptr || to == nullptr) {
    return false;
  }

  const auto edge = edgeFor(direction);

  for (auto *item = from; item != nullptr; item = item->parentItem()) {
    const auto *info = FocusInfo::find(item);

    if (info != nullptr && (info->getHoldEdges() & edge) && !holds(item, to)) {
      return true;
    }
  }

  return false;
}

int FocusNavigator::enterContainer(const QRectF &origin, const Direction direction, QQuickItem *leaving,
                                   const std::vector<FocusCandidate> &candidates) {
  std::vector<QQuickItem *> containers;
  std::vector<QRectF> bounds;

  for (const auto &candidate : candidates) {
    if (candidate.container == nullptr || candidate.container == leaving ||
        std::find(containers.begin(), containers.end(), candidate.container) != containers.end()) {
      continue;
    }

    containers.push_back(candidate.container);
    bounds.push_back(CandidateCollector::rectFor(candidate.container));
  }

  // The same rule the candidates were judged by, asked of the containers: a press only enters one it
  // lines up with, so a rail running out of buttons still does not fall into the grid beside it
  const auto entered = SpatialResolver::pick(origin, direction, bounds);

  if (entered == SpatialResolver::NONE) {
    return SpatialResolver::NONE;
  }

  std::vector<QRectF> inside;
  std::vector<int> indexes;

  for (size_t i = 0; i < candidates.size(); ++i) {
    if (candidates[i].container == containers[entered]) {
      inside.push_back(candidates[i].rect);
      indexes.push_back(static_cast<int>(i));
    }
  }

  const auto row = SpatialResolver::pickNearestLane(origin, direction, inside);

  return row == SpatialResolver::NONE ? SpatialResolver::NONE : indexes[row];
}

void FocusNavigator::forget() {
  m_steppedFrom = nullptr;
  m_steppedTo = nullptr;
}

QQuickItem *FocusNavigator::wayBack(QQuickItem *origin, const Direction direction, const bool isAutoRepeat) const {
  if (isAutoRepeat || m_steppedFrom.isNull() || m_steppedTo != origin || direction != opposite(m_steppedDirection)) {
    return nullptr;
  }

  if (!m_steppedFrom->isVisible() || !m_steppedFrom->isEnabled()) {
    return nullptr;
  }

  return m_steppedFrom;
}

void FocusNavigator::land(QQuickItem *from, QQuickItem *to, const Direction direction) {
  m_landing = true;
  to->forceActiveFocus(Qt::OtherFocusReason);
  m_landing = false;

  m_steppedFrom = from;
  m_steppedTo = to;
  m_steppedDirection = direction;
}

void FocusNavigator::watch(QQuickItem *item) {
  auto *window = item->window();

  if (window == nullptr || window == m_window) {
    return;
  }

  if (!m_window.isNull()) {
    disconnect(m_window, &QQuickWindow::activeFocusItemChanged, this, nullptr);
  }

  m_window = window;

  connect(window, &QQuickWindow::activeFocusItemChanged, this, [this] {
    if (!m_landing) {
      forget();
    }
  });
}

int FocusNavigator::move(QQuickItem *origin, const int key, const bool isAutoRepeat) {
  const auto [direction, isDirectional] = directionFor(key);

  if (!isDirectional || origin == nullptr) {
    return NoTarget;
  }

  auto *from = CandidateCollector::candidateFor(origin);

  if (from == nullptr) {
    return NoTarget;
  }

  watch(from);

  // Thinning happens before the search, so a held direction costs one tree walk per move rather
  // than one per event
  if (!m_governor.allows(direction, isAutoRepeat, QDateTime::currentMSecsSinceEpoch())) {
    return Moved;
  }

  if (auto *back = wayBack(from, direction, isAutoRepeat); back != nullptr) {
    land(from, back, direction);
    return Moved;
  }

  const auto candidates = CandidateCollector::collect(CandidateCollector::scopeFor(from, rootOf(from)));

  std::vector<QRectF> rects;
  rects.reserve(candidates.size());

  for (const auto &candidate : candidates) {
    rects.push_back(candidate.rect);
  }

  const auto fromRect = CandidateCollector::rectFor(from);
  auto picked = SpatialResolver::pick(fromRect, direction, rects);

  if (picked == SpatialResolver::NONE) {
    picked = enterContainer(fromRect, direction, CandidateCollector::containerFor(from), candidates);
  }

  if (picked == SpatialResolver::NONE) {
    return NoTarget;
  }

  auto *to = candidates[picked].item;

  if (isHeldBack(from, to, direction, isAutoRepeat)) {
    return NoTarget;
  }

  land(from, to, direction);

  return Moved;
}

} // namespace firelight::gui
