#include "border_library.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::graphics {

namespace {
// Image extensions we treat as border art.
const QStringList kImageFilters = {"*.png", "*.jpg", "*.jpeg", "*.webp"};

QRectF parseScreenRect(const QJsonObject &obj) {
  // Default to the full window (a frame occupies everything behind the game).
  QRectF rect(0.0, 0.0, 1.0, 1.0);
  if (!obj.contains("screenRect") || !obj["screenRect"].isObject()) {
    return rect;
  }
  const auto r = obj["screenRect"].toObject();
  rect.setX(r.value("x").toDouble(0.0));
  rect.setY(r.value("y").toDouble(0.0));
  rect.setWidth(r.value("width").toDouble(1.0));
  rect.setHeight(r.value("height").toDouble(1.0));
  return rect;
}
} // namespace

BorderLibrary::BorderLibrary(QString builtinDir, QString userDir,
                             QObject *parent)
    : QAbstractListModel(parent), m_builtinDir(std::move(builtinDir)),
      m_userDir(std::move(userDir)) {
  for (const auto &dir : {m_builtinDir, m_userDir}) {
    if (!dir.isEmpty() && QFileInfo::exists(dir)) {
      m_watcher.addPath(dir);
    }
  }
  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
          &BorderLibrary::refresh);
  refresh();
}

int BorderLibrary::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_borders.size());
}

QVariant BorderLibrary::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(m_borders.size())) {
    return {};
  }
  const auto &b = m_borders[index.row()];
  switch (role) {
  case IdRole:
    return b.id;
  case NameRole:
    return b.name;
  case TypeRole:
    return b.type;
  case ImageUrlRole:
    return QUrl::fromLocalFile(b.imagePath).toString();
  case ScreenXRole:
    return b.screenRect.x();
  case ScreenYRole:
    return b.screenRect.y();
  case ScreenWidthRole:
    return b.screenRect.width();
  case ScreenHeightRole:
    return b.screenRect.height();
  case OpacityRole:
    return b.opacity;
  case BuiltInRole:
    return b.builtIn;
  default:
    return {};
  }
}

QHash<int, QByteArray> BorderLibrary::roleNames() const {
  return {
      {IdRole, "borderId"},          {NameRole, "name"},
      {TypeRole, "type"},            {ImageUrlRole, "imageUrl"},
      {ScreenXRole, "screenX"},      {ScreenYRole, "screenY"},
      {ScreenWidthRole, "screenWidth"},
      {ScreenHeightRole, "screenHeight"},
      {OpacityRole, "opacity"},      {BuiltInRole, "builtIn"},
  };
}

QVariantMap BorderLibrary::toMap(const BorderInfo &b) {
  return {
      {"borderId", b.id},
      {"name", b.name},
      {"type", b.type},
      {"imageUrl", QUrl::fromLocalFile(b.imagePath).toString()},
      {"screenX", b.screenRect.x()},
      {"screenY", b.screenRect.y()},
      {"screenWidth", b.screenRect.width()},
      {"screenHeight", b.screenRect.height()},
      {"opacity", b.opacity},
      {"builtIn", b.builtIn},
  };
}

QVariantMap BorderLibrary::borderById(const QString &id) const {
  if (id.isEmpty()) {
    return {};
  }
  const auto it = std::find_if(m_borders.begin(), m_borders.end(),
                               [&id](const BorderInfo &b) { return b.id == id; });
  if (it == m_borders.end()) {
    return {};
  }
  return toMap(*it);
}

void BorderLibrary::scanDirectory(const QString &dir, const bool builtIn,
                                  std::vector<BorderInfo> &out) const {
  if (dir.isEmpty() || !QFileInfo::exists(dir)) {
    return;
  }
  const QDir root(dir);

  // A border is either a subfolder containing a border.json (+ image) or a bare
  // image file (treated as a simple full-window frame).
  const auto subdirs =
      root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const auto &sub : subdirs) {
    const QDir borderDir(root.filePath(sub));
    const QString manifestPath = borderDir.filePath("border.json");

    BorderInfo info;
    info.id = sub;
    info.name = sub;
    info.type = "frame";
    info.builtIn = builtIn;

    if (QFileInfo::exists(manifestPath)) {
      QFile f(manifestPath);
      if (f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
          const auto obj = doc.object();
          info.name = obj.value("name").toString(sub);
          info.type = obj.value("type").toString("frame");
          info.opacity = obj.value("opacity").toDouble(1.0);
          info.screenRect = parseScreenRect(obj);
          const auto image = obj.value("image").toString();
          if (!image.isEmpty()) {
            info.imagePath = borderDir.filePath(image);
          }
        }
      }
    }

    // Fall back to the first image in the folder when no explicit image is set.
    if (info.imagePath.isEmpty()) {
      const auto images = borderDir.entryList(kImageFilters, QDir::Files);
      if (!images.isEmpty()) {
        info.imagePath = borderDir.filePath(images.first());
      }
    }

    if (info.imagePath.isEmpty() || !QFileInfo::exists(info.imagePath)) {
      spdlog::warn("Border '{}' has no usable image; skipping",
                   info.id.toStdString());
      continue;
    }
    out.push_back(std::move(info));
  }

  // Bare image files → simple frames.
  const auto files = root.entryList(kImageFilters, QDir::Files, QDir::Name);
  for (const auto &file : files) {
    BorderInfo info;
    info.id = QFileInfo(file).completeBaseName();
    info.name = info.id;
    info.type = "frame";
    info.imagePath = root.filePath(file);
    info.builtIn = builtIn;
    out.push_back(std::move(info));
  }
}

void BorderLibrary::refresh() {
  std::vector<BorderInfo> scanned;
  scanDirectory(m_builtinDir, /*builtIn=*/true, scanned);
  scanDirectory(m_userDir, /*builtIn=*/false, scanned);

  // De-dup by id (a user border shadows a shipped one of the same name).
  std::vector<BorderInfo> deduped;
  for (auto &b : scanned) {
    const auto existing =
        std::find_if(deduped.begin(), deduped.end(),
                     [&b](const BorderInfo &o) { return o.id == b.id; });
    if (existing != deduped.end()) {
      *existing = b; // later (user) entry wins
    } else {
      deduped.push_back(std::move(b));
    }
  }

  // Re-arm the watcher (directories may have been created since construction).
  for (const auto &dir : {m_builtinDir, m_userDir}) {
    if (!dir.isEmpty() && QFileInfo::exists(dir) &&
        !m_watcher.directories().contains(dir)) {
      m_watcher.addPath(dir);
    }
  }

  beginResetModel();
  m_borders = std::move(deduped);
  endResetModel();
  emit changed();
}

} // namespace firelight::graphics
