// TODO: NEEDS REVIEW
#pragma once

#include "vulkan_core_context.hpp"

#include <memory>
#include <qsgrendererinterface.h>
#include <rhi/qrhi.h>

/**
 * Shows a hardware core's shared picture on the display's own device. One per graphics backend.
 *
 * Threading: render thread only
 */
class IFramePresenter {
public:
  virtual ~IFramePresenter() = default;

  /**
   * Takes what it needs from the RHI and fills what the core's device needs to know about the display
   */
  virtual bool initialize(QRhi *rhi, void *windowHandle, VulkanHostHandles &host) = 0;

  /**
   * Copies frame onto target when it is newer than what was last shown
   * @return Whether anything was copied
   */
  virtual bool show(const SharedFrame &frame, QRhi *rhi, QRhiResourceUpdateBatch *batch, QRhiTexture *target) = 0;

  /**
   * Copies the picture last shown onto target again
   * @return Whether there was one to copy
   */
  virtual bool showAgain(QRhiResourceUpdateBatch *batch, QRhiTexture *target) = 0;

  [[nodiscard]] virtual bool hasShown() const = 0;

  /**
   * Releases everything imported. Safe to call more than once
   */
  virtual void destroy() = 0;
};

/**
 * The presenter for the backend the window is on, or null when that backend cannot show a shared picture
 */
std::unique_ptr<IFramePresenter> makeFramePresenter(QSGRendererInterface::GraphicsApi api);
