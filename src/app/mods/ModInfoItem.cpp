#include "ModInfoItem.hpp"

#include "../platform_metadata.hpp"

namespace firelight::mods {
ModInfoItem::ModInfoItem(const QObject *parent) {
  Q_UNUSED(parent);
}

int ModInfoItem::modId() const {
  return m_modId;
}

void ModInfoItem::setModId(int modId) {
  if (m_modId == modId) {
    return;
  }

  m_modId = modId;

  auto mod = getModRepository()->getModInfo(modId);
  // if (!mod) {
  //   m_modName = "";
  //   emit modNameChanged(m_modName);
  //
  //   m_version = "";
  //   emit versionChanged(m_version);
  //
  //   m_authorName = "";
  //   emit authorNameChanged(m_authorName);
  //
  //   m_targetGameName = "";
  //   emit targetGameNameChanged(m_targetGameName);
  //
  //   m_targetContentHash = "";
  //   emit targetContentHashChanged(m_targetContentHash);
  //
  //   m_platformName = "";
  //   emit platformNameChanged(m_platformName);
  //
  //   m_tagline = "";
  //   emit taglineChanged(m_tagline);
  //
  //   m_description = "";
  //   emit descriptionChanged(m_description);
  //
  //   m_clearLogoUrl = "";
  //   emit clearLogoUrlChanged(m_clearLogoUrl);
  //
  //   m_mediaUrls = QVariant();
  //   emit mediaUrlsChanged(m_mediaUrls);
  //
  //   emit modIdChanged(modId);
  //   return;
  // }

  m_modName = QString::fromStdString(mod->name);
  emit modNameChanged(m_modName);

  m_version = QString::fromStdString(mod->version);
  emit versionChanged(m_version);

  m_authorName = QString::fromStdString(mod->author);
  emit authorNameChanged(m_authorName);

  m_targetGameName = QString::fromStdString(mod->targetGameName);
  emit targetGameNameChanged(m_targetGameName);

  m_targetContentHash = QString::fromStdString(mod->targetContentHash);
  emit targetContentHashChanged(m_targetContentHash);

  m_platformName = QString::fromStdString(PlatformMetadata::getPlatformName(mod->platformId));
  emit platformNameChanged(m_platformName);

  m_tagline = QString::fromStdString(mod->tagline);
  emit taglineChanged(m_tagline);

  m_description = QString::fromStdString(mod->description);
  emit descriptionChanged(m_description);

  m_clearLogoUrl = QString::fromStdString(mod->clearLogoUrl);
  emit clearLogoUrlChanged(m_clearLogoUrl);

  m_mediaUrls.clear();
  for (const auto &url : mod->mediaUrls) {
    m_mediaUrls.emplace_back(QString::fromStdString(url));
  }
  emit mediaUrlsChanged(m_mediaUrls);

  emit modIdChanged(modId);
}

QString ModInfoItem::modName() const {
  return m_modName;
}

QString ModInfoItem::version() const {
  return m_version;
}

QString ModInfoItem::authorName() const {
  return m_authorName;
}

QString ModInfoItem::targetGameName() const {
  return m_targetGameName;
}

QString ModInfoItem::targetContentHash() const {
  return m_targetContentHash;
}

QString ModInfoItem::platformName() const {
  return m_platformName;
}

QString ModInfoItem::tagline() const {
  return m_tagline;
}

QString ModInfoItem::description() const {
  return m_description;
}

QString ModInfoItem::clearLogoUrl() const {
  return m_clearLogoUrl;
}

QVariant ModInfoItem::mediaUrls() const {
  return m_mediaUrls;
}
} // firelight::mods