#pragma once

#include <QRectF>
#include <vector>

namespace firelight::gui {

/**
 * Which way the cursor is being asked to move
 */
enum class Direction { Up, Down, Left, Right };

/**
 * Chooses which of several candidates a directional press should move to, from geometry alone.
 *
 * This is what lets the cursor cross between containers without anything being wired up by hand: a
 * press a container refuses is answered here, against whatever else is on screen.
 */
class SpatialResolver {
public:
  /** Returned when nothing lies in the given direction */
  static constexpr int NONE = -1;

  /**
   * Picks the candidate to move to.
   *
   * Only what shares the origin's lane is reachable: a vertical press needs horizontal overlap and
   * a horizontal press needs vertical overlap, so a press never veers off across the screen to
   * something it does not line up with. Among those, nearer wins, then better aligned, then
   * whichever came first.
   *
   * @param origin Where the cursor is now, in scene coordinates
   * @param direction Which way it is moving
   * @param candidates Everything it could move to, in tree-walk order, in scene coordinates
   * @return The index of the winner, or NONE
   */
  static int pick(const QRectF &origin, Direction direction, const std::vector<QRectF> &candidates);

  /**
   * Picks the candidate to enter a scrolling view at, where sharing a lane cannot be asked for.
   *
   * A view that has been scrolled shows whatever rows happen to be on screen, so the one nearest a
   * press may line up with nothing. Here the nearest lane wins instead of a shared one, then the
   * nearest along the press. Only for entering something already established as lying that way —
   * pick() is what decides that.
   *
   * @param origin Where the cursor is now, in scene coordinates
   * @param direction Which way it is moving
   * @param candidates What the view is offering, in tree-walk order, in scene coordinates
   * @return The index of the winner, or NONE
   */
  static int pickNearestLane(const QRectF &origin, Direction direction, const std::vector<QRectF> &candidates);
};

} // namespace firelight::gui
