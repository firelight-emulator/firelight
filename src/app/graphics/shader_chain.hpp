#pragma once

#include "shader_preset.hpp"

#include <QSize>
#include <QString>
#include <QVector>
#include <memory>
#include <rhi/qrhi.h>
#include <vector>

namespace firelight::graphics {

// A multi-pass video post-processing chain built on Qt RHI, so it runs on
// whatever backend the app uses (D3D11 / OpenGL / Vulkan / Metal). Each pass is
// a fullscreen draw sampling the previous pass's output; the final pass renders
// into the caller's destination render target (the EmulatorItem's colorTexture,
// or the preview item's texture).
//
// Shader contract (GLSL, #version 450):
//   layout(location=0) in vec2 vTexCoord;              // vertex -> fragment
//   layout(location=0) out vec4 FragColor;
//   layout(binding=1) uniform sampler2D Source;        // previous pass / frame
//   layout(std140, binding=0) uniform UBO {
//       vec4 SourceSize;   // xy = input px, zw = 1/input px
//       vec4 OutputSize;   // xy = output px, zw = 1/output px
//       vec4 Frame;        // x = frame count, y = internal (V-flip)
//       vec4 Param[8];     // Param[i].x = declared parameter i
//   };
//   layout(binding=2) uniform sampler2D Feedback;  // previous frame's final
//       // output — only meaningful when the preset sets "feedback": true.
class ShaderChain {
public:
  ShaderChain();
  ~ShaderChain();

  // (Re)builds GPU resources for `preset`. `source` is the input texture for the
  // first pass (its object identity + size is captured, so the shader resource
  // bindings can be wired once here rather than per frame). `finalRpd` is the
  // destination's render pass descriptor. Returns false (and sets `error`) on
  // failure — the caller should then bypass shading. Safe to call every frame;
  // it only does work when something changed.
  bool ensureBuilt(QRhi *rhi, QRhiRenderPassDescriptor *finalRpd,
                   const ShaderPreset &preset, const QVector<float> &params,
                   QRhiTexture *source, const QSize &sourceSize,
                   const QSize &outputSize, QString *error = nullptr);

  [[nodiscard]] bool isValid() const { return m_valid; }
  [[nodiscard]] QString presetId() const { return m_presetId; }

  // Runs every pass: source -> intermediate(s) -> `finalRt`. Must be called
  // outside an active render pass (it opens its own passes). `frameCount` feeds
  // the Frame uniform. `finalTex` is the texture backing `finalRt` (the caller's
  // colorTexture); for feedback presets it is copied into the history texture at
  // the end of the frame.
  void render(QRhiCommandBuffer *cb, QRhiRenderTarget *finalRt,
              QRhiTexture *finalTex, quint64 frameCount);

  // Updates just the tunable parameter values (no rebuild).
  void setParams(const QVector<float> &params) { m_params = params; }

  void releaseResources();

private:
  struct Pass {
    std::unique_ptr<QRhiBuffer> ubuf;
    std::unique_ptr<QRhiSampler> sampler;
    std::unique_ptr<QRhiShaderResourceBindings> srb;
    std::unique_ptr<QRhiGraphicsPipeline> pipeline;
    // Owned output for all but the last pass (last renders into finalRt).
    std::unique_ptr<QRhiTexture> outTex;
    std::unique_ptr<QRhiTextureRenderTarget> outRt;
    std::unique_ptr<QRhiRenderPassDescriptor> outRpd;
    QRhiTexture *inputTex = nullptr; // not owned (source or prior pass output)
    QSize inputSize;
    QSize outSize;
    QString filter;
    QString wrap;
  };

  QRhi *m_rhi = nullptr;
  bool m_valid = false;
  QString m_presetId;
  ShaderPreset m_preset;
  QVector<float> m_params;
  QRhiTexture *m_source = nullptr;
  QSize m_sourceSize;
  QSize m_outputSize;
  QRhiRenderPassDescriptor *m_finalRpd = nullptr;
  QShader m_vertexShader;
  std::vector<Pass> m_passes;

  // Feedback (previous-frame) support, allocated only for feedback presets.
  bool m_feedback = false;
  bool m_historyInitialized = false;
  std::unique_ptr<QRhiTexture> m_historyTex;
  std::unique_ptr<QRhiSampler> m_feedbackSampler;

  bool rebuild(QString *error);
  void drawPass(QRhiCommandBuffer *cb, const Pass &pass, QRhiRenderTarget *rt,
                const QSize &inSize, const QSize &outSize, quint64 frameCount);
};

} // namespace firelight::graphics
