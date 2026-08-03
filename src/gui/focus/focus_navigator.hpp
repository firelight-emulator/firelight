#pragma once

#include "candidate_collector.hpp"
#include "spatial_resolver.hpp"

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <utility>

namespace firelight::gui {

/**
 * Thins out a held direction to a steady cadence.
 *
 * Gamepad autorepeat arrives far faster than the cursor's travel animation settles, so without this
 * a held direction outruns what is on screen
 */
class RepeatGovernor {
public:
  /** How long a held direction waits between moves */
  static constexpr qint64 INTERVAL_MS = 60;

  /**
   * Whether a press should be acted on, recording it when so.
   *
   * A fresh press and a change of direction always pass, so only a sustained hold is thinned
   */
  bool allows(Direction direction, bool isAutoRepeat, qint64 nowMs);

  /**
   * Forgets the last press, so the next one counts as fresh
   */
  void reset();

private:
  Direction m_lastDirection = Direction::Down;
  qint64 m_lastMoveMs = 0;
  bool m_hasMoved = false;
};

/**
 * Answers directional presses that no container claimed, by looking at what is on screen.
 *
 * This is the layer that lets the cursor cross from one container to another without either of them
 * knowing the other exists.
 */
class FocusNavigator : public QObject {
  Q_OBJECT

public:
  /**
   * What became of a directional press
   */
  enum MoveResult {
    /** Nothing lies that way, so the caller should show the press going nowhere */
    NoTarget,
    /** The cursor moved, or deliberately held still to keep a held direction on cadence */
    Moved,
    /** Reserved for a move that scrolls before it lands */
    Deferred
  };
  Q_ENUM(MoveResult)

  explicit FocusNavigator(QObject *parent = nullptr) : QObject(parent) {}

  /**
   * Moves the cursor from wherever it is in the direction key asks for
   *
   * @param origin The item holding key focus
   * @param key A Qt::Key; anything that is not a direction does nothing
   * @param isAutoRepeat Whether this press comes from a held direction
   * @return What became of the press, as a MoveResult
   */
  Q_INVOKABLE int move(QQuickItem *origin, int key, bool isAutoRepeat);

  /**
   * @return The direction key asks for, or false in the second element when it asks for none
   */
  static std::pair<Direction, bool> directionFor(int key);

  /**
   * Whether a held direction is not allowed to leave the container it started in.
   *
   * Containers declare the edges they hold, so a held direction stops at the end of a list instead
   * of shooting out the far side, while a fresh press crosses freely
   */
  static bool isHeldBack(QQuickItem *from, QQuickItem *to, Direction direction, bool isAutoRepeat);

  /**
   * Forgets the way back, so the next press is answered by geometry alone
   */
  void forget();

private:
  /**
   * The candidate to enter a container at, when nothing itself shared the press's lane.
   *
   * A container answers for what it holds: a column running the height of a page is reachable from
   * beside any part of it, and a scrolled view shows whatever rows happen to be on screen. The
   * container still has to lie the way the press points, so a rail is not below the grid beside it
   *
   * @return An index into candidates, or SpatialResolver::NONE
   */
  static int enterContainer(const QRectF &origin, Direction direction, QQuickItem *leaving,
                            const std::vector<FocusCandidate> &candidates);

  /**
   * Where the reverse of the last move would return to, or null when this press is not that
   * reverse.
   *
   * Geometry alone cannot answer "back where I came from": a small button entering a tall tile
   * leaves from the tile's middle, and the button nearest that middle is not the one it started on
   */
  [[nodiscard]] QQuickItem *wayBack(QQuickItem *origin, Direction direction, bool isAutoRepeat) const;

  /**
   * Puts the cursor on an item and records the step, so the opposite press can undo it
   */
  void land(QQuickItem *from, QQuickItem *to, Direction direction);

  /**
   * Watches a window so that anything else moving the cursor — a container, a tap — drops the way
   * back. It is only good while the cursor has not moved on
   */
  void watch(QQuickItem *item);

  RepeatGovernor m_governor;
  QPointer<QQuickItem> m_steppedFrom;
  QPointer<QQuickItem> m_steppedTo;
  Direction m_steppedDirection = Direction::Down;
  QPointer<QQuickWindow> m_window;
  bool m_landing = false;
};

} // namespace firelight::gui
