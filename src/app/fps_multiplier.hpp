//
// Created by nicho on 12/27/2023.
//

#ifndef FPS_MULTIPLIER_H
#define FPS_MULTIPLIER_H
#include <QObject>
#include <iostream>
#include <qobjectdefs.h>
#include <qstring.h>
#include <qtmetamacros.h>

class FpsMultiplier : public QObject {
  Q_OBJECT
  Q_PROPERTY(double sliderValue READ sliderValue WRITE setSliderValue NOTIFY
                 sliderValueChanged)

public:
  inline double sliderValue() const { return m_sliderValue; }

signals:
  void sliderValueChanged(double newValue);

public slots:
  void stop();
  void start();
  void setSliderValue(double);

private:
  double m_sliderValue = 1;
};

#endif // FPS_MULTIPLIER_H
