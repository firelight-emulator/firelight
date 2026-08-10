#include <QKeyEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <gtest/gtest.h>

// Probe for a global key handler installed at the end of the delivery chain
// rather than the start. A key goes to the focused item and then bubbles up
// through its parents, so a filter on the window's root item should see only
// what everything below it declined — which is what would let one handler cover
// both the content tree and the popup overlay without preempting Qt's own
// handling of Space, Escape and menu arrows.
namespace firelight::gui {

namespace {
// Records that it saw a key press, and optionally swallows it
class KeySpy : public QObject {
public:
  KeySpy(QString name, QStringList *log, const bool consume = false)
      : m_name(std::move(name)), m_log(log), m_consume(consume) {}

protected:
  bool eventFilter(QObject *watched, QEvent *event) override {
    if (event->type() == QEvent::KeyPress) {
      m_log->append(m_name);

      if (m_consume) {
        event->accept();
        return true;
      }
    }

    return QObject::eventFilter(watched, event);
  }

private:
  QString m_name;
  QStringList *m_log;
  bool m_consume;
};

QQuickItem *item(QQuickItem *parent, const Qt::FocusPolicy policy = Qt::NoFocus) {
  auto *created = new QQuickItem(parent);
  created->setSize(QSizeF(40.0, 40.0));
  created->setFocusPolicy(policy);

  return created;
}

void press(QQuickWindow *window, const int key) {
  QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
  QCoreApplication::sendEvent(window, &event);
}
} // namespace

TEST(RootKeyFilterTest, FocusReachesAnItemInAWindowThatWasNeverShown) {
  QQuickWindow window;
  auto *leaf = item(window.contentItem(), Qt::StrongFocus);
  leaf->forceActiveFocus();

  EXPECT_EQ(window.activeFocusItem(), leaf);
}

TEST(RootKeyFilterTest, AKeyTheFocusedItemDeclinedReachesTheRootItem) {
  QQuickWindow window;
  auto *branch = item(window.contentItem());
  auto *leaf = item(branch, Qt::StrongFocus);
  leaf->forceActiveFocus();

  QStringList log;
  KeySpy rootSpy("root", &log);
  KeySpy branchSpy("branch", &log);
  KeySpy leafSpy("leaf", &log);

  window.contentItem()->installEventFilter(&rootSpy);
  branch->installEventFilter(&branchSpy);
  leaf->installEventFilter(&leafSpy);

  press(&window, Qt::Key_Down);

  // The root is reached last, which is what makes it a fallback rather than a
  // preemption
  EXPECT_EQ(log, QStringList({"leaf", "branch", "root"}));
}

// The popup case in miniature: content under a sibling branch still converges on
// the same root item, so one filter there covers both trees
TEST(RootKeyFilterTest, AKeyFromASeparateBranchAlsoReachesTheRootItem) {
  QQuickWindow window;
  item(window.contentItem());
  auto *overlay = item(window.contentItem());
  auto *inside = item(overlay, Qt::StrongFocus);
  inside->forceActiveFocus();

  QStringList log;
  KeySpy rootSpy("root", &log);
  window.contentItem()->installEventFilter(&rootSpy);

  press(&window, Qt::Key_Down);

  EXPECT_EQ(log, QStringList({"root"}));
}

// Anything that handles the key first keeps it, so Space on a button and Escape
// in a popup would never reach the handler
TEST(RootKeyFilterTest, SomethingBelowThatTakesTheKeyKeepsItFromTheRootItem) {
  QQuickWindow window;
  auto *branch = item(window.contentItem());
  auto *leaf = item(branch, Qt::StrongFocus);
  leaf->forceActiveFocus();

  QStringList log;
  KeySpy rootSpy("root", &log);
  KeySpy branchSpy("branch", &log, true);

  window.contentItem()->installEventFilter(&rootSpy);
  branch->installEventFilter(&branchSpy);

  press(&window, Qt::Key_Down);

  EXPECT_EQ(log, QStringList({"branch"}));
}

} // namespace firelight::gui
