#include "shader_chain.hpp"

#include "shader_compiler.hpp"

#include <QColor>
#include <QFile>
#include <cstring>
#include <spdlog/spdlog.h>

namespace firelight::graphics {

namespace {
// Shared fullscreen-triangle vertex shader. Emits a triangle covering the
// viewport and a 0..1 texcoord. Frame.y carries a V-flip so the caller can
// reconcile the frame's row order with the RHI framebuffer origin without every
// fragment shader having to care.
constexpr const char *kVertexGlsl = R"(#version 450
layout(std140, binding = 0) uniform UBO {
    vec4 SourceSize;
    vec4 OutputSize;
    vec4 Frame;
    vec4 Param[8];
};
layout(location = 0) out vec2 vTexCoord;
void main() {
    vec2 p = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
    vec2 uv = p;
    if (Frame.y > 0.5) { uv.y = 1.0 - uv.y; }
    vTexCoord = uv;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

// The uniform block, in std140 order matching the contract in the header.
struct UboData {
  float sourceSize[4];
  float outputSize[4];
  float frame[4];
  float param[8][4];
};

QRhiSampler::Filter filterFor(const QString &f) {
  return f == "nearest" ? QRhiSampler::Nearest : QRhiSampler::Linear;
}
QRhiSampler::AddressMode wrapFor(const QString &w) {
  return w == "repeat" ? QRhiSampler::Repeat : QRhiSampler::ClampToEdge;
}

QString readFile(const QString &path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    return {};
  }
  return QString::fromUtf8(f.readAll());
}
} // namespace

ShaderChain::ShaderChain() = default;
ShaderChain::~ShaderChain() { releaseResources(); }

void ShaderChain::releaseResources() {
  m_passes.clear();
  m_valid = false;
  m_presetId.clear();
}

bool ShaderChain::ensureBuilt(QRhi *rhi, QRhiRenderPassDescriptor *finalRpd,
                              const ShaderPreset &preset,
                              const QVector<float> &params, QRhiTexture *source,
                              const QSize &sourceSize, const QSize &outputSize,
                              QString *error) {
  m_params = params;
  const bool changed = rhi != m_rhi || finalRpd != m_finalRpd ||
                       preset.id != m_presetId || source != m_source ||
                       sourceSize != m_sourceSize ||
                       outputSize != m_outputSize || m_passes.empty();
  if (!changed) {
    return m_valid;
  }
  m_rhi = rhi;
  m_finalRpd = finalRpd;
  m_preset = preset;
  m_source = source;
  m_sourceSize = sourceSize;
  m_outputSize = outputSize;
  return rebuild(error);
}

bool ShaderChain::rebuild(QString *error) {
  releaseResources();
  m_presetId = m_preset.id;

  if (!m_rhi || !m_finalRpd || !m_source || m_preset.passes.isEmpty() ||
      m_sourceSize.isEmpty() || m_outputSize.isEmpty()) {
    if (error) {
      *error = QStringLiteral("shader chain: missing rhi/target/source/size");
    }
    return false;
  }

  // Compile the shared vertex shader once per rebuild.
  auto vs = shader_compiler::compile(QShader::VertexStage,
                                     QString::fromUtf8(kVertexGlsl), error);
  if (!vs) {
    return false;
  }
  m_vertexShader = *vs;

  const int passCount = m_preset.passes.size();
  for (int i = 0; i < passCount; ++i) {
    const auto &def = m_preset.passes[i];
    const bool isLast = (i == passCount - 1);

    const auto fragSource = readFile(def.fragmentGlsl);
    if (fragSource.isEmpty()) {
      if (error) {
        *error = QStringLiteral("could not read fragment shader %1")
                     .arg(def.fragmentGlsl);
      }
      return false;
    }
    auto fs = shader_compiler::compile(QShader::FragmentStage, fragSource, error);
    if (!fs) {
      return false;
    }

    Pass pass;
    pass.filter = def.filter;
    pass.wrap = def.wrap;

    // Input for pass 0 is the source; later passes read the previous output.
    pass.inputTex = (i == 0) ? m_source : m_passes[i - 1].outTex.get();
    pass.inputSize = (i == 0) ? m_sourceSize : m_passes[i - 1].outSize;

    // Output size: the final pass matches the destination; earlier passes scale
    // off the source.
    pass.outSize =
        isLast ? m_outputSize
               : QSize(qMax(1, int(m_sourceSize.width() * def.scale)),
                       qMax(1, int(m_sourceSize.height() * def.scale)));

    QRhiRenderPassDescriptor *rpd = m_finalRpd;
    if (!isLast) {
      pass.outTex.reset(m_rhi->newTexture(QRhiTexture::RGBA8, pass.outSize, 1,
                                          QRhiTexture::RenderTarget |
                                              QRhiTexture::UsedAsTransferSource));
      if (!pass.outTex->create()) {
        if (error) {
          *error = QStringLiteral("failed to create intermediate texture");
        }
        return false;
      }
      QRhiColorAttachment att(pass.outTex.get());
      QRhiTextureRenderTargetDescription rtDesc(att);
      pass.outRt.reset(m_rhi->newTextureRenderTarget(rtDesc));
      pass.outRpd.reset(pass.outRt->newCompatibleRenderPassDescriptor());
      pass.outRt->setRenderPassDescriptor(pass.outRpd.get());
      if (!pass.outRt->create()) {
        if (error) {
          *error = QStringLiteral("failed to create intermediate render target");
        }
        return false;
      }
      rpd = pass.outRpd.get();
    }

    pass.ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic,
                                     QRhiBuffer::UniformBuffer,
                                     sizeof(UboData)));
    if (!pass.ubuf->create()) {
      if (error) {
        *error = QStringLiteral("failed to create uniform buffer");
      }
      return false;
    }

    pass.sampler.reset(m_rhi->newSampler(
        filterFor(def.filter), filterFor(def.filter), QRhiSampler::None,
        wrapFor(def.wrap), wrapFor(def.wrap)));
    if (!pass.sampler->create()) {
      if (error) {
        *error = QStringLiteral("failed to create sampler");
      }
      return false;
    }

    // Input textures are stable objects (source / prior pass output) for the
    // lifetime of this build, so wire the bindings once here — no per-frame srb
    // churn, and the layout matches the pipeline exactly.
    pass.srb.reset(m_rhi->newShaderResourceBindings());
    pass.srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(
            0,
            QRhiShaderResourceBinding::VertexStage |
                QRhiShaderResourceBinding::FragmentStage,
            pass.ubuf.get()),
        QRhiShaderResourceBinding::sampledTexture(
            1, QRhiShaderResourceBinding::FragmentStage, pass.inputTex,
            pass.sampler.get()),
    });
    if (!pass.srb->create()) {
      if (error) {
        *error = QStringLiteral("failed to create shader resource bindings");
      }
      return false;
    }

    pass.pipeline.reset(m_rhi->newGraphicsPipeline());
    pass.pipeline->setShaderStages(
        {{QRhiShaderStage::Vertex, m_vertexShader},
         {QRhiShaderStage::Fragment, *fs}});
    QRhiVertexInputLayout inputLayout; // no vertex buffers (gl_VertexIndex)
    pass.pipeline->setVertexInputLayout(inputLayout);
    pass.pipeline->setShaderResourceBindings(pass.srb.get());
    pass.pipeline->setRenderPassDescriptor(rpd);
    if (!pass.pipeline->create()) {
      if (error) {
        *error = QStringLiteral("failed to create graphics pipeline");
      }
      return false;
    }

    m_passes.push_back(std::move(pass));
  }

  m_valid = true;
  return true;
}

void ShaderChain::drawPass(QRhiCommandBuffer *cb, const Pass &pass,
                           QRhiRenderTarget *rt, const QSize &inSize,
                           const QSize &outSize, quint64 frameCount) {
  QRhiResourceUpdateBatch *batch = m_rhi->nextResourceUpdateBatch();

  UboData ubo{};
  ubo.sourceSize[0] = float(inSize.width());
  ubo.sourceSize[1] = float(inSize.height());
  ubo.sourceSize[2] = inSize.width() ? 1.0f / inSize.width() : 0.0f;
  ubo.sourceSize[3] = inSize.height() ? 1.0f / inSize.height() : 0.0f;
  ubo.outputSize[0] = float(outSize.width());
  ubo.outputSize[1] = float(outSize.height());
  ubo.outputSize[2] = outSize.width() ? 1.0f / outSize.width() : 0.0f;
  ubo.outputSize[3] = outSize.height() ? 1.0f / outSize.height() : 0.0f;
  ubo.frame[0] = float(frameCount);
  // Reconcile frame row order with the framebuffer origin. Tunable per backend;
  // isYUpInFramebuffer() differs between GL and the others.
  ubo.frame[1] = m_rhi->isYUpInFramebuffer() ? 0.0f : 1.0f;
  for (int i = 0; i < m_params.size() && i < 8; ++i) {
    ubo.param[i][0] = m_params[i];
  }
  batch->updateDynamicBuffer(pass.ubuf.get(), 0, sizeof(UboData), &ubo);

  const QColor clear(0, 0, 0, 1);
  cb->beginPass(rt, clear, {1.0f, 0}, batch);
  cb->setGraphicsPipeline(pass.pipeline.get());
  cb->setViewport({0, 0, float(outSize.width()), float(outSize.height())});
  cb->setShaderResources(pass.srb.get());
  cb->draw(3);
  cb->endPass();
}

void ShaderChain::render(QRhiCommandBuffer *cb, QRhiRenderTarget *finalRt,
                         quint64 frameCount) {
  if (!m_valid || m_passes.empty() || !finalRt) {
    return;
  }

  for (size_t i = 0; i < m_passes.size(); ++i) {
    const bool isLast = (i == m_passes.size() - 1);
    const auto &pass = m_passes[i];
    QRhiRenderTarget *rt = isLast ? finalRt : pass.outRt.get();
    const QSize outSize = isLast ? m_outputSize : pass.outSize;
    drawPass(cb, pass, rt, pass.inputSize, outSize, frameCount);
  }
}

} // namespace firelight::graphics
