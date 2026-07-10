#pragma once

#include <QSize>
#include <functional>
#include <mutex>
#include <vector>

#include <libretro/libretro_vulkan.h>
#include <rhi/qrhi.h>
#ifdef _WIN32
#  include <windows.h>
#  include <vulkan/vulkan_win32.h>
#endif

#include "emulation/emulator_instance.hpp"

class EmulatorVulkanRenderer {
public:
  EmulatorVulkanRenderer() = default;
  ~EmulatorVulkanRenderer();

  // Called from render() on first Vulkan frame after the emulator is loaded.
  // negotiation must be non-null. resetCallback is called at the end of init.
  bool initialize(QRhi *rhi,
                  const retro_hw_render_context_negotiation_interface_vulkan *negotiation,
                  VkSurfaceKHR surface,
                  std::function<void()> resetCallback);

  // Safe to call multiple times; no-op if already destroyed.
  void destroy();

  // Runs the emulator frame(s), blits core → shared image.
  // targetSize must be colorTexture()->pixelSize() so the shared image matches.
  // When `nativeShared` is true the shared image is instead sized to the core's
  // render dimensions (native), leaving the display upscale to the shader chain
  // — used when a video shader is active so it samples an un-upscaled source.
  void renderFrame(firelight::emulation::EmulatorInstance *emulator,
                   float playbackMultiplier, QSize targetSize, QRhi *rhi,
                   bool nativeShared = false);

  // Called from receive() when data == RETRO_HW_FRAME_BUFFER_VALID.
  void setRenderDimensions(uint32_t w, uint32_t h);

  bool isInitialized() const { return m_vulkanInitialized; }
  bool isFirstFrameReady() const { return m_firstFrameReady; }
  uint32_t pendingWidth() const { return m_pendingColorBufferW; }
  uint32_t pendingHeight() const { return m_pendingColorBufferH; }
  QRhiTexture *sharedTexture() { return m_sharedTex; }
  VkImage qtSharedImage() const { return m_qtSharedImage; }
  uint64_t sharedSemValue() const { return m_sharedSemValue; }
  retro_hw_render_interface_vulkan *hwRenderInterface() { return &m_vkInterface; }

private:
  // ── Render dimensions (from video callback) ──────────────────────────────
  uint32_t m_pendingColorBufferW = 0;
  uint32_t m_pendingColorBufferH = 0;
  bool m_firstFrameReady = false;

  // ── Device & queue ────────────────────────────────────────────────────────
  VkInstance m_graniteInstance = VK_NULL_HANDLE;
  VkDevice m_vkDevice = VK_NULL_HANDLE;
  VkQueue m_vkQueue = VK_NULL_HANDLE;
  uint32_t m_vkQueueFamilyIndex = 0;
  PFN_vkGetDeviceProcAddr m_vkGetDeviceProcAddr = nullptr;
  void (*m_fnDestroyDevice)() = nullptr;

  // ── Physical device ───────────────────────────────────────────────────────
  VkPhysicalDeviceMemoryProperties m_vkPhysDevMemProps{};

  // ── Core image (set by set_image callback) ────────────────────────────────
  VkImage m_coreImage = VK_NULL_HANDLE;
  VkImageView m_coreImageView = VK_NULL_HANDLE;
  VkImageLayout m_coreImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkFormat m_coreImageFormat = VK_FORMAT_UNDEFINED;
  VkSemaphore m_coreSignalSem = VK_NULL_HANDLE;
  uint32_t m_vkRenderWidth = 0;
  uint32_t m_vkRenderHeight = 0;
  bool m_vkFrameReady = false;

  // ── Shared image (Granite → Qt device copy path; Win32 only) ─────────────
  VkImage m_sharedImage = VK_NULL_HANDLE;
  VkDeviceMemory m_sharedImageMem = VK_NULL_HANDLE;
  VkFormat m_sharedImageFmt = VK_FORMAT_UNDEFINED;
  uint32_t m_sharedImageW = 0;
  uint32_t m_sharedImageH = 0;
#ifdef _WIN32
  HANDLE m_sharedImageHandle = nullptr;
  HANDLE m_sharedSemHandle = nullptr;
#endif

  // ── Timeline semaphore (Granite → Qt sync) ────────────────────────────────
  VkSemaphore m_sharedSemaphore = VK_NULL_HANDLE;
  uint64_t m_sharedSemValue = 0;

  // ── Qt device resources ───────────────────────────────────────────────────
  VkDevice m_qtDevice = VK_NULL_HANDLE;
  VkQueue m_qtQueue = VK_NULL_HANDLE;
  VkImage m_qtSharedImage = VK_NULL_HANDLE;
  VkDeviceMemory m_qtSharedImageMem = VK_NULL_HANDLE;
  VkSemaphore m_qtImportedSem = VK_NULL_HANDLE;
  QRhiTexture *m_sharedTex = nullptr;

  // ── Per-frame resources ───────────────────────────────────────────────────
  std::vector<VkFence> m_vkFrameFences;
  VkCommandPool m_vkCmdPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> m_vkCmdBuffers;

  // ── Staging buffer (CPU readback fallback) ────────────────────────────────
  VkBuffer m_vkStagingBuffer = VK_NULL_HANDLE;
  VkDeviceMemory m_vkStagingMemory = VK_NULL_HANDLE;
  VkDeviceSize m_vkStagingSize = 0;

  // ── HW render interface returned to the core ──────────────────────────────
  retro_hw_render_interface_vulkan m_vkInterface{};
  bool m_vkInterfaceReady = false;
  bool m_vulkanInitialized = false;

  // ── Thread safety ─────────────────────────────────────────────────────────
  std::mutex m_vkQueueMutex;

  // ── Granite device function pointers ─────────────────────────────────────
  PFN_vkQueueSubmit m_vkfQueueSubmit = nullptr;
  PFN_vkCreateCommandPool m_vkfCreateCommandPool = nullptr;
  PFN_vkAllocateCommandBuffers m_vkfAllocCommandBuffers = nullptr;
  PFN_vkBeginCommandBuffer m_vkfBeginCommandBuffer = nullptr;
  PFN_vkEndCommandBuffer m_vkfEndCommandBuffer = nullptr;
  PFN_vkCmdPipelineBarrier m_vkfCmdPipelineBarrier = nullptr;
  PFN_vkCmdCopyImageToBuffer m_vkfCmdCopyImageToBuffer = nullptr;
  PFN_vkCmdCopyImage m_vkfCmdCopyImage = nullptr;
  PFN_vkCmdBlitImage m_vkfCmdBlitImage = nullptr;
  PFN_vkCreateFence m_vkfCreateFence = nullptr;
  PFN_vkDestroyFence m_vkfDestroyFence = nullptr;
  PFN_vkWaitForFences m_vkfWaitForFences = nullptr;
  PFN_vkResetFences m_vkfResetFences = nullptr;
  PFN_vkDestroyCommandPool m_vkfDestroyCommandPool = nullptr;
  PFN_vkDestroyDevice m_vkfDestroyDevice = nullptr;
  PFN_vkDestroyInstance m_vkfDestroyInstance = nullptr;
  PFN_vkDeviceWaitIdle m_vkfDeviceWaitIdle = nullptr;
  PFN_vkCreateBuffer m_vkfCreateBuffer = nullptr;
  PFN_vkDestroyBuffer m_vkfDestroyBuffer = nullptr;
  PFN_vkAllocateMemory m_vkfAllocateMemory = nullptr;
  PFN_vkFreeMemory m_vkfFreeMemory = nullptr;
  PFN_vkBindBufferMemory m_vkfBindBufferMemory = nullptr;
  PFN_vkGetBufferMemoryRequirements m_vkfGetBufferMemoryReqs = nullptr;
  PFN_vkMapMemory m_vkfMapMemory = nullptr;
  PFN_vkUnmapMemory m_vkfUnmapMemory = nullptr;
  PFN_vkCreateImage m_vkfCreateImage = nullptr;
  PFN_vkDestroyImage m_vkfDestroyImage = nullptr;
  PFN_vkBindImageMemory m_vkfBindImageMemory = nullptr;
  PFN_vkGetImageMemoryRequirements m_vkfGetImageMemReqs = nullptr;
  PFN_vkCreateSemaphore m_vkfCreateSemaphore = nullptr;
  PFN_vkDestroySemaphore m_vkfDestroySemaphore = nullptr;
#ifdef _WIN32
  PFN_vkGetMemoryWin32HandleKHR m_vkfGetMemWin32Handle = nullptr;
  PFN_vkGetSemaphoreWin32HandleKHR m_vkfGetSemWin32Handle = nullptr;
#endif

  // ── Qt device function pointers ───────────────────────────────────────────
  PFN_vkGetDeviceProcAddr m_qtGetDeviceProcAddr = nullptr;
  PFN_vkCreateImage m_qtfCreateImage = nullptr;
  PFN_vkDestroyImage m_qtfDestroyImage = nullptr;
  PFN_vkAllocateMemory m_qtfAllocateMemory = nullptr;
  PFN_vkFreeMemory m_qtfFreeMemory = nullptr;
  PFN_vkBindImageMemory m_qtfBindImageMemory = nullptr;
  PFN_vkGetImageMemoryRequirements m_qtfGetImageMemReqs = nullptr;
  PFN_vkCreateSemaphore m_qtfCreateSemaphore = nullptr;
  PFN_vkDestroySemaphore m_qtfDestroySemaphore = nullptr;
  PFN_vkQueueSubmit m_qtfQueueSubmit = nullptr;
#ifdef _WIN32
  PFN_vkImportSemaphoreWin32HandleKHR m_qtfImportSemWin32Handle = nullptr;
#endif

  void createVulkanPerFrameResources();
  void buildVulkanHwInterface(VkInstance instance, VkPhysicalDevice physDev,
                              PFN_vkGetInstanceProcAddr procAddr);
  bool ensureStagingBuffer();
  bool ensureSharedImage(QSize targetSize, QRhi *rhi);
  void destroySharedImage();
};
