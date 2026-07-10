#pragma once

#include "service_accessor.hpp"
#include "shader_chain.hpp"

#include <QQuickRhiItem>
#include <QVector>
#include <optional>

namespace firelight::graphics {

// A small QQuickRhiItem that renders a live preview of a shader preset on a
// procedurally-generated sample frame, so the shader picker can show the actual
// effect (and update as parameter sliders move). It reuses the same ShaderChain
// as the emulator, so what you see here is what you get in-game. Fully isolated
// from the emulator render path.
//
// QML usage: ShaderPreviewItem { shaderId: "crt-scanlines"; params: '{"strength":0.5}' }
class ShaderPreviewItem : public QQuickRhiItem, public firelight::ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(
      QString shaderId READ shaderId WRITE setShaderId NOTIFY shaderIdChanged)
  // JSON object of parameter overrides, e.g. {"strength":0.5}. Missing keys use
  // the preset's declared defaults.
  Q_PROPERTY(QString params READ params WRITE setParams NOTIFY paramsChanged)

public:
  explicit ShaderPreviewItem(QQuickItem *parent = nullptr);

  [[nodiscard]] QString shaderId() const { return m_shaderId; }
  void setShaderId(const QString &id);

  [[nodiscard]] QString params() const { return m_params; }
  void setParams(const QString &params);

  // --- read by the renderer in synchronize() (GUI thread blocked) ---
  [[nodiscard]] bool hasPreset() const { return m_preset.has_value(); }
  [[nodiscard]] const ShaderPreset &preset() const { return *m_preset; }
  [[nodiscard]] const QVector<float> &paramValues() const {
    return m_paramValues;
  }
  // Bumped on any change so the renderer knows to re-resolve.
  [[nodiscard]] quint64 revision() const { return m_revision; }

signals:
  void shaderIdChanged();
  void paramsChanged();

protected:
  QQuickRhiItemRenderer *createRenderer() override;

private:
  QString m_shaderId;
  QString m_params;
  std::optional<ShaderPreset> m_preset;
  QVector<float> m_paramValues;
  quint64 m_revision = 0;

  // Resolves m_shaderId + m_params -> m_preset + m_paramValues and repaints.
  void resolve();
};

} // namespace firelight::graphics
