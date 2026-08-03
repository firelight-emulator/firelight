#pragma once

#include <QQuickItem>
#include <QRectF>
#include <vector>

namespace firelight::gui {

/**
 * One thing the cursor could move to, and where it is
 */
struct FocusCandidate {
  QQuickItem *item = nullptr;
  QRectF rect;
  /** The container that answers for this when a press asks what it can reach, or null */
  QQuickItem *container = nullptr;
};

/**
 * Walks an item tree gathering everything the cursor could move to.
 *
 * Whole branches are skipped rather than filtered one item at a time, so an entire hidden page or a
 * closed panel costs nothing to pass over.
 */
class CandidateCollector {
public:
  /**
   * Everything below root that could take the cursor, in walk order.
   *
   * Walk order is what breaks exact geometric ties, so it has to be stable
   */
  static std::vector<FocusCandidate> collect(QQuickItem *root);

  /**
   * The subtree a press starting at origin is confined to: the nearest ancestor that declares
   * itself a barrier, or fallback when none does.
   *
   * This is what keeps arrows inside an open popup, and out of one that is not focused
   */
  static QQuickItem *scopeFor(QQuickItem *origin, QQuickItem *fallback);

  /**
   * The scene rectangle an item is navigated by, which is the one the cursor draws around
   */
  static QRectF rectFor(QQuickItem *item);

  /**
   * The nearest ancestor-or-self declaring itself a container, or null
   */
  static QQuickItem *containerFor(QQuickItem *item);

  /**
   * The nearest ancestor-or-self of item that collect() would have gathered, or null.
   *
   * The focused item is whatever holds key focus, which may be a child of the thing the cursor is
   * actually drawn around
   */
  static QQuickItem *candidateFor(QQuickItem *item);
};

} // namespace firelight::gui
