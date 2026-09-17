// TODO: NEEDS REVIEW
#pragma once

#include "frame_presenter.hpp"

#include <cstdint>

struct ID3D11Device1;
struct ID3D11Texture2D;

/**
 * Shows the shared texture when the display is on D3D11: opens it on the display's device and wraps
 * it as an RHI texture, once per generation.
 *
 * Threading: render thread only
 */
class D3dPresenter final : public IFramePresenter {
public:
  D3dPresenter() = default;

  ~D3dPresenter() override;

  D3dPresenter(const D3dPresenter &) = delete;

  D3dPresenter &operator=(const D3dPresenter &) = delete;

  bool initialize(QRhi *rhi, void *windowHandle, VulkanHostHandles &host) override;

  bool show(const SharedFrame &frame, QRhi *rhi, QRhiResourceUpdateBatch *batch, QRhiTexture *target) override;

  bool showAgain(QRhiResourceUpdateBatch *batch, QRhiTexture *target) override;

  [[nodiscard]] bool hasShown() const override { return m_shownValue > 0; }

  void destroy() override;

private:
  bool openTexture(const SharedFrame &frame, QRhi *rhi);

  void releaseTexture();

  ID3D11Device1 *m_device = nullptr;
  ID3D11Texture2D *m_texture = nullptr;
  QRhiTexture *m_wrapper = nullptr;
  uint32_t m_importedGeneration = 0;
  uint64_t m_shownValue = 0;
};
