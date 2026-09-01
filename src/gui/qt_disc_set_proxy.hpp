// TODO: NEEDS REVIEW
#pragma once

#include <QObject>

namespace firelight::library {
class DiscSetService;
class IUserLibraryRepository;
} // namespace firelight::library

namespace firelight::gui {

// TODO
/**
 * QML bridge for managing which discs make up a game.
 *
 * Every method here changes membership; reading a set's contents is the disc set model's job
 */
class QtDiscSetProxy final : public QObject {
  Q_OBJECT

public:
  QtDiscSetProxy(library::DiscSetService &service, library::IUserLibraryRepository &library, QObject *parent = nullptr);

  /**
   * Puts a disc at a different number within the set holding it
   */
  Q_INVOKABLE bool setDiscNumber(int memberId, int discNumber);

  /**
   * Takes a disc out of its set and gives it one of its own
   */
  Q_INVOKABLE bool removeFromSet(int contentFileId);

  /**
   * Hands a disc back to placement, undoing an earlier removal
   */
  Q_INVOKABLE bool clearUserChoice(int entryId);

  /**
   * Accepts a placement that was recorded as a guess, so it stops being shown as one
   */
  Q_INVOKABLE bool confirmPlacement(int memberId);

signals:
  /**
   * Something changed that a view of the sets would want to read again
   */
  void setsChanged();

private:
  library::DiscSetService &m_service;
  library::IUserLibraryRepository &m_library;
};

} // namespace firelight::gui
