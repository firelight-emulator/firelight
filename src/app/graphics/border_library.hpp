#pragma once

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QRectF>
#include <QString>
#include <QVariantMap>
#include <vector>

namespace firelight::graphics {

// A border/bezel the user can place around the gameplay area.
//   - type "frame": a decorative image drawn BEHIND the game; the game keeps its
//     normal picture-mode sizing and is centered on top.
//   - type "bezel": a full-window overlay image drawn IN FRONT with a
//     transparent screen cutout; `screenRect` (normalized 0..1 of the window)
//     positions/sizes the game into the cutout.
struct BorderInfo {
  QString id;          // stable id (folder name, or file stem for a bare image)
  QString name;        // display name
  QString type;        // "frame" | "bezel"
  QString imagePath;   // absolute path to the image on disk
  QRectF screenRect;   // normalized cutout for bezels (unit rect for frames)
  double opacity = 1.0;
  bool builtIn = false;
};

// Discovers borders/bezels from the shipped system folder (read-only) and the
// user's writable borders folder, and exposes them to QML as a list model for
// the picker plus a lookup for the in-game page. Mirrors the app's other
// filesystem-backed models; a QFileSystemWatcher refreshes the list live when a
// user drops in a new border.
class BorderLibrary : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    TypeRole,
    ImageUrlRole,   // "file://" url, ready for a QML Image source
    ScreenXRole,
    ScreenYRole,
    ScreenWidthRole,
    ScreenHeightRole,
    OpacityRole,
    BuiltInRole,
  };

  // `builtinDir` is the shipped read-only tree (…/system/borders); `userDir` is
  // the writable folder users drop their own borders into.
  BorderLibrary(QString builtinDir, QString userDir, QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  // Resolves a border id to a map with the same keys as the model roles, or an
  // empty map when `id` is empty/unknown. Used by the in-game page to render the
  // selected border.
  Q_INVOKABLE [[nodiscard]] QVariantMap borderById(const QString &id) const;

public slots:
  // Rescans both directories and resets the model.
  void refresh();

signals:
  // Emitted after a rescan (list may have changed).
  void changed();

private:
  QString m_builtinDir;
  QString m_userDir;
  QFileSystemWatcher m_watcher;
  std::vector<BorderInfo> m_borders;

  void scanDirectory(const QString &dir, bool builtIn,
                     std::vector<BorderInfo> &out) const;
  static QVariantMap toMap(const BorderInfo &b);
};

} // namespace firelight::graphics
