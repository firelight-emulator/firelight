#pragma once

#include "shader_preset.hpp"

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QString>
#include <QVariantList>
#include <optional>
#include <vector>

namespace firelight::graphics {

// Discovers shader presets (.flsp) from the shipped system folder and the user's
// writable shaders folder, exposing them to QML as a picker model plus lookups
// for parameter metadata and full-preset resolution (used by the renderer/
// preview). Refreshes live via a QFileSystemWatcher.
class ShaderLibrary : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    IdRole = Qt::UserRole + 1,
    NameRole,
    PreviewUrlRole,
    BuiltInRole,
  };

  ShaderLibrary(QString builtinDir, QString userDir, QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  // The tunable parameters of preset `id` as a list of maps
  // {key,label,min,max,step,default}. Empty when the preset is unknown or has
  // no parameters. Used by the picker to build sliders.
  Q_INVOKABLE [[nodiscard]] QVariantList parametersFor(const QString &id) const;

  // Resolves a preset id to its parsed form (passes + parameters), or nullopt.
  // Used by the renderer and the live preview.
  [[nodiscard]] std::optional<ShaderPreset>
  presetById(const QString &id) const;

public slots:
  void refresh();

signals:
  void changed();

private:
  QString m_builtinDir;
  QString m_userDir;
  QFileSystemWatcher m_watcher;

  struct Entry {
    ShaderPreset preset;
    QString previewPath; // optional thumbnail
  };
  std::vector<Entry> m_entries;

  void scanDirectory(const QString &dir, bool builtIn,
                     std::vector<Entry> &out) const;
};

} // namespace firelight::graphics
