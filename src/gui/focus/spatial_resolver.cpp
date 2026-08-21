#include "spatial_resolver.hpp"

#include <algorithm>
#include <cmath>

namespace firelight::gui {

namespace {
// TODO
// Slack on the direction test, so pixel rounding never decides whether
// something counts as being ahead
constexpr qreal EDGE_EPSILON = 1.0;

// TODO
// How much being poorly lined up costs, as a fraction of the narrower of the
// two, and how much the remaining centre offset costs on top
constexpr qreal MISALIGN_WEIGHT = 0.5;
constexpr qreal CENTER_WEIGHT = 0.25;

/**
 * Rewrites a rectangle so that "the pressed direction" is always downward, letting one comparison
 * serve all four directions
 */
QRectF orient(const QRectF &rect, const Direction direction) {
  switch (direction) {
  case Direction::Down:
    return rect;
  case Direction::Up:
    return {rect.x(), -rect.bottom(), rect.width(), rect.height()};
  case Direction::Right:
    return {rect.y(), rect.x(), rect.height(), rect.width()};
  case Direction::Left:
    return {rect.y(), -rect.right(), rect.height(), rect.width()};
  }

  return rect;
}

/**
 * Whether a candidate counts as lying ahead of the origin.
 *
 * Its centre has to be past the origin's, and one of its two edges as well: centre alone lets a
 * tall neighbour alongside win. Either edge will do, because reversing the press swaps which of
 * the two is the leading one, and a move that can be made has to be one that can be undone
 */
bool isAhead(const QRectF &origin, const QRectF &candidate) {
  return candidate.center().y() > origin.center().y() + EDGE_EPSILON &&
         (candidate.bottom() > origin.bottom() + EDGE_EPSILON || candidate.top() > origin.top() + EDGE_EPSILON);
}

} // namespace

int SpatialResolver::pick(const QRectF &origin, const Direction direction, const std::vector<QRectF> &candidates) {
  const auto from = orient(origin, direction);

  auto bestIndex = NONE;
  auto bestScore = 0.0;

  for (size_t i = 0; i < candidates.size(); ++i) {
    const auto to = orient(candidates[i], direction);

    if (to.isEmpty() || from.isEmpty()) {
      continue;
    }

    const auto shared = std::max(0.0, std::min(from.right(), to.right()) - std::max(from.left(), to.left()));
    const auto narrower = std::max(1.0, std::min(from.width(), to.width()));
    const auto alignment = std::min(1.0, shared / narrower);

    if (alignment <= 0.0 || !isAhead(from, to)) {
      continue;
    }

    const auto gap = std::max(0.0, to.top() - from.bottom());
    const auto score = gap + MISALIGN_WEIGHT * (1.0 - alignment) * narrower +
                       CENTER_WEIGHT * std::abs(to.center().x() - from.center().x());

    // Walk order breaks an exact tie, so the same layout always resolves the same way
    if (bestIndex == NONE || score < bestScore) {
      bestIndex = static_cast<int>(i);
      bestScore = score;
    }
  }

  return bestIndex;
}

int SpatialResolver::pickNearestLane(const QRectF &origin, const Direction direction,
                                     const std::vector<QRectF> &candidates) {
  const auto from = orient(origin, direction);

  auto bestIndex = NONE;
  auto bestLane = 0.0;
  auto bestGap = 0.0;

  for (size_t i = 0; i < candidates.size(); ++i) {
    const auto to = orient(candidates[i], direction);

    if (to.isEmpty() || from.isEmpty() || !isAhead(from, to)) {
      continue;
    }

    const auto lane = std::max(0.0, std::max(from.left(), to.left()) - std::min(from.right(), to.right()));
    const auto gap = std::max(0.0, to.top() - from.bottom());

    // Lane first, so the row nearest the press wins outright; distance only separates that row
    if (bestIndex == NONE || lane < bestLane - EDGE_EPSILON || (lane < bestLane + EDGE_EPSILON && gap < bestGap)) {
      bestIndex = static_cast<int>(i);
      bestLane = lane;
      bestGap = gap;
    }
  }

  return bestIndex;
}

} // namespace firelight::gui
