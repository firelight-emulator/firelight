// TODO: NEEDS REVIEW
#pragma once

#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSet>
#include <functional>

namespace firelight::gui {

class FocusInfo;

// TODO
/**
 * Every FLFocus declaration that marks its item as a surface, kept where the arbiter can find them
 * without each having to know it exists
 */
class FocusSurfaces : public QObject {
  Q_OBJECT

public:
  static FocusSurfaces &instance();

  void add(FocusInfo *info);

  void remove(FocusInfo *info);

  [[nodiscard]] const QList<FocusInfo *> &getAll() const { return m_surfaces; }

signals:
  void changed();

private:
  QList<FocusInfo *> m_surfaces;
};

// TODO
/**
 * Keeps a window's focus inside the surface that owns it.
 *
 * Surfaces declare when they are active and where focus enters them. Per window the active surface
 * on the highest layer owns focus, the most recently activated one on a tie. Once per event-loop
 * turn, never inside one of Qt's own focus operations, focus that is null, hidden, disabled, outside
 * the owner, or inside one of the owner's exclusive regions without being inside its entry, is put
 * on the entry
 */
class FocusArbiter : public QObject {
  Q_OBJECT

public:
  /** Puts focus on an item */
  using Landing = std::function<void(QQuickItem *)>;

  explicit FocusArbiter(QObject *parent = nullptr);

  ~FocusArbiter() override;

  /**
   * How focus is put on an entry
   */
  void setLander(Landing lander);

  /**
   * How a cursor that has come to rest on something it cannot sit on is settled
   */
  void setParker(Landing parker);

  /**
   * @return The declaration owning window's focus, or null when no active surface sits in it
   */
  [[nodiscard]] FocusInfo *getOwnerInfo(QQuickWindow *window) const;

  /**
   * @return The item owning window's focus, or null
   */
  [[nodiscard]] QQuickItem *getOwner(QQuickWindow *window) const;

  /**
   * Asks for one pass over window once the current event has finished. Repeated asks before then
   * collapse into that one pass
   */
  void request(QQuickWindow *window);

  /**
   * Records that surface's entry moved, so the next pass over its window lands on that entry if the
   * surface owns focus and focus is not already inside it, and asks for that pass
   */
  void requestForEntry(FocusInfo *surface);

  /**
   * The pass itself, reachable for tests
   */
  void enforceNow(QQuickWindow *window);

  /**
   * @return Whether focus on item satisfies owner without a landing: inside the entry, or a real
   *         target inside the owner and outside its exclusive regions
   */
  static bool isAllowed(const FocusInfo *owner, QQuickItem *item);

  /**
   * @return The item a surface's focus lands on: its entry, or the surface item itself
   */
  static QQuickItem *getEntryOf(const FocusInfo *surface);

  /**
   * @return The item a declaration is attached to, or null when it is attached to something else
   */
  static QQuickItem *getItemOf(const FocusInfo *surface);

signals:
  void ownerChanged(QQuickWindow *window);

private:
  // TODO
  /**
   * What one window's pass watches between passes, so a change on any of them asks for another
   */
  struct Watch {
    QPointer<QQuickItem> lastOwner;
    QPointer<QQuickItem> owner;
    QPointer<QQuickItem> entry;
    QPointer<QQuickItem> focused;
    QList<QMetaObject::Connection> connections;
  };

  /**
   * Starts listening to every registered surface not yet listened to
   */
  void adoptSurfaces();

  /**
   * Listens to one surface's declarations
   */
  void observeSurface(FocusInfo *info);

  /**
   * Re-points what one window watches, and listens to the visibility of each
   */
  void follow(QQuickWindow *window, Watch &watch, QQuickItem *owner, QQuickItem *entry, QQuickItem *focused);

  /**
   * @return The window a surface sits in, or null
   */
  static QQuickWindow *getWindowOf(const FocusInfo *surface);

  Landing m_lander;
  Landing m_parker;
  QHash<FocusInfo *, quint64> m_activation;
  QSet<FocusInfo *> m_observed;
  quint64 m_activationCounter = 0;
  QSet<QQuickWindow *> m_pending;
  QHash<QQuickWindow *, QSet<FocusInfo *>> m_entryMoved;
  QHash<QQuickWindow *, Watch> m_watches;
  bool m_isEnforcing = false;
};

} // namespace firelight::gui
