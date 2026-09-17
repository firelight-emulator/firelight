// TODO: NEEDS REVIEW
#pragma once

#include <QObject>
#include <cstdint>
#include <vector>

namespace firelight::gui {

/** One value at one time */
struct SeriesPoint {
  int64_t atNs = 0;
  double value = 0.0;
};

/**
 * Something a series chart can draw. Whoever owns the numbers implements it; the chart only asks
 * for the points in a window and the time the window ends at
 */
class SeriesSource : public QObject {
  Q_OBJECT

public:
  explicit SeriesSource(QObject *parent = nullptr) : QObject(parent) {}

  /**
   * Appends every point at or after sinceNs to out, oldest first
   */
  virtual void copyPoints(int64_t sinceNs, std::vector<SeriesPoint> &out) const = 0;

  /**
   * @return The current time on the clock the points are stamped with
   */
  [[nodiscard]] virtual int64_t getNowNs() const = 0;

signals:
  void changed();
};

} // namespace firelight::gui
