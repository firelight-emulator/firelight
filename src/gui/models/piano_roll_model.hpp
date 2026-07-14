#pragma once

#include <QAbstractListModel>

namespace firelight::tas {
class TasSession;
}

namespace firelight::gui {

// The editable input grid behind the TAS Studio's piano-roll. Rows are the movie's
// frames; each row carries the frame index, its 16-bit button bitmask, whether it is
// the session's current (playhead) frame, and whether it is a greenzone keyframe.
//
// All edits route through the bound TasSession (edit -> invalidate greenzone tail ->
// bump rerecord -> reposition if the playhead was ahead), so the grid and the
// emulator never disagree. The model holds every toggle/paint semantic and is unit-
// tested headlessly; the QML TableView is a thin presentation layer over it.
class PianoRollModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int currentFrame READ currentFrame NOTIFY playheadChanged)
  Q_PROPERTY(int frameCount READ frameCount NOTIFY movieChanged)
  Q_PROPERTY(int rerecordCount READ rerecordCount NOTIFY movieChanged)

public:
  enum Roles {
    FrameIndexRole = Qt::UserRole + 1,
    ButtonsRole,   // uint16 RETRO_DEVICE_ID_JOYPAD_* bitmask for the frame
    IsCurrentRole, // is the session's playhead on this frame
    IsKeyframeRole // is there a greenzone keyframe at this frame
  };

  explicit PianoRollModel(QObject *parent = nullptr);

  // Bind (or rebind) the session whose movie this grid edits; nullptr => empty grid.
  void setSession(tas::TasSession *session);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int currentFrame() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int rerecordCount() const;

  // Whether button `buttonId` (0..15) is held on `frame`.
  Q_INVOKABLE bool isButtonPressed(int frame, int buttonId) const;
  // Flip one cell; returns the resulting pressed state (unchanged/false if invalid).
  Q_INVOKABLE bool toggleButton(int frame, int buttonId);
  // Drag-paint: set `buttonId` to `pressed` across [firstFrame, lastFrame]
  // (order-independent).
  Q_INVOKABLE void paintButton(int firstFrame, int lastFrame, int buttonId,
                               bool pressed);

  // Re-read the session after external changes (a step/seek moved the playhead, or
  // playback advanced) so the view refreshes.
  Q_INVOKABLE void refresh();

signals:
  void playheadChanged();
  void movieChanged();

private:
  // Edits one cell through the session without emitting; returns true if it changed.
  bool applyCell(int frame, int buttonId, bool pressed);
  // Emit the range/playhead/movie change notifications after an edit at/after `from`.
  void emitEdited(int fromFrame);

  tas::TasSession *m_session = nullptr;
};

} // namespace firelight::gui
