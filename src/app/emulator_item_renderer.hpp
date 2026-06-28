#pragma once

#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQueue>
#include <QQuickRhiItemRenderer>
#include <QSGRenderNode>
#include <QVideoFrameInput>
#include <firelight/libretro/video_data_receiver.hpp>
#include <mutex>
#include <qsgrendererinterface.h>
#include <rhi/qrhi.h>

#include "audio/audio_manager.hpp"
#include "emulation/emulator_instance.hpp"
#include "libretro/core.hpp"
#include "libretro/core_configuration.hpp"
#include "manager_accessor.hpp"

#include <libretro/libretro_vulkan.h>
#include <qchronotimer.h>
#ifdef _WIN32
#  include <windows.h>
#  include <vulkan/vulkan_win32.h>
#endif

class EmulatorItem;

class EmulatorItemRenderer : public QQuickRhiItemRenderer,
                             public QOpenGLFunctions,
                             public firelight::libretro::IVideoDataReceiver,
                             public firelight::ManagerAccessor {
public:
  explicit EmulatorItemRenderer(
    QSGRendererInterface::GraphicsApi api, QWindow *window,
    firelight::emulation::EmulatorInstance *emulatorInstance);

  void setHwRenderInterface(retro_hw_render_callback *iface) override;

  void onGeometryChanged(
    const std::function<void(int, int, float, double)> &callback) {
    m_geometryChangedCallback = callback;
  }

  void receive(const void *data, unsigned width, unsigned height,
               size_t pitch) override;

  retro_hw_context_type getPreferredHwRender() override;

  void getHwRenderContext(retro_hw_context_type &contextType,
                          unsigned int &major, unsigned int &minor) override;

  proc_address_t getProcAddress(const char *sym) override;

  void setResetContextFunc(context_reset_func contextResetFunc) override;

  void setDestroyContextFunc(context_destroy_func contextDestroyFunc) override;

  uintptr_t getCurrentFramebufferId() override;

  void setSystemAVInfo(retro_system_av_info *info) override;

  void setPixelFormat(retro_pixel_format *format) override;

  void setScreenRotation(unsigned rotation) override;

  void setHwRenderContextNegotiationInterface(
    retro_hw_render_context_negotiation_interface *iface) override;

  void setHwRenderInterface(retro_hw_render_interface **iface) override;

  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;
  QString m_contentHash;
  int m_saveSlotNumber;
  int m_platformId;
  QString m_contentPath;
  bool m_gameReady;
  float m_playbackMultiplier = 1;

  enum EmulatorCommandType {
    WriteRewindPoint,
    EmitRewindPoints,
    LoadRewindPoint,
    WriteSuspendPoint,
    LoadSuspendPoint,
    UndoLoadSuspendPoint,
    SetPlaybackMultiplier,
    RunFrame
  };

  struct EmulatorCommand {
    EmulatorCommandType type;
    int suspendPointIndex;
    int rewindPointIndex;
    float playbackMultiplier;
  };

  void submitCommand(EmulatorCommand command);

protected:
  ~EmulatorItemRenderer() override;

  void initialize(QRhiCommandBuffer *cb) override;

  void synchronize(QQuickRhiItem *item) override;

  void render(QRhiCommandBuffer *cb) override;

private:
  QWindow *m_window = nullptr;
  EmulatorItem *m_emulatorItem = nullptr;
  const QSGRendererInterface::GraphicsApi m_graphicsApi;
  firelight::emulation::EmulatorInstance *m_emulatorInstance;

  QRhiResourceUpdateBatch *m_currentUpdateBatch = nullptr;
  QQueue<EmulatorCommand> m_commandQueue;

  QImage m_overlayImage;
  QImage m_currentImage;

  SuspendPoint m_beforeLastLoadSuspendPoint;
  QList<QString> m_rewindImageUrls{};

  QElapsedTimer m_playSessionTimer;
  firelight::activity::PlaySession m_playSession{};

  bool m_quitting = false;
  bool m_shouldRunFrame = true;

  QThread m_emulatorThread;
  QChronoTimer m_emulatorTimer{};

  QElapsedTimer m_renderCallTimer{};
  QList<int64_t> m_renderCallTimes;
  int64_t m_averageTimeBetweenRenderCalls = 0;

  bool m_measureTime = false;
  QThread m_emulationTimerThread{};
  QList<int64_t> m_emulationWorkTimeBuffer{};
  int64_t m_averageEmulationTime = 0;

  int m_frameNumber = 0;
  int m_currentWaitFrames = 0;
  int m_waitFrames = 0;

  QList<SuspendPoint> m_rewindPoints;

  std::function<void(int, int, float, double)> m_geometryChangedCallback = nullptr;

  // ── HW render callbacks (all APIs) ──────────────────────────────────────
  std::function<void()> m_resetContextFunction = nullptr;
  std::function<void()> m_destroyContextFunction = nullptr;

  // ── OpenGL ──────────────────────────────────────────────────────────────
  bool m_openGlInitialized = false;
  GLint m_currentFramebufferId = 0;

  // ── Common display state ─────────────────────────────────────────────────
  QImage::Format m_pixelFormat = QImage::Format_RGB16;
  unsigned m_screenRotation = 0;
  bool m_paused = false;
  bool m_shouldSave = false;

  uint m_coreBaseWidth = 0;
  uint m_coreBaseHeight = 0;
  uint m_coreMaxWidth = 0;
  uint m_coreMaxHeight = 0;
  float m_coreAspectRatio = 0.0f;
  float m_calculatedAspectRatio = 0.0f;

  bool m_usingHardwareRenderer = false;

  // ── Vulkan ───────────────────────────────────────────────────────────────
  const retro_hw_render_context_negotiation_interface_vulkan *m_negotiation = nullptr;

  void (*m_fnDestroyDevice)() = nullptr; // stored from m_negotiation before DLL can unload
  retro_hw_render_interface_vulkan m_vkInterface{};
  bool m_vkInterfaceReady = false;
  bool m_vulkanInitialized = false;

  // Granite's VkInstance — created by create_instance, owned and destroyed by us
  VkInstance m_graniteInstance = VK_NULL_HANDLE;

  // Device / queue — owned by us after create_device
  VkDevice m_vkDevice = VK_NULL_HANDLE;
  VkQueue m_vkQueue = VK_NULL_HANDLE;
  uint32_t m_vkQueueFamilyIndex = 0;
  PFN_vkGetDeviceProcAddr m_vkGetDeviceProcAddr = nullptr;

  // Physical device memory properties — queried once for staging buffer allocation
  VkPhysicalDeviceMemoryProperties m_vkPhysDevMemProps{};

  // Single fence for frame sync (pre-signaled; wait_sync_index waits+resets it,
  // our readback submission re-signals it)
  std::vector<VkFence> m_vkFrameFences; // size 1

  // Command pool / buffer for the GPU→CPU readback blit each frame
  VkCommandPool m_vkCmdPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> m_vkCmdBuffers; // size 1

  // Staging buffer: Granite's VkImage is copied here each frame, then mapped to CPU
  VkBuffer m_vkStagingBuffer = VK_NULL_HANDLE;
  VkDeviceMemory m_vkStagingMemory = VK_NULL_HANDLE;
  VkDeviceSize m_vkStagingSize = 0;

  // Core's rendered image — populated by set_image() each frame
  VkImage m_coreImage = VK_NULL_HANDLE;
  VkImageView m_coreImageView = VK_NULL_HANDLE;
  VkImageLayout m_coreImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkFormat m_coreImageFormat = VK_FORMAT_UNDEFINED;
  VkSemaphore m_coreSignalSem = VK_NULL_HANDLE;
  uint32_t m_vkRenderWidth = 0;
  uint32_t m_vkRenderHeight = 0;
  bool m_vkFrameReady = false;

  // Shared image: Granite copies here each frame; Qt device samples via imported memory
  VkImage m_sharedImage = VK_NULL_HANDLE;
  VkDeviceMemory m_sharedImageMem = VK_NULL_HANDLE;
  VkFormat m_sharedImageFmt = VK_FORMAT_UNDEFINED;
  uint32_t m_sharedImageW = 0;
  uint32_t m_sharedImageH = 0;
#ifdef _WIN32
  HANDLE m_sharedImageHandle = nullptr;
  HANDLE m_sharedSemHandle = nullptr;
#endif

  // Timeline semaphore: Granite signals after each frame copy, Qt waits before compositing
  VkSemaphore m_sharedSemaphore = VK_NULL_HANDLE;
  uint64_t m_sharedSemValue = 0;

  // Qt-device resources (imported Granite shared memory)
  VkDevice m_qtDevice = VK_NULL_HANDLE;
  VkQueue m_qtQueue = VK_NULL_HANDLE;
  VkImage m_qtSharedImage = VK_NULL_HANDLE;
  VkDeviceMemory m_qtSharedImageMem = VK_NULL_HANDLE;
  VkSemaphore m_qtImportedSem = VK_NULL_HANDLE;
  QRhiTexture *m_sharedTex = nullptr;

  // Protects vkQueueSubmit against Granite's internal submission threads
  std::mutex m_vkQueueMutex;

  // Granite device-level function pointers
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

  // Qt device function pointers (loaded separately from Qt's VkDevice)
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

  void destroyHwContext() override {
    if (m_graphicsApi == QSGRendererInterface::Vulkan)
      destroyVulkan();
  }

  bool initVulkan();

  void createVulkanPerFrameResources();

  void buildVulkanHwInterface(VkInstance instance, VkPhysicalDevice physDev,
                              PFN_vkGetInstanceProcAddr procAddr);

  void renderVulkanFrame();

  bool ensureStagingBuffer();

  bool ensureSharedImage();

  void destroySharedImage();

  void destroyVulkan();
};
