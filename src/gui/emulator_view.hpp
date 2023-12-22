//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATOR_VIEW_HPP
#define FIRELIGHT_EMULATOR_VIEW_HPP

#include <QQuickFramebufferObject>
#include <QQuickItem>
#include <QQuickWindow>
class EmulatorView : public QQuickFramebufferObject {
  Q_OBJECT

public:
  explicit EmulatorView(QQuickItem *parent = nullptr);
  [[nodiscard]] Renderer *createRenderer() const override;

  Q_INVOKABLE void init();
private slots:
  void updateThing();
  // private slots:
  //   void handleWindowChanged(QQuickWindow *win);

private:
  Renderer *myRenderer;
};

#endif // FIRELIGHT_EMULATOR_VIEW_HPP
