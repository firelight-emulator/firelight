#pragma once

#include <firelight/metadata/art_provider.hpp>

#include <QHash>
#include <QObject>
#include <QSettings>
#include <QThreadPool>
#include <QUrl>
#include <QVariantList>
#include <QVector>

#include <atomic>

namespace firelight::metadata {
class MetadataService;
class IMediaAssetRepository;
class SteamGridDbArtProvider;
} // namespace firelight::metadata

namespace firelight::gui {

// QML bridge backing the "Change artwork" picker. Exposes, per (contentHash,
// mediaType): the stored candidates + current selection (from the media store),
// an async SteamGridDB search, and actions to select a stored asset, apply an
// online candidate, or import a local image. Every mutating action goes through
// MetadataService, which reprojects the entry's art columns and emits
// EntryUpdatedEvent so the library tile refreshes in place.
class QtGameArtProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool providerConfigured READ providerConfigured NOTIFY
                 providerConfiguredChanged)
  Q_PROPERTY(QString providerName READ providerName CONSTANT)
  Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)
  Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY
                 providerConfiguredChanged)

public:
  QtGameArtProxy(metadata::MetadataService &service,
                 metadata::SteamGridDbArtProvider &provider,
                 metadata::IMediaAssetRepository &mediaAssets,
                 QObject *parent = nullptr);
  ~QtGameArtProxy() override;

  [[nodiscard]] bool providerConfigured() const;
  [[nodiscard]] QString providerName() const;
  [[nodiscard]] bool searching() const { return m_pendingSearches > 0; }
  [[nodiscard]] QString apiKey() const;
  void setApiKey(const QString &key);

  // The stored candidates for a (game, type), newest first:
  // [{assetId, source, url, selected, width, height}]. Read on the GUI thread;
  // the media store is internally synchronized.
  Q_INVOKABLE QVariantList storedAssets(const QString &contentHash,
                                        int mediaType);

  // Kicks off an async provider search on a worker thread. Emits
  // searchResultsReady(contentHash, mediaType) when it finishes (also on
  // failure, with an empty result set); the picker then reads searchResults().
  Q_INVOKABLE void search(const QString &contentHash, const QString &gameName,
                          int platformId, int mediaType);

  // The most-recent search results for a (game, type), index-addressable:
  // [{url, thumbUrl, externalId, width, height}].
  Q_INVOKABLE QVariantList searchResults(const QString &contentHash,
                                         int mediaType);

  // Makes an already-stored asset the selection for its (game, type).
  Q_INVOKABLE void selectStored(const QString &contentHash, int assetId);

  // Applies search-result [index] for (game, type) as a new, selected asset.
  Q_INVOKABLE void applyOnline(const QString &contentHash, int mediaType,
                               int index);

  // Copies a local image file into the managed media dir and selects it.
  Q_INVOKABLE bool importImage(const QString &contentHash, int mediaType,
                               const QUrl &fileUrl);

signals:
  void providerConfiguredChanged();
  void searchingChanged();
  // Fired on the GUI thread when a search finishes; the picker reads
  // searchResults() in response.
  void searchResultsReady(const QString &contentHash, int mediaType);
  // Fired after any selection/apply/import so the picker re-reads storedAssets()
  // and highlights the new selection.
  void assetsChanged(const QString &contentHash, int mediaType);

private:
  static QString cacheKey(const QString &contentHash, int mediaType);

  metadata::MetadataService &m_service;
  metadata::SteamGridDbArtProvider &m_provider;
  metadata::IMediaAssetRepository &m_mediaAssets;

  QThreadPool m_pool;
  std::atomic_bool m_shuttingDown = false;
  int m_pendingSearches = 0; // GUI-thread only

  // Last search results per (contentHash, mediaType). Touched only on the GUI
  // thread (workers marshal their results here), so no lock is needed.
  QHash<QString, QVector<metadata::ArtCandidate>> m_results;
};

} // namespace firelight::gui
