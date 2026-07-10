#include "shader_preview_item.hpp"

#include "shader_library.hpp"

#include "../gui/game_image_provider.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLinearGradient>
#include <QPainter>
#include <QUrl>
#include <memory>

namespace firelight::graphics {

namespace {
// Builds a representative sample frame: gradients (to show colour handling),
// hard edges and fine lines (to show scanlines / sharpening / masks). Generated
// once, so no asset needs shipping.
QImage buildSampleImage() {
  constexpr int w = 320;
  constexpr int h = 240;
  QImage img(w, h, QImage::Format_RGBA8888);
  img.fill(Qt::black);
  QPainter p(&img);

  QLinearGradient g(0, 0, w, h);
  g.setColorAt(0.0, QColor(40, 60, 160));
  g.setColorAt(0.5, QColor(160, 40, 120));
  g.setColorAt(1.0, QColor(40, 150, 90));
  p.fillRect(img.rect(), g);

  // Colour bars across the top.
  constexpr int n = 7;
  const QColor bars[n] = {Qt::white,   Qt::yellow, Qt::cyan,   Qt::green,
                          Qt::magenta, Qt::red,    Qt::blue};
  for (int i = 0; i < n; ++i) {
    p.fillRect(QRect(i * w / n, 0, w / n + 1, h / 4), bars[i]);
  }

  // A few bright shapes on the gradient for edge/detail contrast.
  p.setPen(Qt::NoPen);
  p.setBrush(QColor(255, 255, 255, 230));
  p.drawEllipse(QPoint(w / 2, h * 5 / 8), 34, 34);
  p.setBrush(QColor(20, 20, 20, 230));
  p.drawRoundedRect(QRect(w / 6, h * 3 / 4, w / 4, h / 6), 6, 6);

  // Thin high-contrast lines to reveal sampling/scanline behaviour.
  p.setPen(QPen(QColor(255, 255, 255), 1));
  for (int y = h / 2; y < h; y += 4) {
    p.drawLine(w * 3 / 5, y, w - 8, y);
  }
  p.end();
  return img;
}

class ShaderPreviewRenderer : public QQuickRhiItemRenderer {
protected:
  void initialize(QRhiCommandBuffer *) override {}

  void synchronize(QQuickRhiItem *item) override {
    auto *preview = dynamic_cast<ShaderPreviewItem *>(item);
    if (!preview) {
      return;
    }
    if (preview->revision() == m_revision) {
      return;
    }
    m_revision = preview->revision();
    if (preview->hasPreset()) {
      m_preset = preview->preset();
      m_params = preview->paramValues();
    } else {
      m_preset.reset();
      m_params.clear();
    }
    // Source frame: the item's resolved image (current game frame / screenshot),
    // or the generated sample when it has none.
    QImage src = preview->sourceImage();
    if (src.isNull()) {
      src = buildSampleImage();
    }
    m_pendingSource = src.convertToFormat(QImage::Format_RGBA8888);
    m_sourceDirty = true;
  }

  void render(QRhiCommandBuffer *cb) override {
    // (Re)upload the source frame when it changed.
    if (m_sourceDirty || !m_sampleTex) {
      QImage src = m_pendingSource;
      if (src.isNull()) {
        src = buildSampleImage().convertToFormat(QImage::Format_RGBA8888);
      }
      m_sampleSize = src.size();
      m_sampleTex.reset(
          rhi()->newTexture(QRhiTexture::RGBA8, m_sampleSize, 1,
                            QRhiTexture::UsedAsTransferSource));
      if (!m_sampleTex->create()) {
        m_sampleTex.reset();
      } else {
        QRhiResourceUpdateBatch *b = rhi()->nextResourceUpdateBatch();
        b->uploadTexture(m_sampleTex.get(), src);
        cb->resourceUpdate(b);
      }
      m_sourceDirty = false;
    }

    const QSize outputSize = colorTexture()->pixelSize();
    QString err;
    const bool ok =
        m_sampleTex && m_preset &&
        m_shaderChain.ensureBuilt(rhi(), renderTarget()->renderPassDescriptor(),
                                  *m_preset, m_params, m_sampleTex.get(),
                                  m_sampleSize, outputSize, &err);
    if (ok) {
      m_shaderChain.setParams(m_params);
      m_shaderChain.render(cb, renderTarget(), m_frame++);
    } else {
      // No (usable) shader — the picker hides the preview in this case, so just
      // clear to a neutral colour.
      cb->beginPass(renderTarget(), {0, 0, 0, 1}, {1.0f, 0}, nullptr);
      cb->endPass();
    }
  }

private:
  ShaderChain m_shaderChain;
  std::unique_ptr<QRhiTexture> m_sampleTex;
  QSize m_sampleSize;
  std::optional<ShaderPreset> m_preset;
  QVector<float> m_params;
  QImage m_pendingSource;
  bool m_sourceDirty = true;
  quint64 m_revision = 0;
  quint64 m_frame = 0;
};
} // namespace

ShaderPreviewItem::ShaderPreviewItem(QQuickItem *parent)
    : QQuickRhiItem(parent) {}

void ShaderPreviewItem::setShaderId(const QString &id) {
  if (m_shaderId == id) {
    return;
  }
  m_shaderId = id;
  emit shaderIdChanged();
  resolve();
}

void ShaderPreviewItem::setParams(const QString &params) {
  if (m_params == params) {
    return;
  }
  m_params = params;
  emit paramsChanged();
  resolve();
}

void ShaderPreviewItem::setSourceImageUrl(const QString &url) {
  if (m_sourceImageUrl == url) {
    return;
  }
  m_sourceImageUrl = url;
  emit sourceImageUrlChanged();
  loadSource();
}

void ShaderPreviewItem::loadSource() {
  m_sourceImage = QImage();
  const QString url = m_sourceImageUrl;
  if (!url.isEmpty()) {
    if (url.startsWith("image://")) {
      // image://<provider>/<id> — we only use the "gameimages" provider.
      const QString id = url.section('/', 3);
      if (auto *provider = getGameImageProvider(); provider && !id.isEmpty()) {
        QSize sz;
        const QImage img = provider->requestImage(id, &sz, QSize());
        if (!img.isNull()) {
          m_sourceImage = img;
        }
      }
    } else {
      const QString path =
          url.startsWith("file:") ? QUrl(url).toLocalFile() : url;
      const QImage img(path);
      if (!img.isNull()) {
        m_sourceImage = img;
      }
    }
  }
  ++m_revision;
  update();
}

void ShaderPreviewItem::resolve() {
  m_preset.reset();
  m_paramValues.clear();

  auto *library = getShaderLibrary();
  if (library && !m_shaderId.isEmpty()) {
    if (auto preset = library->presetById(m_shaderId)) {
      QJsonObject params;
      const auto doc = QJsonDocument::fromJson(m_params.toUtf8());
      if (doc.isObject()) {
        params = doc.object();
      }
      QVector<float> vals;
      vals.reserve(preset->parameters.size());
      for (const auto &p : preset->parameters) {
        float v = p.defaultValue;
        if (params.contains(p.key)) {
          v = static_cast<float>(params.value(p.key).toDouble(p.defaultValue));
        }
        vals.push_back(v);
      }
      m_preset = std::move(preset);
      m_paramValues = std::move(vals);
    }
  }

  ++m_revision;
  update();
}

QQuickRhiItemRenderer *ShaderPreviewItem::createRenderer() {
  return new ShaderPreviewRenderer();
}

} // namespace firelight::graphics
