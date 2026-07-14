#include "piano_roll_model.hpp"

#include <firelight/input/input_frame.hpp>
#include <firelight/tas/tas_session.hpp>

#include <algorithm>

namespace firelight::gui {

PianoRollModel::PianoRollModel(QObject *parent) : QAbstractListModel(parent) {}

void PianoRollModel::setSession(tas::TasSession *session) {
  beginResetModel();
  m_session = session;
  endResetModel();
  emit playheadChanged();
  emit movieChanged();
}

int PianoRollModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid() || m_session == nullptr) {
    return 0;
  }
  return static_cast<int>(m_session->movieLength());
}

QVariant PianoRollModel::data(const QModelIndex &index, int role) const {
  if (m_session == nullptr || !index.isValid()) {
    return {};
  }
  const int row = index.row();
  if (row < 0 || row >= static_cast<int>(m_session->movieLength())) {
    return {};
  }
  switch (role) {
  case FrameIndexRole:
    return row;
  case ButtonsRole:
    return static_cast<unsigned>(m_session->movie()[static_cast<size_t>(row)].buttons);
  case IsCurrentRole:
    return static_cast<uint64_t>(row) == m_session->currentFrame();
  case IsKeyframeRole:
    return m_session->greenzone().has(static_cast<uint64_t>(row));
  default:
    return {};
  }
}

QHash<int, QByteArray> PianoRollModel::roleNames() const {
  return {
      {FrameIndexRole, "frameIndex"},
      {ButtonsRole, "buttons"},
      {IsCurrentRole, "isCurrent"},
      {IsKeyframeRole, "isKeyframe"},
  };
}

int PianoRollModel::currentFrame() const {
  return m_session ? static_cast<int>(m_session->currentFrame()) : 0;
}

int PianoRollModel::frameCount() const {
  return m_session ? static_cast<int>(m_session->movieLength()) : 0;
}

int PianoRollModel::rerecordCount() const {
  return m_session ? static_cast<int>(m_session->rerecordCount()) : 0;
}

bool PianoRollModel::isButtonPressed(int frame, int buttonId) const {
  if (m_session == nullptr || frame < 0 ||
      frame >= static_cast<int>(m_session->movieLength())) {
    return false;
  }
  return m_session->movie()[static_cast<size_t>(frame)].button(
      static_cast<unsigned>(buttonId));
}

bool PianoRollModel::applyCell(int frame, int buttonId, bool pressed) {
  if (m_session == nullptr || frame < 0 ||
      frame >= static_cast<int>(m_session->movieLength()) || buttonId < 0 ||
      buttonId >= 16) {
    return false;
  }
  input::InputFrame f = m_session->movie()[static_cast<size_t>(frame)];
  if (f.button(static_cast<unsigned>(buttonId)) == pressed) {
    return false; // no change
  }
  f.setButton(static_cast<unsigned>(buttonId), pressed);
  m_session->editFrame(static_cast<uint64_t>(frame), f);
  return true;
}

bool PianoRollModel::toggleButton(int frame, int buttonId) {
  const bool next = !isButtonPressed(frame, buttonId);
  if (applyCell(frame, buttonId, next)) {
    emitEdited(frame);
  }
  // The actual resulting state: `next` if the edit applied, else unchanged (e.g. no
  // bound session or an out-of-range frame, where nothing happens).
  return isButtonPressed(frame, buttonId);
}

void PianoRollModel::paintButton(int firstFrame, int lastFrame, int buttonId,
                                 bool pressed) {
  if (m_session == nullptr) {
    return;
  }
  const int lo = std::max(0, std::min(firstFrame, lastFrame));
  const int hi = std::min(static_cast<int>(m_session->movieLength()) - 1,
                          std::max(firstFrame, lastFrame));
  bool changed = false;
  // Ascending so the edit only repositions the playhead once, to the lowest frame.
  for (int f = lo; f <= hi; ++f) {
    changed = applyCell(f, buttonId, pressed) || changed;
  }
  if (changed) {
    emitEdited(lo);
  }
}

void PianoRollModel::emitEdited(int fromFrame) {
  const int last = rowCount() - 1;
  if (last >= 0 && fromFrame <= last) {
    // Buttons changed at/after the edit; the greenzone tail after it was
    // invalidated, and the playhead may have been pulled back to the edit point.
    emit dataChanged(index(std::max(0, fromFrame)), index(last),
                     {ButtonsRole, IsKeyframeRole, IsCurrentRole});
  }
  emit playheadChanged();
  emit movieChanged();
}

void PianoRollModel::refresh() {
  const int last = rowCount() - 1;
  if (last >= 0) {
    emit dataChanged(index(0), index(last),
                     {IsCurrentRole, IsKeyframeRole, ButtonsRole});
  }
  emit playheadChanged();
  emit movieChanged();
}

} // namespace firelight::gui
