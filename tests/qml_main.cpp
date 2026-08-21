// src_qmltest_qquicktest.cpp
#include "gui/focus/focus_navigator.hpp"
#include "gui/focus_info.hpp"

#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QtQuickTest>

class Setup : public QObject {
  Q_OBJECT

public:
  Setup() {}

public slots:

  void applicationAvailable() {
    // Initialization that only requires the QApplication object to be available
    QQuickStyle::setStyle("Basic");
  }

  void qmlEngineAvailable(QQmlEngine *engine) {
    // Initialization requiring the QQmlEngine to be constructed
    // engine->rootContext()->setContextProperty("myContextProperty", QVariant(true));

    // The same registrations main.cpp makes, so a test can exercise the focus
    // metadata the way the app sees it
    qmlRegisterType<firelight::gui::FocusAction>("Firelight", 1, 0, "FLAction");
    qmlRegisterType<firelight::gui::FocusInfo>("Firelight", 1, 0, "FLFocus");
    qmlRegisterSingletonInstance("Firelight", 1, 0, "FocusNavigator", new firelight::gui::FocusNavigator(engine));
  }

  void cleanupTestCase() {
    // Implement custom resource cleanup
  }
};

QUICK_TEST_MAIN_WITH_SETUP(Firelight, Setup)

#include "qml_main.moc"
