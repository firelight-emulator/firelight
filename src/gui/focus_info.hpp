// TODO: NEEDS REVIEW
#pragma once

#include "focus_action.hpp"

#include <QColor>
#include <QList>
#include <QObject>
#include <QPair>
#include <QQmlListProperty>
#include <QQuickItem>
#include <QtNumeric>
#include <qqmlintegration.h>

namespace firelight::gui {

/**
 * Focus metadata attached to anything the global cursor can land on.
 *
 * It is an attached property rather than a base class because the things that need it share no
 * common ancestor below QObject: controls, plain items, focus scopes and popups all carry it the
 * same way.
 */
class FocusInfo : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool showCursor READ showsCursor WRITE setShowCursor NOTIFY showCursorChanged)
  Q_PROPERTY(QQuickItem *proxy READ getProxy WRITE setProxy NOTIFY proxyChanged)
  Q_PROPERTY(qreal spacing READ getSpacing WRITE setSpacing NOTIFY spacingChanged)
  Q_PROPERTY(qreal radius READ getRadius WRITE setRadius NOTIFY radiusChanged)
  Q_PROPERTY(qreal topLeftRadius READ getTopLeftRadius WRITE setTopLeftRadius NOTIFY topLeftRadiusChanged)
  Q_PROPERTY(qreal topRightRadius READ getTopRightRadius WRITE setTopRightRadius NOTIFY topRightRadiusChanged)
  Q_PROPERTY(qreal bottomLeftRadius READ getBottomLeftRadius WRITE setBottomLeftRadius NOTIFY bottomLeftRadiusChanged)
  Q_PROPERTY(
      qreal bottomRightRadius READ getBottomRightRadius WRITE setBottomRightRadius NOTIFY bottomRightRadiusChanged)
  Q_PROPERTY(QColor fill READ getFill WRITE setFill NOTIFY fillChanged)
  Q_PROPERTY(QObject *focusSound READ getFocusSound WRITE setFocusSound NOTIFY focusSoundChanged)
  Q_PROPERTY(QQmlListProperty<firelight::gui::FocusAction> actions READ getActions)
  Q_PROPERTY(Mode mode READ getMode WRITE setMode NOTIFY modeChanged)
  Q_PROPERTY(bool barrier READ isBarrier WRITE setBarrier NOTIFY barrierChanged)
  Q_PROPERTY(bool container READ isContainer WRITE setContainer NOTIFY containerChanged)
  Q_PROPERTY(Edges holdEdges READ getHoldEdges WRITE setHoldEdges NOTIFY holdEdgesChanged)
  QML_ELEMENT
  QML_ATTACHED(FocusInfo)

public:
  /**
   * How directional navigation treats the attached item
   */
  enum Mode {
    /** An ordinary focus stop */
    Normal,
    /** Not a target, and nothing inside it is either */
    Skip,
    /** Not a target itself; offer what is inside it instead */
    Group,
    /** One target; do not look inside it */
    Stop
  };
  Q_ENUM(Mode)

  /**
   * Directions in which a held press will not leave the attached item
   */
  enum Edge {
    NoEdges = 0x0,
    Up = 0x1,
    Down = 0x2,
    Left = 0x4,
    Right = 0x8,
    Vertical = Up | Down,
    Horizontal = Left | Right,
    AllEdges = Vertical | Horizontal
  };
  Q_DECLARE_FLAGS(Edges, Edge)
  Q_FLAG(Edges)

  /**
   * Creates the metadata for one object, owned by that object
   */
  static FocusInfo *qmlAttachedProperties(QObject *object) { return new FocusInfo(object); }

  explicit FocusInfo(QObject *parent = nullptr) : QObject(parent) {}

  /**
   * The metadata an object already carries, without creating any. Null means the object never
   * declared any, which is how a half-migrated tree stays distinguishable
   */
  Q_INVOKABLE static FocusInfo *find(QObject *object) {
    if (object == nullptr) {
      return nullptr;
    }

    return qobject_cast<FocusInfo *>(qmlAttachedPropertiesObject<FocusInfo>(object, false));
  }

  /**
   * Every action a press on item could reach: its own first, then each ancestor's, stopping at a
   * barrier because navigation cannot leave one.
   *
   * A key appears once per set of modifiers, so a key and the same key under Shift are both listed.
   * The nearest declaration wins, which is the one a press would actually run, and a disabled action
   * is left out because it would answer nothing
   */
  Q_INVOKABLE static QList<FocusAction *> collectActions(QQuickItem *item) {
    QList<FocusAction *> actions;
    QList<QPair<int, int>> claimedKeys;

    for (auto *current = item; current != nullptr; current = current->parentItem()) {
      auto *info = find(current);

      if (info == nullptr) {
        continue;
      }

      for (auto *action : info->m_actions) {
        if (action == nullptr || !action->isEnabled()) {
          continue;
        }

        const auto keys = action->getKeys();
        const auto modifiers = action->getModifiers();
        auto isClaimed = false;

        for (const auto key : keys) {
          if (claimedKeys.contains({key, modifiers})) {
            isClaimed = true;
            break;
          }
        }

        if (isClaimed) {
          continue;
        }

        for (const auto key : keys) {
          claimedKeys.append({key, modifiers});
        }

        actions.append(action);
      }

      if (info->isBarrier()) {
        break;
      }
    }

    return actions;
  }

  /**
   * @return Whether the global cursor draws on the attached item
   */
  [[nodiscard]] bool showsCursor() const { return m_showCursor; }

  /**
   * @return The item whose shape the cursor should take instead, or null to use the attached item
   */
  [[nodiscard]] QQuickItem *getProxy() const { return m_proxy; }

  /**
   * @return The gap between the item and the cursor, or NaN when unset
   */
  [[nodiscard]] qreal getSpacing() const { return m_spacing; }

  /**
   * @return The corner radius the cursor should take, or NaN when unset
   */
  [[nodiscard]] qreal getRadius() const { return m_radius; }

  /**
   * @return The radius that corner of the cursor should take, or NaN to follow `radius`
   */
  [[nodiscard]] qreal getTopLeftRadius() const { return m_topLeftRadius; }

  /**
   * @return The radius that corner of the cursor should take, or NaN to follow `radius`
   */
  [[nodiscard]] qreal getTopRightRadius() const { return m_topRightRadius; }

  /**
   * @return The radius that corner of the cursor should take, or NaN to follow `radius`
   */
  [[nodiscard]] qreal getBottomLeftRadius() const { return m_bottomLeftRadius; }

  /**
   * @return The radius that corner of the cursor should take, or NaN to follow `radius`
   */
  [[nodiscard]] qreal getBottomRightRadius() const { return m_bottomRightRadius; }

  /**
   * @return What to paint between the item and the cursor, or transparent for nothing
   */
  [[nodiscard]] QColor getFill() const { return m_fill; }

  /**
   * @return The sound to play when the cursor arrives, or null for none
   */
  [[nodiscard]] QObject *getFocusSound() const { return m_focusSound; }

  [[nodiscard]] Mode getMode() const { return m_mode; }

  /**
   * @return Whether navigation may not cross out of the attached item
   */
  [[nodiscard]] bool isBarrier() const { return m_barrier; }

  /**
   * @return Whether the attached item answers for its children when a press asks what it can reach
   */
  [[nodiscard]] bool isContainer() const { return m_container; }

  [[nodiscard]] Edges getHoldEdges() const { return m_holdEdges; }

  /**
   * The actions the attached item offers, in the order they were declared
   */
  [[nodiscard]] QQmlListProperty<FocusAction> getActions() {
    return {
        this, this, &FocusInfo::appendAction, &FocusInfo::countActions, &FocusInfo::actionAt, &FocusInfo::clearActions};
  }

  /**
   * @return The first enabled action answering to key pressed with modifiers held, or null when none does
   */
  Q_INVOKABLE FocusAction *getActionFor(const int key, const int modifiers = Qt::NoModifier) const {
    for (auto *action : m_actions) {
      if (action != nullptr && action->handles(key, modifiers)) {
        return action;
      }
    }

    return nullptr;
  }

  /**
   * @return How many actions the attached item offers
   */
  [[nodiscard]] Q_INVOKABLE int getActionCount() const { return static_cast<int>(m_actions.size()); }

  /**
   * @return The action at index, or null when out of range
   */
  [[nodiscard]] Q_INVOKABLE FocusAction *getAction(const int index) const {
    if (index < 0 || index >= m_actions.size()) {
      return nullptr;
    }

    return m_actions.at(index);
  }

  void setShowCursor(const bool showCursor) {
    if (m_showCursor != showCursor) {
      m_showCursor = showCursor;
      emit showCursorChanged();
    }
  }

  void setProxy(QQuickItem *proxy) {
    if (m_proxy != proxy) {
      m_proxy = proxy;
      emit proxyChanged();
    }
  }

  void setSpacing(const qreal spacing) {
    if (!qFuzzyCompare(m_spacing, spacing)) {
      m_spacing = spacing;
      emit spacingChanged();
    }
  }

  void setRadius(const qreal radius) {
    if (!qFuzzyCompare(m_radius, radius)) {
      m_radius = radius;
      emit radiusChanged();
    }
  }

  void setTopLeftRadius(const qreal radius) {
    if (!qFuzzyCompare(m_topLeftRadius, radius)) {
      m_topLeftRadius = radius;
      emit topLeftRadiusChanged();
    }
  }

  void setTopRightRadius(const qreal radius) {
    if (!qFuzzyCompare(m_topRightRadius, radius)) {
      m_topRightRadius = radius;
      emit topRightRadiusChanged();
    }
  }

  void setBottomLeftRadius(const qreal radius) {
    if (!qFuzzyCompare(m_bottomLeftRadius, radius)) {
      m_bottomLeftRadius = radius;
      emit bottomLeftRadiusChanged();
    }
  }

  void setBottomRightRadius(const qreal radius) {
    if (!qFuzzyCompare(m_bottomRightRadius, radius)) {
      m_bottomRightRadius = radius;
      emit bottomRightRadiusChanged();
    }
  }

  void setFill(const QColor &fill) {
    if (m_fill != fill) {
      m_fill = fill;
      emit fillChanged();
    }
  }

  void setFocusSound(QObject *focusSound) {
    if (m_focusSound != focusSound) {
      m_focusSound = focusSound;
      emit focusSoundChanged();
    }
  }

  void setMode(const Mode mode) {
    if (m_mode != mode) {
      m_mode = mode;
      emit modeChanged();
    }
  }

  void setBarrier(const bool barrier) {
    if (m_barrier != barrier) {
      m_barrier = barrier;
      emit barrierChanged();
    }
  }

  void setContainer(const bool container) {
    if (m_container != container) {
      m_container = container;
      emit containerChanged();
    }
  }

  void setHoldEdges(const Edges holdEdges) {
    if (m_holdEdges != holdEdges) {
      m_holdEdges = holdEdges;
      emit holdEdgesChanged();
    }
  }

signals:
  void showCursorChanged();

  void proxyChanged();

  void spacingChanged();

  void radiusChanged();

  void topLeftRadiusChanged();

  void topRightRadiusChanged();

  void bottomLeftRadiusChanged();

  void bottomRightRadiusChanged();

  void fillChanged();

  void focusSoundChanged();

  void modeChanged();

  void barrierChanged();

  void containerChanged();

  void holdEdgesChanged();

private:
  static void appendAction(QQmlListProperty<FocusAction> *list, FocusAction *action) {
    static_cast<FocusInfo *>(list->data)->m_actions.append(action);
  }

  static qsizetype countActions(QQmlListProperty<FocusAction> *list) {
    return static_cast<FocusInfo *>(list->data)->m_actions.size();
  }

  static FocusAction *actionAt(QQmlListProperty<FocusAction> *list, const qsizetype index) {
    return static_cast<FocusInfo *>(list->data)->m_actions.at(index);
  }

  static void clearActions(QQmlListProperty<FocusAction> *list) {
    static_cast<FocusInfo *>(list->data)->m_actions.clear();
  }

  bool m_showCursor = false;
  QQuickItem *m_proxy = nullptr;
  qreal m_spacing = qQNaN();
  qreal m_radius = qQNaN();
  qreal m_topLeftRadius = qQNaN();
  qreal m_topRightRadius = qQNaN();
  qreal m_bottomLeftRadius = qQNaN();
  qreal m_bottomRightRadius = qQNaN();
  QColor m_fill = Qt::transparent;
  QObject *m_focusSound = nullptr;
  QList<FocusAction *> m_actions;
  Mode m_mode = Normal;
  bool m_barrier = false;
  bool m_container = false;
  Edges m_holdEdges = NoEdges;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FocusInfo::Edges)

} // namespace firelight::gui
