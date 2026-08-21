#include "gui/qt_game_art_proxy.hpp"

#include "metadata/metadata_service.hpp"

#include <firelight/metadata/media_asset.hpp>
#include <firelight/metadata/media_asset_repository.hpp>
#include <firelight/metadata/steamgriddb_art_provider.hpp>

#include <QVariantMap>

namespace firelight::gui {

QtGameArtProxy::QtGameArtProxy(metadata::MetadataService &service, metadata::SteamGridDbArtProvider &provider,
                               metadata::IMediaAssetRepository &mediaAssets, QObject *parent)
    : QObject(parent), m_service(service), m_provider(provider), m_mediaAssets(mediaAssets) {
  // One worker: SteamGridDB requests are serialized so overlapping searches
  // can't interleave state, and we never hammer the API
  m_pool.setMaxThreadCount(1);

  // Seed the provider with the persisted key (the user can also paste one at
  // runtime via setApiKey)
  const QSettings settings;
  m_provider.setApiKey(settings.value(QtGameArtProxy::API_KEY_SETTING).toString().toStdString());
}

QtGameArtProxy::~QtGameArtProxy() {
  m_shuttingDown = true;
  m_pool.waitForDone();
}

QString QtGameArtProxy::cacheKey(const QString &contentHash, int mediaType) {
  return contentHash + QStringLiteral(":") + QString::number(mediaType);
}

bool QtGameArtProxy::providerConfigured() const { return m_provider.isConfigured(); }

QString QtGameArtProxy::providerName() const { return QString::fromStdString(m_provider.name()); }

QString QtGameArtProxy::apiKey() const {
  const QSettings settings;
  return settings.value(QtGameArtProxy::API_KEY_SETTING).toString();
}

void QtGameArtProxy::setApiKey(const QString &key) {
  QSettings settings;
  settings.setValue(QtGameArtProxy::API_KEY_SETTING, key);
  m_provider.setApiKey(key.toStdString());

  // TODO
  // Entries skipped while there was no key were deliberately left unmarked, so the
  // sweep still has them to look up
  m_service.startArtSweep();
  emit providerConfiguredChanged();
}

QVariantList QtGameArtProxy::storedAssets(const QString &contentHash, int mediaType) {
  QVariantList list;
  const auto assets =
      m_mediaAssets.listForContentAndType(contentHash.toStdString(), static_cast<metadata::MediaType>(mediaType));
  for (const auto &asset : assets) {
    QVariantMap map;
    map["assetId"] = asset.id;
    map["source"] = QString::fromStdString(metadata::toString(asset.source));
    // Prefer the thumbnail for this small preview so it stays crisp and matches
    // what the search results showed
    map["url"] = QString::fromStdString(asset.displayThumb());
    map["selected"] = asset.selected;
    map["width"] = asset.width;
    map["height"] = asset.height;
    list.append(map);
  }
  return list;
}

void QtGameArtProxy::search(const QString &contentHash, const QString &gameName, int platformId, int mediaType) {
  if (m_shuttingDown || !m_provider.isConfigured()) {
    // Nothing to search: still notify so the picker can drop any spinner
    emit searchResultsReady(contentHash, mediaType);
    return;
  }

  ++m_pendingSearches;
  emit searchingChanged();

  const auto name = gameName.toStdString();
  const auto type = static_cast<metadata::MediaType>(mediaType);
  m_pool.start([this, contentHash, mediaType, name, platformId, type] {
    std::vector<metadata::ArtCandidate> results;
    if (!m_shuttingDown) {
      results = m_provider.search(name, platformId, type).candidates;
    }

    // Marshal the results back onto the GUI thread. Tying the invocation to
    // `this` means it's silently dropped if the proxy is destroyed first
    QMetaObject::invokeMethod(
        this,
        [this, contentHash, mediaType, results = std::move(results)]() mutable {
          m_results[cacheKey(contentHash, mediaType)] = QVector<metadata::ArtCandidate>(results.begin(), results.end());
          if (m_pendingSearches > 0) {
            --m_pendingSearches;
          }
          emit searchingChanged();
          emit searchResultsReady(contentHash, mediaType);
        },
        Qt::QueuedConnection);
  });
}

QVariantList QtGameArtProxy::searchResults(const QString &contentHash, int mediaType) {
  QVariantList list;
  const auto it = m_results.constFind(cacheKey(contentHash, mediaType));
  if (it == m_results.constEnd()) {
    return list;
  }
  int resultIndex = 0;
  for (const auto &candidate : *it) {
    QVariantMap map;
    map["url"] = QString::fromStdString(candidate.url);
    map["thumbUrl"] = QString::fromStdString(candidate.thumbUrl.empty() ? candidate.url : candidate.thumbUrl);
    map["externalId"] = QString::fromStdString(candidate.externalId);
    map["width"] = candidate.width;
    map["height"] = candidate.height;
    map["gameName"] = QString::fromStdString(candidate.gameName);
    // Position in the flat result list, so the grouped QML view can still call
    // applyOnline() with the right index after regrouping by game
    map["resultIndex"] = resultIndex++;
    list.append(map);
  }
  return list;
}

void QtGameArtProxy::selectStored(const QString &contentHash, int assetId) {
  m_service.selectAsset(contentHash.toStdString(), assetId);
  emit assetsChanged(contentHash, -1);
}

void QtGameArtProxy::applyOnline(const QString &contentHash, int mediaType, int index) {
  const auto it = m_results.constFind(cacheKey(contentHash, mediaType));
  if (it == m_results.constEnd() || index < 0 || index >= it->size()) {
    return;
  }
  m_service.applyCandidate(contentHash.toStdString(), it->at(index));
  emit assetsChanged(contentHash, mediaType);
}

bool QtGameArtProxy::importImage(const QString &contentHash, int mediaType, const QUrl &fileUrl) {
  const QString localPath = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();
  if (localPath.isEmpty()) {
    return false;
  }
  const bool ok = m_service.importLocalImage(contentHash.toStdString(), static_cast<metadata::MediaType>(mediaType),
                                             localPath.toStdString());
  if (ok) {
    emit assetsChanged(contentHash, mediaType);
  }
  return ok;
}

} // namespace firelight::gui
