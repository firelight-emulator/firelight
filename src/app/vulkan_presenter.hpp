// TODO: NEEDS REVIEW
#pragma once

#include "frame_presenter.hpp"

#include <cstdint>

/**
 * Shows the shared texture when the display is on Vulkan: imports it into an image on the display's
 * device and wraps that as an RHI texture, once per generation.
 *
 * Threading: render thread only
 */
class VulkanPresenter final : public IFramePresenter {
public:
  VulkanPresenter() = default;

  ~VulkanPresenter() override;

  VulkanPresenter(const VulkanPresenter &) = delete;

  VulkanPresenter &operator=(const VulkanPresenter &) = delete;

  bool initialize(QRhi *rhi, void *windowHandle, VulkanHostHandles &host) override;

  bool show(const SharedFrame &frame, QRhi *rhi, QRhiResourceUpdateBatch *batch, QRhiTexture *target) override;

  bool showAgain(QRhiResourceUpdateBatch *batch, QRhiTexture *target) override;

  [[nodiscard]] bool hasShown() const override { return m_shownValue > 0; }

  void destroy() override;

private:
  bool importImage(const SharedFrame &frame, QRhi *rhi);

  void releaseImage();

  VkDevice m_device = VK_NULL_HANDLE;

  VkImage m_image = VK_NULL_HANDLE;
  VkDeviceMemory m_memory = VK_NULL_HANDLE;
  QRhiTexture *m_texture = nullptr;
  uint32_t m_importedGeneration = 0;
  uint64_t m_shownValue = 0;

  PFN_vkGetDeviceProcAddr m_getDeviceProcAddr = nullptr;
  PFN_vkCreateImage m_fnCreateImage = nullptr;
  PFN_vkDestroyImage m_fnDestroyImage = nullptr;
  PFN_vkAllocateMemory m_fnAllocateMemory = nullptr;
  PFN_vkFreeMemory m_fnFreeMemory = nullptr;
  PFN_vkBindImageMemory m_fnBindImageMemory = nullptr;
  PFN_vkGetImageMemoryRequirements m_fnGetImageMemoryRequirements = nullptr;
#ifdef _WIN32
  PFN_vkGetMemoryWin32HandlePropertiesKHR m_fnGetMemoryWin32HandleProperties = nullptr;
#endif
};
