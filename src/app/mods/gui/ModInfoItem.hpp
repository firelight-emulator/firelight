#pragma once
#include <manager_accessor.hpp>

#include <QObject>
#include <QVariant>

namespace firelight::mods {

class ModInfoItem : public QObject, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(int modId READ modId WRITE setModId NOTIFY modIdChanged)
  Q_PROPERTY(QString modName READ modName NOTIFY modNameChanged)
  Q_PROPERTY(QString version READ version NOTIFY versionChanged)
  Q_PROPERTY(QString authorName READ authorName NOTIFY authorNameChanged)
  Q_PROPERTY(QString targetGameName READ targetGameName NOTIFY targetGameNameChanged)
  Q_PROPERTY(QString targetContentHash READ targetContentHash NOTIFY targetContentHashChanged)
  Q_PROPERTY(QString platformName READ platformName NOTIFY platformNameChanged)
  Q_PROPERTY(QString tagline READ tagline NOTIFY taglineChanged)
  Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
  Q_PROPERTY(QString clearLogoUrl READ clearLogoUrl NOTIFY clearLogoUrlChanged)
  Q_PROPERTY(QVariant mediaUrls READ mediaUrls NOTIFY mediaUrlsChanged)

public:
  explicit ModInfoItem(const QObject *parent = nullptr);
  int modId() const;
  void setModId(int modId);
  [[nodiscard]] QString modName() const;
  [[nodiscard]] QString version() const;
  [[nodiscard]] QString authorName() const;
  [[nodiscard]] QString targetGameName() const;
  [[nodiscard]] QString targetContentHash() const;
  [[nodiscard]] QString platformName() const;
  [[nodiscard]] QString tagline() const;
  [[nodiscard]] QString description() const;
  [[nodiscard]] QString clearLogoUrl() const;
  [[nodiscard]] QVariant mediaUrls() const;

signals:
  void modIdChanged(int modId);
  void modNameChanged(const QString &modName);
  void versionChanged(const QString &version);
  void authorNameChanged(const QString &authorName);
  void targetGameNameChanged(const QString &targetGameName);
  void targetContentHashChanged(const QString &targetContentHash);
  void platformNameChanged(const QString &platformName);
  void taglineChanged(const QString &tagline);
  void descriptionChanged(const QString &description);
  void clearLogoUrlChanged(const QString &clearLogoUrl);
  void mediaUrlsChanged(const QVariant &mediaUrls);

private:
  int m_modId{};
  QString m_modName;
  QString m_version;
  QString m_authorName;
  QString m_targetGameName;
  QString m_targetContentHash;
  QString m_platformName;
  QString m_tagline;
  QString m_description;
  QString m_clearLogoUrl;
  QVariantList m_mediaUrls;

};

} // firelight::mods