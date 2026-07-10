#include "shader_library.hpp"

#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::graphics {

namespace {
const QStringList kPresetFilters = {"*.flsp"};
const QStringList kPreviewFilters = {"*.png", "*.jpg", "*.jpeg", "*.webp"};

// Finds a thumbnail next to a preset: "<stem>.png" or, in a folder, "preview.*".
QString findPreview(const QDir &dir, const QString &stem) {
  for (const auto &ext : {"png", "jpg", "jpeg", "webp"}) {
    const auto candidate = dir.filePath(stem + "." + ext);
    if (QFileInfo::exists(candidate)) {
      return candidate;
    }
    const auto preview = dir.filePath(QStringLiteral("preview.") + ext);
    if (QFileInfo::exists(preview)) {
      return preview;
    }
  }
  return {};
}
} // namespace

ShaderLibrary::ShaderLibrary(QString builtinDir, QString userDir,
                             QObject *parent)
    : QAbstractListModel(parent), m_builtinDir(std::move(builtinDir)),
      m_userDir(std::move(userDir)) {
  for (const auto &dir : {m_builtinDir, m_userDir}) {
    if (!dir.isEmpty() && QFileInfo::exists(dir)) {
      m_watcher.addPath(dir);
    }
  }
  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
          &ShaderLibrary::refresh);
  refresh();
}

int ShaderLibrary::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_entries.size());
}

QVariant ShaderLibrary::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(m_entries.size())) {
    return {};
  }
  const auto &e = m_entries[index.row()];
  switch (role) {
  case IdRole:
    return e.preset.id;
  case NameRole:
    return e.preset.name;
  case PreviewUrlRole:
    return e.previewPath.isEmpty()
               ? QString()
               : QUrl::fromLocalFile(e.previewPath).toString();
  case BuiltInRole:
    return e.preset.builtIn;
  default:
    return {};
  }
}

QHash<int, QByteArray> ShaderLibrary::roleNames() const {
  return {
      {IdRole, "shaderId"},
      {NameRole, "name"},
      {PreviewUrlRole, "previewUrl"},
      {BuiltInRole, "builtIn"},
  };
}

QVariantList ShaderLibrary::parametersFor(const QString &id) const {
  QVariantList result;
  const auto preset = presetById(id);
  if (!preset) {
    return result;
  }
  for (const auto &p : preset->parameters) {
    result.append(QVariantMap{
        {"key", p.key},
        {"label", p.label},
        {"min", p.minValue},
        {"max", p.maxValue},
        {"step", p.step},
        {"default", p.defaultValue},
    });
  }
  return result;
}

std::optional<ShaderPreset> ShaderLibrary::presetById(const QString &id) const {
  const auto it = std::find_if(
      m_entries.begin(), m_entries.end(),
      [&id](const Entry &e) { return e.preset.id == id; });
  if (it == m_entries.end()) {
    return std::nullopt;
  }
  return it->preset;
}

void ShaderLibrary::scanDirectory(const QString &dir, const bool builtIn,
                                  std::vector<Entry> &out) const {
  if (dir.isEmpty() || !QFileInfo::exists(dir)) {
    return;
  }
  const QDir root(dir);

  const auto loadInto = [&](const QString &presetPath, const QDir &presetDir) {
    QString error;
    auto preset = shader_preset::loadFromFile(presetPath, &error);
    if (!preset) {
      spdlog::warn("Skipping shader preset {}: {}", presetPath.toStdString(),
                   error.toStdString());
      return;
    }
    preset->builtIn = builtIn;
    Entry entry;
    entry.previewPath = findPreview(presetDir, preset->id);
    entry.preset = std::move(*preset);
    out.push_back(std::move(entry));
  };

  // Top-level "<id>.flsp" files.
  for (const auto &file : root.entryList(kPresetFilters, QDir::Files, QDir::Name)) {
    loadInto(root.filePath(file), root);
  }

  // Subfolders containing a "preset.flsp".
  for (const auto &sub :
       root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
    const QDir subDir(root.filePath(sub));
    const auto presetPath = subDir.filePath("preset.flsp");
    if (QFileInfo::exists(presetPath)) {
      loadInto(presetPath, subDir);
    }
  }
}

void ShaderLibrary::refresh() {
  std::vector<Entry> scanned;
  scanDirectory(m_builtinDir, /*builtIn=*/true, scanned);
  scanDirectory(m_userDir, /*builtIn=*/false, scanned);

  // De-dup by id (user preset shadows a shipped one of the same id).
  std::vector<Entry> deduped;
  for (auto &e : scanned) {
    const auto existing =
        std::find_if(deduped.begin(), deduped.end(), [&e](const Entry &o) {
          return o.preset.id == e.preset.id;
        });
    if (existing != deduped.end()) {
      *existing = e;
    } else {
      deduped.push_back(std::move(e));
    }
  }

  for (const auto &dir : {m_builtinDir, m_userDir}) {
    if (!dir.isEmpty() && QFileInfo::exists(dir) &&
        !m_watcher.directories().contains(dir)) {
      m_watcher.addPath(dir);
    }
  }

  beginResetModel();
  m_entries = std::move(deduped);
  endResetModel();
  emit changed();
}

} // namespace firelight::graphics
