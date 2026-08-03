#include "candidate_collector.hpp"

#include "gui/focus_info.hpp"

#include <optional>

namespace firelight::gui {

namespace {
// TODO
// How much of an item may hang over the edge of what holds it before it stops counting as on
// display, so a rounded pixel does not decide it
constexpr qreal VISIBLE_SLACK = 0.5;

/**
 * Where an item sits in scene coordinates
 */
QRectF sceneRect(const QQuickItem *item) {
  return item->mapRectToScene(QRectF(0.0, 0.0, item->width(), item->height()));
}

/**
 * The shape a candidate is measured by, which is the one the cursor draws around: an item may hand
 * that job to something else, and navigation has to agree with what is on screen
 */
QRectF targetRect(const QQuickItem *item, const FocusInfo *info) {
  if (info != nullptr && info->getProxy() != nullptr) {
    const auto proxied = sceneRect(info->getProxy());

    if (!proxied.isEmpty()) {
      return proxied;
    }
  }

  return sceneRect(item);
}

/**
 * Whether an item's bounds narrow what is reachable inside it.
 *
 * A view keeps delegates alive outside what it shows — recycled, buffered, or the current one left
 * behind by scrolling — at positions that no longer mean anything, so it is bounded whether or not
 * it clips. Anything else scrolling is taken at its word: what it holds is where it says it is
 */
bool boundsCandidates(const QQuickItem *item) {
  if (item->inherits("QQuickItemView")) {
    return true;
  }

  if (!item->clip()) {
    return false;
  }

  return !item->inherits("QQuickFlickable") &&
         (item->parentItem() == nullptr || !item->parentItem()->inherits("QQuickFlickable"));
}

/**
 * Whether a rectangle is entirely on display within what bounds it
 */
bool isWhollyWithin(const QRectF &rect, const std::optional<QRectF> &clip) {
  if (!clip.has_value()) {
    return true;
  }

  return clip->contains(rect.adjusted(VISIBLE_SLACK, VISIBLE_SLACK, -VISIBLE_SLACK, -VISIBLE_SLACK));
}

/**
 * Whether an item stands for the things inside it rather than being a target itself
 */
bool isTransparent(const QQuickItem *item, const FocusInfo *info) {
  const auto mode = info != nullptr ? info->getMode() : FocusInfo::Normal;

  if (mode == FocusInfo::Group) {
    return true;
  }

  return mode == FocusInfo::Normal && item->isFocusScope();
}

/**
 * Whether an item is somewhere the cursor can land, ignoring where it is
 */
bool isTarget(const QQuickItem *item, const FocusInfo *info) {
  if (info != nullptr && info->getMode() == FocusInfo::Skip) {
    return false;
  }

  return item->focusPolicy() != Qt::NoFocus && !isTransparent(item, info);
}

/**
 * Adds everything below item, narrowing clip as clipping ancestors are passed
 */
void walk(QQuickItem *item, const std::optional<QRectF> &clip, QQuickItem *container,
          std::vector<FocusCandidate> &found) {
  if (item == nullptr || !item->isVisible() || !item->isEnabled() || item->opacity() <= 0.0) {
    return;
  }

  const auto *info = FocusInfo::find(item);
  const auto mode = info != nullptr ? info->getMode() : FocusInfo::Normal;

  if (mode == FocusInfo::Skip) {
    return;
  }

  const auto rect = sceneRect(item);

  if (clip.has_value() && !rect.isEmpty() && !clip->intersects(rect)) {
    return;
  }

  // An item with no size of its own still lays out children around it, so it is passed through
  // rather than treated as absent
  if (isTarget(item, info)) {
    const auto drawn = targetRect(item, info);

    // Half a row showing is not somewhere to move to: landing there would drag the whole container
    // along to finish showing it. The container's own stepping reaches it instead
    if (!drawn.isEmpty() && isWhollyWithin(drawn, clip)) {
      found.push_back({item, drawn, container});
      return;
    }
  }

  if (mode == FocusInfo::Stop) {
    return;
  }

  auto childClip = clip;
  auto *childContainer = info != nullptr && info->isContainer() ? item : container;

  if (boundsCandidates(item)) {
    childClip = clip.has_value() ? clip->intersected(rect) : rect;
  }

  for (auto *child : item->childItems()) {
    walk(child, childClip, childContainer, found);
  }
}
} // namespace

std::vector<FocusCandidate> CandidateCollector::collect(QQuickItem *root) {
  std::vector<FocusCandidate> found;

  if (root == nullptr || !root->isVisible() || !root->isEnabled() || root->opacity() <= 0.0) {
    return found;
  }

  // The root is where the search happens rather than something found by it, so it is descended into
  // even when it would otherwise be a target
  const auto clip = boundsCandidates(root) ? std::optional(sceneRect(root)) : std::nullopt;

  for (auto *child : root->childItems()) {
    walk(child, clip, containerFor(root), found);
  }

  return found;
}

QRectF CandidateCollector::rectFor(QQuickItem *item) {
  if (item == nullptr) {
    return {};
  }

  return targetRect(item, FocusInfo::find(item));
}

QQuickItem *CandidateCollector::containerFor(QQuickItem *item) {
  for (auto *current = item; current != nullptr; current = current->parentItem()) {
    const auto *info = FocusInfo::find(current);

    if (info != nullptr && info->isContainer()) {
      return current;
    }
  }

  return nullptr;
}

QQuickItem *CandidateCollector::scopeFor(QQuickItem *origin, QQuickItem *fallback) {
  for (auto *item = origin; item != nullptr; item = item->parentItem()) {
    const auto *info = FocusInfo::find(item);

    if (info != nullptr && info->isBarrier()) {
      return item;
    }
  }

  return fallback;
}

QQuickItem *CandidateCollector::candidateFor(QQuickItem *item) {
  for (auto *current = item; current != nullptr; current = current->parentItem()) {
    const auto *info = FocusInfo::find(current);

    if (info != nullptr && info->getMode() == FocusInfo::Skip) {
      return nullptr;
    }

    if (isTarget(current, info)) {
      return current;
    }
  }

  return nullptr;
}

} // namespace firelight::gui
