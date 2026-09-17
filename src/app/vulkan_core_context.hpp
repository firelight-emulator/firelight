// TODO: NEEDS REVIEW
#pragma once

#include <firelight/monitoring/monitor.hpp>

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <libretro/libretro_vulkan.h>
#include <mutex>
#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dxgiformat.h>
#include <vulkan/vulkan_win32.h>
// clang-format on
struct ID3D11Device;
struct ID3D11Texture2D;
#endif

/**
 * What the display tells a core's private Vulkan device: which GPU it is on and the window it draws to
 */
struct VulkanHostHandles {
  /** The adapter LUID, 0 when unknown */
  uint64_t adapterLuid = 0;

  /** The native window handle, for the surface a core may query */
  void *windowHandle = nullptr;
};

/**
 * The newest picture the core handed over, as a shared texture any device on the same GPU can open
 */
struct SharedFrame {
  /** Counts the pictures published; 0 before the first */
  uint64_t value = 0;

  /** Counts the shared textures made, so an importer knows when the one it holds is stale */
  uint32_t generation = 0;

  uint32_t width = 0;
  uint32_t height = 0;
#ifdef _WIN32
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

  /** NT handle of the shared texture. Owned by the context; importers open it without closing it */
  HANDLE handle = nullptr;
#endif
};

/**
 * The Vulkan instance and device a core renders on, made through its negotiation interface on the
 * display's GPU, and the copy of each finished picture into a shared texture.
 *
 * Threading: initialize(), blitLatest() and the callbacks the core makes run on the
 * thread that runs frames. getLatestFrame() and the pending size may be read from any thread
 */
class VulkanCoreContext {
public:
  /** How many pictures may be in flight before a slot is reused */
  static constexpr uint32_t FRAMES_IN_FLIGHT = 3;

  VulkanCoreContext() = default;

  ~VulkanCoreContext();

  VulkanCoreContext(const VulkanCoreContext &) = delete;

  VulkanCoreContext &operator=(const VulkanCoreContext &) = delete;

  /**
   * Creates the instance and, through the core, the device, builds the interface the core renders
   * with, and calls resetCallback so the core finishes its own setup
   */
  bool initialize(const VulkanHostHandles &host,
                  const retro_hw_render_context_negotiation_interface_vulkan *negotiation,
                  const std::function<void()> &resetCallback);

  /**
   * Tears everything down. Safe to call more than once
   */
  void destroy();

  [[nodiscard]] bool isInitialized() const { return m_initialized; }

  /**
   * The size the core says it rendered at, from its video callback
   */
  void setRenderDimensions(uint32_t width, uint32_t height);

  [[nodiscard]] uint32_t pendingWidth() const { return m_pendingWidth.load(std::memory_order_acquire); }

  [[nodiscard]] uint32_t pendingHeight() const { return m_pendingHeight.load(std::memory_order_acquire); }

  /**
   * Copies the picture the core handed over into the shared texture and publishes it
   * @return Whether there was a picture to copy
   */
  bool blitLatest();

  [[nodiscard]] SharedFrame getLatestFrame() const;

  retro_hw_render_interface_vulkan *hwRenderInterface() { return &m_interface; }

private:
  /**
   * The private instance, with the app info the core asks for
   */
  bool createInstance(const retro_hw_render_context_negotiation_interface_vulkan *negotiation);

  /**
   * The physical device with the display's adapter LUID, or the first when the LUID is unknown
   */
  bool pickPhysicalDevice();

  /**
   * Fences and command buffers, one set per frame in flight
   */
  void createPerFrameResources();

  void buildInterface();

  /**
   * The D3D11 device the shared texture is allocated on, made on first use
   */
  bool ensureAllocationDevice();

  /**
   * Makes the shared texture match the core's picture, remaking it when the size or format changed
   */
  bool ensureSharedImage();

  void destroySharedImage();

  /**
   * Waits for the copy that last used a slot
   */
  void waitSlot(uint32_t slot);

  bool m_initialized = false;

  std::atomic<uint32_t> m_pendingWidth{0};
  std::atomic<uint32_t> m_pendingHeight{0};

  uint64_t m_adapterLuid = 0;
  void *m_vulkanLibrary = nullptr;
  PFN_vkGetInstanceProcAddr m_getInstanceProcAddr = nullptr;
  VkInstance m_instance = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
  uint32_t m_queueFamilyIndex = 0;
  PFN_vkGetDeviceProcAddr m_getDeviceProcAddr = nullptr;
  void (*m_destroyDeviceThroughCore)() = nullptr;
  VkPhysicalDeviceMemoryProperties m_memoryProperties{};

  // What set_image handed over this frame
  VkImage m_coreImage = VK_NULL_HANDLE;
  VkImageLayout m_coreImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkFormat m_coreImageFormat = VK_FORMAT_UNDEFINED;
  VkSemaphore m_coreSignalSemaphore = VK_NULL_HANDLE;
  uint32_t m_renderWidth = 0;
  uint32_t m_renderHeight = 0;
  bool m_frameReady = false;

  VkImage m_sharedImage = VK_NULL_HANDLE;
  VkDeviceMemory m_sharedImageMemory = VK_NULL_HANDLE;
#ifdef _WIN32
  ID3D11Device *m_allocationDevice = nullptr;
  ID3D11Texture2D *m_sharedTexture = nullptr;
#endif

  mutable std::mutex m_latestMutex;
  SharedFrame m_latest;

  std::array<VkFence, FRAMES_IN_FLIGHT> m_fences{};
  std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> m_commandBuffers{};
  VkCommandPool m_commandPool = VK_NULL_HANDLE;
  uint32_t m_syncIndex = 0;

  retro_hw_render_interface_vulkan m_interface{};
  std::mutex m_queueMutex;

  PFN_vkDestroyInstance m_fnDestroyInstance = nullptr;
  PFN_vkDestroySurfaceKHR m_fnDestroySurface = nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties2 m_fnGetPhysicalDeviceImageFormatProperties2 = nullptr;
  PFN_vkQueueSubmit m_fnQueueSubmit = nullptr;
  PFN_vkCreateCommandPool m_fnCreateCommandPool = nullptr;
  PFN_vkAllocateCommandBuffers m_fnAllocateCommandBuffers = nullptr;
  PFN_vkBeginCommandBuffer m_fnBeginCommandBuffer = nullptr;
  PFN_vkEndCommandBuffer m_fnEndCommandBuffer = nullptr;
  PFN_vkCmdPipelineBarrier m_fnCmdPipelineBarrier = nullptr;
  PFN_vkCmdBlitImage m_fnCmdBlitImage = nullptr;
  PFN_vkCreateFence m_fnCreateFence = nullptr;
  PFN_vkDestroyFence m_fnDestroyFence = nullptr;
  PFN_vkWaitForFences m_fnWaitForFences = nullptr;
  PFN_vkResetFences m_fnResetFences = nullptr;
  PFN_vkDestroyCommandPool m_fnDestroyCommandPool = nullptr;
  PFN_vkDestroyDevice m_fnDestroyDevice = nullptr;
  PFN_vkDeviceWaitIdle m_fnDeviceWaitIdle = nullptr;
  PFN_vkAllocateMemory m_fnAllocateMemory = nullptr;
  PFN_vkFreeMemory m_fnFreeMemory = nullptr;
  PFN_vkCreateImage m_fnCreateImage = nullptr;
  PFN_vkDestroyImage m_fnDestroyImage = nullptr;
  PFN_vkBindImageMemory m_fnBindImageMemory = nullptr;
  PFN_vkGetImageMemoryRequirements m_fnGetImageMemoryRequirements = nullptr;
#ifdef _WIN32
  PFN_vkGetMemoryWin32HandlePropertiesKHR m_fnGetMemoryWin32HandleProperties = nullptr;
#endif

  firelight::monitoring::Span m_blitWaitPrevSpan = firelight::monitoring::Monitor::instance().span(
      "blit_wait_prev", "Waiting for the copy that last used the slot before its command buffer is reused");
  firelight::monitoring::Span m_blitSubmitSpan =
      firelight::monitoring::Monitor::instance().span("blit_submit", "Recording and submitting the copy");
  firelight::monitoring::Span m_blitWaitDoneSpan = firelight::monitoring::Monitor::instance().span(
      "blit_wait_done", "Waiting for the copy, and the core's picture behind it, to finish on the GPU");
};
