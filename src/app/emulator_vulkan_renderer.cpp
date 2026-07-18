#include "emulator_vulkan_renderer.hpp"

#ifdef _WIN32
#  include <windows.h>
#  include <vulkan/vulkan_win32.h>
#endif
#include <cstring>
#include <spdlog/spdlog.h>
#include <vector>

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

EmulatorVulkanRenderer::~EmulatorVulkanRenderer() {
  destroy();
}

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Public interface
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorVulkanRenderer::setRenderDimensions(uint32_t w, uint32_t h) {
  if (w >= 2 && h >= 2) {
    m_pendingColorBufferW = w;
    m_pendingColorBufferH = h;
  }
  m_vkRenderWidth = w;
  m_vkRenderHeight = h;
  m_vkFrameReady = true;
}

void EmulatorVulkanRenderer::renderFrame(
  firelight::emulation::EmulatorInstance *emulator, float playbackMultiplier,
  QSize targetSize, QRhi *rhi) {
  m_coreSignalSem = VK_NULL_HANDLE;
  m_vkFrameReady = false;

  if (playbackMultiplier > 1) {
    for (int i = 0; i < static_cast<int>(playbackMultiplier); i++)
      emulator->runFrame();
  } else {
    emulator->runFrame();
  }

  if (!m_vkFrameReady || m_coreImage == VK_NULL_HANDLE) {
    spdlog::debug("renderFrame: early exit – frameReady={} coreImage={}",
                  m_vkFrameReady, m_coreImage != VK_NULL_HANDLE);
    return;
  }

  if (m_vkRenderWidth < 2 || m_vkRenderHeight < 2) {
    spdlog::debug("renderFrame: degenerate dims {}x{}, skipping blit",
                  m_vkRenderWidth, m_vkRenderHeight);
    return;
  }

  if (!ensureSharedImage(targetSize, rhi)) {
    spdlog::debug("renderFrame: ensureSharedImage() failed");
    return;
  }

  m_firstFrameReady = true;

  spdlog::debug("renderFrame: blitting {}x{} -> {}x{} fmt={}",
                m_vkRenderWidth, m_vkRenderHeight, m_sharedImageW, m_sharedImageH,
                static_cast<int>(m_coreImageFormat));

  // Wait for the previous frame's copy command buffer to finish before we
  // record into it again, then reset for re-use. (Fence is pre-signaled on first call.)
  m_vkfWaitForFences(m_vkDevice, 1, &m_vkFrameFences[0], VK_TRUE, UINT64_MAX);
  m_vkfResetFences(m_vkDevice, 1, &m_vkFrameFences[0]);

  VkCommandBuffer cmd = m_vkCmdBuffers[0];
  VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  m_vkfBeginCommandBuffer(cmd, &bi);

  // Transition core image SHADER_READ_ONLY → TRANSFER_SRC
  // Transition shared image UNDEFINED → TRANSFER_DST (discard previous contents)
  VkImageMemoryBarrier preCopy[2]{};
  preCopy[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  preCopy[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  preCopy[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  preCopy[0].oldLayout = m_coreImageLayout;
  preCopy[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  preCopy[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[0].image = m_coreImage;
  preCopy[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  preCopy[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  preCopy[1].srcAccessMask = 0;
  preCopy[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  preCopy[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  preCopy[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  preCopy[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  preCopy[1].image = m_sharedImage;
  preCopy[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  m_vkfCmdPipelineBarrier(cmd,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                          0, 0, nullptr, 0, nullptr, 2, preCopy);

  // GPU blit: core image (render res) → shared image (display res), with linear scaling
  VkImageBlit blitRgn{};
  blitRgn.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blitRgn.srcOffsets[0] = {0, 0, 0};
  blitRgn.srcOffsets[1] = {static_cast<int32_t>(m_vkRenderWidth), static_cast<int32_t>(m_vkRenderHeight), 1};
  blitRgn.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  blitRgn.dstOffsets[0] = {0, 0, 0};
  blitRgn.dstOffsets[1] = {static_cast<int32_t>(m_sharedImageW), static_cast<int32_t>(m_sharedImageH), 1};
  m_vkfCmdBlitImage(cmd,
                    m_coreImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_sharedImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blitRgn, VK_FILTER_LINEAR);

  // Transition core image back to SHADER_READ_ONLY
  // Transition shared image TRANSFER_DST → SHADER_READ_ONLY so Qt can sample it
  VkImageMemoryBarrier postCopy[2]{};
  postCopy[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  postCopy[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  postCopy[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  postCopy[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  postCopy[0].newLayout = m_coreImageLayout;
  postCopy[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[0].image = m_coreImage;
  postCopy[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  postCopy[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  postCopy[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  postCopy[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  postCopy[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  postCopy[1].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  postCopy[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  postCopy[1].image = m_sharedImage;
  postCopy[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  m_vkfCmdPipelineBarrier(cmd,
                          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          0, 0, nullptr, 0, nullptr, 2, postCopy);

  m_vkfEndCommandBuffer(cmd);

  m_sharedSemValue++;

  uint64_t waitVal = 0;
  uint64_t signalVal = m_sharedSemValue;

  VkTimelineSemaphoreSubmitInfo tsInfo{VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO};
  tsInfo.signalSemaphoreValueCount = 1;
  tsInfo.pSignalSemaphoreValues = &signalVal;

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  si.pNext = &tsInfo;
  si.commandBufferCount = 1;
  si.pCommandBuffers = &cmd;
  si.signalSemaphoreCount = 1;
  si.pSignalSemaphores = &m_sharedSemaphore;

  if (m_coreSignalSem != VK_NULL_HANDLE) {
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &m_coreSignalSem;
    si.pWaitDstStageMask = &waitStage;
    tsInfo.waitSemaphoreValueCount = 1;
    tsInfo.pWaitSemaphoreValues = &waitVal;
  } {
    std::lock_guard lock(m_vkQueueMutex);
    m_vkfQueueSubmit(m_vkQueue, 1, &si, m_vkFrameFences[0]);
  }
  // CPU-sync: wait for the blit to finish so Qt can safely read m_sharedImage
  // without needing a cross-device semaphore import
  m_vkfWaitForFences(m_vkDevice, 1, &m_vkFrameFences[0], VK_TRUE, UINT64_MAX);
}

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

bool EmulatorVulkanRenderer::initialize(
  QRhi *rhi,
  const retro_hw_render_context_negotiation_interface_vulkan *negotiation,
  VkSurfaceKHR surface,
  std::function<void()> resetCallback) {
  const auto *vk =
      static_cast<const QRhiVulkanNativeHandles *>(rhi->nativeHandles());

  // TODO
  // Get the raw vkGetInstanceProcAddr directly from the Vulkan loader DLL
  //
  // Qt's QVulkanInstance::getInstanceProcAddr("vkGetInstanceProcAddr") returns
  // the trampoline that routes through Qt's layer chain (Steam overlay, OBS,
  // etc.).  When Granite creates its own separate VkInstance and then calls
  // volkLoadInstance(granite_instance), volk uses this proc addr to resolve
  // every function pointer.  Qt's layer-chained trampoline may not recognise
  // granite_instance (because the overlay layer was never attached to it) and
  // returns null / crashes for physical-device functions such as
  // vkGetPhysicalDeviceMemoryProperties
  //
  // The raw loader export (from vulkan-1.dll) correctly dispatches for ANY
  // valid instance, bypassing the Qt-specific layer chain entirely
  PFN_vkGetInstanceProcAddr globalProcAddr = nullptr;
#ifdef _WIN32
  if (HMODULE vulkanLib = GetModuleHandleA("vulkan-1.dll")) {
    globalProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      GetProcAddress(vulkanLib, "vkGetInstanceProcAddr"));
  }
#endif
  if (!globalProcAddr) {
    globalProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      vk->inst->getInstanceProcAddr("vkGetInstanceProcAddr"));
  }
  if (!globalProcAddr) {
    spdlog::error("EmulatorVulkanRenderer: could not obtain vkGetInstanceProcAddr");
    return false;
  }

  // TODO
  // ── Create the VkDevice ───────────────────────────────────────────────────
  //
  // Pass Qt's VkInstance so Granite uses it directly rather than creating its
  // own. This keeps the instance consistent between Granite's internal state
  // and the m_vkInterface.instance we hand back to the core — they must match
  // GPU is left as VK_NULL_HANDLE so Granite still picks the best device
  //
  // VkPhysicalDeviceFeatures must not be nullptr: parallel_create_device
  // SIMD-copies the features struct without a null guard, causing a crash
  retro_vulkan_context ctx{};
  VkPhysicalDeviceFeatures features{};

  // TODO
  // Disable implicit Vulkan layers before Granite calls vkCreateInstance
  // Steam/OBS/Overwolf overlay layers crash or return null function pointers
  // when invoked for a second VkInstance in the same process (they already
  // initialised for Qt's instance and don't handle the second one)
  // VK_LOADER_LAYERS_DISABLE=~implicit~ is honoured by the Vulkan Loader
  std::string savedLayerDisable; {
    const char *v = getenv("VK_LOADER_LAYERS_DISABLE");
    if (v) savedLayerDisable = v;
  }
  qputenv("VK_LOADER_LAYERS_DISABLE", "~implicit~");

  // TODO
  // Device extensions the core must enable so the shared-image path works. The
  // core creates the VkDevice, and cores that only enable what they need (PPSSPP)
  // leave vkGetMemoryWin32HandleKHR / vkGetSemaphoreWin32HandleKHR null, crashing
  // ensureSharedImage. (parallel-RDP/Granite enables these itself.) Filter to
  // what the physical device actually advertises so create_device can't fail
  // with VK_ERROR_EXTENSION_NOT_PRESENT
  std::vector<const char *> requiredDeviceExts;
#ifdef _WIN32
  {
    static const char *const CANDIDATES[] = {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    };
    auto enumDevExt =
        reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(globalProcAddr(
            vk->inst->vkInstance(), "vkEnumerateDeviceExtensionProperties"));
    if (enumDevExt) {
      uint32_t count = 0;
      enumDevExt(vk->physDev, nullptr, &count, nullptr);
      std::vector<VkExtensionProperties> props(count);
      if (count)
        enumDevExt(vk->physDev, nullptr, &count, props.data());
      for (const char *cand : CANDIDATES) {
        for (const auto &p : props) {
          if (std::strcmp(p.extensionName, cand) == 0) {
            requiredDeviceExts.push_back(cand);
            break;
          }
        }
      }
    }
  }
#endif

  // TODO
  // Hand the core Qt's physical device (not VK_NULL_HANDLE): per the libretro
  // Vulkan spec the core must then use exactly this VkPhysicalDevice, which is
  // required for the shared-image path (core + Qt must be on the same GPU). The
  // surface must also be a real VkSurfaceKHR — PPSSPP asserts it and queries
  // swapchain capabilities from it; VK_NULL_HANDLE crashes it (parallel-RDP
  // ignores it)
  const bool deviceOk = negotiation->create_device(
    &ctx, vk->inst->vkInstance(), vk->physDev, surface, globalProcAddr,
    requiredDeviceExts.empty() ? nullptr : requiredDeviceExts.data(),
    static_cast<unsigned>(requiredDeviceExts.size()), nullptr, 0, &features);

  if (savedLayerDisable.empty())
    qunsetenv("VK_LOADER_LAYERS_DISABLE");
  else
    qputenv("VK_LOADER_LAYERS_DISABLE", savedLayerDisable.c_str());

  if (!deviceOk) {
    spdlog::error("EmulatorVulkanRenderer: create_device failed");
    return false;
  }

  // TODO
  // Store destroy_device as a bare function pointer now, before the core DLL could
  // be unloaded. negotiation itself becomes a dangling pointer after coreLib->unload(),
  // so destroy() must call m_fnDestroyDevice directly
  m_fnDestroyDevice = negotiation->destroy_device;

  m_vkDevice = ctx.device;
  m_vkQueue = ctx.queue;
  m_vkQueueFamilyIndex = ctx.queue_family_index;

  m_vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
    globalProcAddr(vk->inst->vkInstance(), "vkGetDeviceProcAddr"));

  auto d = [&](const char *name) {
    return m_vkGetDeviceProcAddr(m_vkDevice, name);
  };
  m_vkfQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(d("vkQueueSubmit"));
  m_vkfCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(d("vkCreateCommandPool"));
  m_vkfAllocCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(d("vkAllocateCommandBuffers"));
  m_vkfBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(d("vkBeginCommandBuffer"));
  m_vkfEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(d("vkEndCommandBuffer"));
  m_vkfCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(d("vkCmdPipelineBarrier"));
  m_vkfCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(d("vkCmdCopyImageToBuffer"));
  m_vkfCreateFence = reinterpret_cast<PFN_vkCreateFence>(d("vkCreateFence"));
  m_vkfDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(d("vkDestroyFence"));
  m_vkfWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(d("vkWaitForFences"));
  m_vkfResetFences = reinterpret_cast<PFN_vkResetFences>(d("vkResetFences"));
  m_vkfDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(d("vkDestroyCommandPool"));
  m_vkfDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(d("vkDestroyDevice"));
  m_vkfDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(d("vkDeviceWaitIdle"));
  m_vkfCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(d("vkCreateBuffer"));
  m_vkfDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(d("vkDestroyBuffer"));
  m_vkfAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(d("vkAllocateMemory"));
  m_vkfFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(d("vkFreeMemory"));
  m_vkfBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(d("vkBindBufferMemory"));
  m_vkfGetBufferMemoryReqs = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(d("vkGetBufferMemoryRequirements"));
  m_vkfMapMemory = reinterpret_cast<PFN_vkMapMemory>(d("vkMapMemory"));
  m_vkfUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(d("vkUnmapMemory"));
  m_vkfCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(d("vkCmdCopyImage"));
  m_vkfCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(d("vkCmdBlitImage"));
  m_vkfCreateImage = reinterpret_cast<PFN_vkCreateImage>(d("vkCreateImage"));
  m_vkfDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(d("vkDestroyImage"));
  m_vkfBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(d("vkBindImageMemory"));
  m_vkfGetImageMemReqs = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(d("vkGetImageMemoryRequirements"));
  m_vkfCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(d("vkCreateSemaphore"));
  m_vkfDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(d("vkDestroySemaphore"));
#ifdef _WIN32
  m_vkfGetMemWin32Handle = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>(d("vkGetMemoryWin32HandleKHR"));
  m_vkfGetSemWin32Handle = reinterpret_cast<PFN_vkGetSemaphoreWin32HandleKHR>(d("vkGetSemaphoreWin32HandleKHR"));
#endif

  // Physical device memory properties — needed by ensureStagingBuffer()
  auto vkGetPhysDevMemProps =
      reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(
        globalProcAddr(vk->inst->vkInstance(), "vkGetPhysicalDeviceMemoryProperties"));
  vkGetPhysDevMemProps(ctx.gpu, &m_vkPhysDevMemProps);

#ifdef _WIN32
  // ── Create Granite-side timeline semaphore (exported for Qt device import) ──
  spdlog::info("EmulatorVulkanRenderer: setting up shared-image path");
  if (!m_vkfCreateSemaphore || !m_vkfGetSemWin32Handle) {
    spdlog::warn("  Granite missing vkCreateSemaphore/vkGetSemaphoreWin32HandleKHR — CPU readback");
  } else {
    VkSemaphoreTypeCreateInfo semType{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
    semType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semType.initialValue = 0;

    VkExportSemaphoreCreateInfo exportSem{VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO};
    exportSem.pNext = &semType;
    exportSem.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    sci.pNext = &exportSem;

    spdlog::info("  creating Granite-side timeline semaphore");
    VkResult res = m_vkfCreateSemaphore(m_vkDevice, &sci, nullptr, &m_sharedSemaphore);
    if (res != VK_SUCCESS) {
      spdlog::warn("  vkCreateSemaphore failed ({}) — CPU readback", int(res));
    } else {
      VkSemaphoreGetWin32HandleInfoKHR getHandle{VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR};
      getHandle.semaphore = m_sharedSemaphore;
      getHandle.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
      spdlog::info("  exporting Win32 handle");
      if (m_vkfGetSemWin32Handle(m_vkDevice, &getHandle, &m_sharedSemHandle) != VK_SUCCESS) {
        spdlog::warn("  vkGetSemaphoreWin32HandleKHR failed — CPU readback");
        m_vkfDestroySemaphore(m_vkDevice, m_sharedSemaphore, nullptr);
        m_sharedSemaphore = VK_NULL_HANDLE;
      } else {
        spdlog::info("  Granite semaphore exported OK");
      }
    }
  }

  // ── Get Qt's VkDevice, load its function pointers, import the semaphore ───
  if (m_sharedSemHandle) {
    spdlog::info("  loading Qt device functions");
    const auto *qtNative = static_cast<const QRhiVulkanNativeHandles *>(rhi->nativeHandles());
    m_qtDevice = qtNative->dev;
    spdlog::info("  Qt VkDevice = {:p}", static_cast<void *>(m_qtDevice));

    bool sharedOk = false;
    do {
      HMODULE vulkanLib = GetModuleHandleA("vulkan-1.dll");
      if (!vulkanLib) {
        spdlog::warn("  vulkan-1.dll not found — CPU readback");
        break;
      }

      m_qtGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
        GetProcAddress(vulkanLib, "vkGetDeviceProcAddr"));
      if (!m_qtGetDeviceProcAddr) {
        spdlog::warn("  vkGetDeviceProcAddr missing — CPU readback");
        break;
      }

      auto qd = [&](const char *name) -> PFN_vkVoidFunction {
        return m_qtGetDeviceProcAddr(m_qtDevice, name);
      };
      m_qtfCreateImage = reinterpret_cast<PFN_vkCreateImage>(qd("vkCreateImage"));
      m_qtfDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(qd("vkDestroyImage"));
      m_qtfAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(qd("vkAllocateMemory"));
      m_qtfFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(qd("vkFreeMemory"));
      m_qtfBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(qd("vkBindImageMemory"));
      m_qtfGetImageMemReqs = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(qd("vkGetImageMemoryRequirements"));
      m_qtfCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(qd("vkCreateSemaphore"));
      m_qtfDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(qd("vkDestroySemaphore"));
      m_qtfQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(qd("vkQueueSubmit"));
      m_qtfImportSemWin32Handle = reinterpret_cast<PFN_vkImportSemaphoreWin32HandleKHR>(qd(
        "vkImportSemaphoreWin32HandleKHR"));

      spdlog::info("  Qt fn ptrs: createSem={} importSem={} queueSubmit={}",
                   m_qtfCreateSemaphore ? "ok" : "null",
                   m_qtfImportSemWin32Handle ? "ok" : "null",
                   m_qtfQueueSubmit ? "ok" : "null");

      if (!m_qtfCreateSemaphore || !m_qtfImportSemWin32Handle || !m_qtfQueueSubmit ||
          !m_qtfCreateImage || !m_qtfAllocateMemory || !m_qtfBindImageMemory ||
          !m_qtfGetImageMemReqs) {
        spdlog::warn("  Qt device missing required functions — CPU readback");
        break;
      }

      auto vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(qd("vkGetDeviceQueue"));
      if (!vkGetDeviceQueue) {
        spdlog::warn("  vkGetDeviceQueue null");
        break;
      }
      vkGetDeviceQueue(m_qtDevice, qtNative->gfxQueueFamilyIdx, 0, &m_qtQueue);
      spdlog::info("  Qt queue obtained, family={}", qtNative->gfxQueueFamilyIdx);

      spdlog::info("  creating Qt-side timeline semaphore");
      VkSemaphoreTypeCreateInfo qtSemType{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
      qtSemType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
      qtSemType.initialValue = 0;
      VkSemaphoreCreateInfo qtSci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
      qtSci.pNext = &qtSemType;
      VkResult res = m_qtfCreateSemaphore(m_qtDevice, &qtSci, nullptr, &m_qtImportedSem);
      if (res != VK_SUCCESS) {
        spdlog::warn("  Qt vkCreateSemaphore failed ({}) — CPU readback", int(res));
        break;
      }

      spdlog::info("  importing Granite semaphore handle into Qt device");
      VkImportSemaphoreWin32HandleInfoKHR importSem{VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR};
      importSem.semaphore = m_qtImportedSem;
      importSem.flags = 0;
      importSem.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
      importSem.handle = m_sharedSemHandle;
      importSem.name = nullptr;
      res = m_qtfImportSemWin32Handle(m_qtDevice, &importSem);
      if (res != VK_SUCCESS) {
        spdlog::warn("  vkImportSemaphoreWin32HandleKHR failed ({}) — CPU readback", int(res));
        m_qtfDestroySemaphore(m_qtDevice, m_qtImportedSem, nullptr);
        m_qtImportedSem = VK_NULL_HANDLE;
        break;
      }
      spdlog::info("  cross-device semaphore ready — GPU copy path enabled");
      sharedOk = true;
    } while (false);

    if (!sharedOk && m_qtImportedSem != VK_NULL_HANDLE && m_qtfDestroySemaphore) {
      m_qtfDestroySemaphore(m_qtDevice, m_qtImportedSem, nullptr);
      m_qtImportedSem = VK_NULL_HANDLE;
    }
  }
#endif

  createVulkanPerFrameResources();
  buildVulkanHwInterface(vk->inst->vkInstance(), ctx.gpu, globalProcAddr);

  // context_reset: the core calls GET_HW_RENDER_INTERFACE from inside here,
  // which returns &m_vkInterface. Granite then finishes its device setup
  if (resetCallback) {
    resetCallback();
  }

  m_vulkanInitialized = true;
  spdlog::info("EmulatorVulkanRenderer: Vulkan initialized");
  return true;
}

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Per-frame resource setup
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorVulkanRenderer::createVulkanPerFrameResources() {
  m_vkFrameFences.resize(1);
  m_vkCmdBuffers.resize(1);

  VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  m_vkfCreateFence(m_vkDevice, &fi, nullptr, &m_vkFrameFences[0]);

  VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pi.queueFamilyIndex = m_vkQueueFamilyIndex;
  pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  m_vkfCreateCommandPool(m_vkDevice, &pi, nullptr, &m_vkCmdPool);

  VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  ai.commandPool = m_vkCmdPool;
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  ai.commandBufferCount = 1;
  m_vkfAllocCommandBuffers(m_vkDevice, &ai, m_vkCmdBuffers.data());
}

void EmulatorVulkanRenderer::buildVulkanHwInterface(
  VkInstance instance, VkPhysicalDevice physDev,
  PFN_vkGetInstanceProcAddr procAddr) {
  m_vkInterface = {};
  m_vkInterface.interface_type = RETRO_HW_RENDER_INTERFACE_VULKAN;
  m_vkInterface.interface_version = RETRO_HW_RENDER_INTERFACE_VULKAN_VERSION;
  m_vkInterface.instance = instance;
  m_vkInterface.gpu = physDev;
  m_vkInterface.device = m_vkDevice;
  m_vkInterface.queue = m_vkQueue;
  m_vkInterface.queue_index = m_vkQueueFamilyIndex;
  m_vkInterface.handle = this;
  m_vkInterface.get_instance_proc_addr = procAddr;
  m_vkInterface.get_device_proc_addr = m_vkGetDeviceProcAddr;

  m_vkInterface.get_sync_index = [](void *) -> unsigned { return 0; };
  m_vkInterface.get_sync_index_mask = [](void *) -> unsigned { return 1; };
  m_vkInterface.wait_sync_index = [](void *) {};

  m_vkInterface.set_image = [](void *handle,
                               const retro_vulkan_image *image,
                               uint32_t,
                               const VkSemaphore *,
                               uint32_t) {
    auto *r = static_cast<EmulatorVulkanRenderer *>(handle);
    r->m_coreImage = image->create_info.image;
    r->m_coreImageFormat = image->create_info.format;
    r->m_coreImageView = image->image_view;
    r->m_coreImageLayout = image->image_layout;
    r->m_vkFrameReady = true;
  };

  m_vkInterface.set_signal_semaphore = [](void *handle, VkSemaphore sem) {
    static_cast<EmulatorVulkanRenderer *>(handle)->m_coreSignalSem = sem;
  };

  m_vkInterface.lock_queue = [](void *handle) {
    static_cast<EmulatorVulkanRenderer *>(handle)->m_vkQueueMutex.lock();
  };

  m_vkInterface.unlock_queue = [](void *handle) {
    static_cast<EmulatorVulkanRenderer *>(handle)->m_vkQueueMutex.unlock();
  };

  m_vkInterface.set_command_buffers = nullptr;

  m_vkInterfaceReady = true;
}

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Staging buffer (CPU readback fallback)
// ─────────────────────────────────────────────────────────────────────────────

bool EmulatorVulkanRenderer::ensureStagingBuffer() {
  VkDeviceSize required =
      static_cast<VkDeviceSize>(m_vkRenderWidth) * m_vkRenderHeight * 4;
  if (m_vkStagingBuffer != VK_NULL_HANDLE && m_vkStagingSize >= required)
    return true;

  if (m_vkStagingBuffer != VK_NULL_HANDLE) {
    m_vkfDestroyBuffer(m_vkDevice, m_vkStagingBuffer, nullptr);
    m_vkfFreeMemory(m_vkDevice, m_vkStagingMemory, nullptr);
    m_vkStagingBuffer = VK_NULL_HANDLE;
    m_vkStagingMemory = VK_NULL_HANDLE;
  }
  m_vkStagingSize = required;

  VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bci.size = m_vkStagingSize;
  bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (m_vkfCreateBuffer(m_vkDevice, &bci, nullptr, &m_vkStagingBuffer) != VK_SUCCESS) {
    spdlog::error("EmulatorVulkanRenderer: staging buffer creation failed");
    return false;
  }

  VkMemoryRequirements memReqs;
  m_vkfGetBufferMemoryReqs(m_vkDevice, m_vkStagingBuffer, &memReqs);

  uint32_t memType = UINT32_MAX;
  for (uint32_t i = 0; i < m_vkPhysDevMemProps.memoryTypeCount; i++) {
    if (!(memReqs.memoryTypeBits & (1u << i))) continue;
    auto flags = m_vkPhysDevMemProps.memoryTypes[i].propertyFlags;
    if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
        (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      memType = i;
      break;
    }
  }
  if (memType == UINT32_MAX) {
    spdlog::error("EmulatorVulkanRenderer: no host-visible+coherent memory type found");
    return false;
  }

  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.allocationSize = memReqs.size;
  mai.memoryTypeIndex = memType;
  if (m_vkfAllocateMemory(m_vkDevice, &mai, nullptr, &m_vkStagingMemory) != VK_SUCCESS) {
    spdlog::error("EmulatorVulkanRenderer: staging memory allocation failed");
    return false;
  }

  m_vkfBindBufferMemory(m_vkDevice, m_vkStagingBuffer, m_vkStagingMemory, 0);
  spdlog::info("EmulatorVulkanRenderer: staging buffer {}x{}", m_vkRenderWidth, m_vkRenderHeight);
  return true;
}

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Shared image management
// ─────────────────────────────────────────────────────────────────────────────

bool EmulatorVulkanRenderer::ensureSharedImage(QSize targetSize, QRhi *rhi) {
#ifdef _WIN32
  const uint32_t wantW = static_cast<uint32_t>(targetSize.width());
  const uint32_t wantH = static_cast<uint32_t>(targetSize.height());

  if (m_sharedImage != VK_NULL_HANDLE &&
      m_sharedImageW == wantW &&
      m_sharedImageH == wantH &&
      m_sharedImageFmt == m_coreImageFormat)
    return true;

  destroySharedImage();

  m_sharedImageW = wantW;
  m_sharedImageH = wantH;
  m_sharedImageFmt = m_coreImageFormat;

  // ── Create image on Granite's device with exportable memory ──────────────
  VkExternalMemoryImageCreateInfo extImg{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  extImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

  VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  ici.pNext = &extImg;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = m_coreImageFormat;
  ici.extent = {wantW, wantH, 1};
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = VK_SAMPLE_COUNT_1_BIT;
  ici.tiling = VK_IMAGE_TILING_OPTIMAL;
  ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (m_vkfCreateImage(m_vkDevice, &ici, nullptr, &m_sharedImage) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Granite-side image creation failed");
    return false;
  }

  VkMemoryRequirements memReqs{};
  m_vkfGetImageMemReqs(m_vkDevice, m_sharedImage, &memReqs);

  uint32_t memType = UINT32_MAX;
  for (uint32_t i = 0; i < m_vkPhysDevMemProps.memoryTypeCount; i++) {
    if (!(memReqs.memoryTypeBits & (1u << i))) continue;
    if (m_vkPhysDevMemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
      memType = i;
      break;
    }
  }
  if (memType == UINT32_MAX) {
    spdlog::error("ensureSharedImage: no device-local memory type found");
    return false;
  }

  VkExportMemoryAllocateInfo exportMem{VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO};
  exportMem.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.pNext = &exportMem;
  mai.allocationSize = memReqs.size;
  mai.memoryTypeIndex = memType;

  if (m_vkfAllocateMemory(m_vkDevice, &mai, nullptr, &m_sharedImageMem) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Granite-side memory allocation failed");
    return false;
  }
  m_vkfBindImageMemory(m_vkDevice, m_sharedImage, m_sharedImageMem, 0);

  if (!m_vkfGetMemWin32Handle) {
    spdlog::error("ensureSharedImage: vkGetMemoryWin32HandleKHR unavailable "
                  "(core device missing VK_KHR_external_memory_win32)");
    return false;
  }
  VkMemoryGetWin32HandleInfoKHR getHandle{VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR};
  getHandle.memory = m_sharedImageMem;
  getHandle.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
  if (m_vkfGetMemWin32Handle(m_vkDevice, &getHandle, &m_sharedImageHandle) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: vkGetMemoryWin32HandleKHR failed");
    return false;
  }

  // ── Create corresponding image on Qt's device backed by the imported memory ──
  VkExternalMemoryImageCreateInfo qtExtImg{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  qtExtImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

  VkImageCreateInfo qtIci = ici;
  qtIci.pNext = &qtExtImg;
  qtIci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

  if (m_qtfCreateImage(m_qtDevice, &qtIci, nullptr, &m_qtSharedImage) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Qt-side image creation failed");
    return false;
  }

  VkImportMemoryWin32HandleInfoKHR importMem{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
  importMem.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
  importMem.handle = m_sharedImageHandle;

  VkMemoryAllocateInfo qtMai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  qtMai.pNext = &importMem;
  qtMai.allocationSize = memReqs.size;
  qtMai.memoryTypeIndex = memType;

  if (m_qtfAllocateMemory(m_qtDevice, &qtMai, nullptr, &m_qtSharedImageMem) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: Qt-side memory import failed");
    return false;
  }
  m_qtfBindImageMemory(m_qtDevice, m_qtSharedImage, m_qtSharedImageMem, 0);

  // ── Wrap as QRhiTexture so Qt can copy from it ────────────────────────────
  if (m_sharedTex) {
    m_sharedTex->destroy();
    delete m_sharedTex;
    m_sharedTex = nullptr;
  }
  const QRhiTexture::Format rhiFormat =
  (m_coreImageFormat == VK_FORMAT_R8G8B8A8_UNORM ||
   m_coreImageFormat == VK_FORMAT_R8G8B8A8_SRGB)
    ? QRhiTexture::RGBA8
    : QRhiTexture::BGRA8;

  m_sharedTex = rhi->newTexture(rhiFormat, QSize(int(wantW), int(wantH)), 1);
  if (!m_sharedTex->createFrom({
    reinterpret_cast<quint64>(m_qtSharedImage),
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  })) {
    spdlog::error("ensureSharedImage: QRhiTexture::createFrom failed");
    delete m_sharedTex;
    m_sharedTex = nullptr;
    return false;
  }

  spdlog::info("ensureSharedImage: shared={}x{} core={}x{} fmt={} — GPU copy path active",
               wantW, wantH, m_vkRenderWidth, m_vkRenderHeight, int(m_coreImageFormat));
  return true;
#else
  return false;
#endif
}

void EmulatorVulkanRenderer::destroySharedImage() {
  if (m_sharedTex) {
    m_sharedTex->destroy();
    delete m_sharedTex;
    m_sharedTex = nullptr;
  }
#ifdef _WIN32
  if (m_qtSharedImage != VK_NULL_HANDLE && m_qtfDestroyImage) {
    m_qtfDestroyImage(m_qtDevice, m_qtSharedImage, nullptr);
    m_qtSharedImage = VK_NULL_HANDLE;
  }
  if (m_qtSharedImageMem != VK_NULL_HANDLE && m_qtfFreeMemory) {
    m_qtfFreeMemory(m_qtDevice, m_qtSharedImageMem, nullptr);
    m_qtSharedImageMem = VK_NULL_HANDLE;
  }
  if (m_sharedImageHandle) {
    CloseHandle(m_sharedImageHandle);
    m_sharedImageHandle = nullptr;
  }
  if (m_sharedImage != VK_NULL_HANDLE && m_vkfDestroyImage) {
    m_vkfDestroyImage(m_vkDevice, m_sharedImage, nullptr);
    m_sharedImage = VK_NULL_HANDLE;
  }
  if (m_sharedImageMem != VK_NULL_HANDLE && m_vkfFreeMemory) {
    m_vkfFreeMemory(m_vkDevice, m_sharedImageMem, nullptr);
    m_sharedImageMem = VK_NULL_HANDLE;
  }
#endif
  m_sharedImageW = m_sharedImageH = 0;
  m_sharedImageFmt = VK_FORMAT_UNDEFINED;
}

// TODO
// ─────────────────────────────────────────────────────────────────────────────
// Teardown
// ─────────────────────────────────────────────────────────────────────────────

void EmulatorVulkanRenderer::destroy() {
  if (m_vkDevice == VK_NULL_HANDLE)
    return;

  m_vkfDeviceWaitIdle(m_vkDevice);

  destroySharedImage();
#ifdef _WIN32
  if (m_qtImportedSem != VK_NULL_HANDLE && m_qtfDestroySemaphore) {
    m_qtfDestroySemaphore(m_qtDevice, m_qtImportedSem, nullptr);
    m_qtImportedSem = VK_NULL_HANDLE;
  }
  if (m_sharedSemHandle) {
    CloseHandle(m_sharedSemHandle);
    m_sharedSemHandle = nullptr;
  }
  if (m_sharedSemaphore != VK_NULL_HANDLE && m_vkfDestroySemaphore) {
    m_vkfDestroySemaphore(m_vkDevice, m_sharedSemaphore, nullptr);
    m_sharedSemaphore = VK_NULL_HANDLE;
  }
#endif

  if (m_fnDestroyDevice) {
    m_fnDestroyDevice();
    m_fnDestroyDevice = nullptr;
  }

  if (!m_vkFrameFences.empty() && m_vkFrameFences[0])
    m_vkfDestroyFence(m_vkDevice, m_vkFrameFences[0], nullptr);
  m_vkFrameFences.clear();

  if (m_vkCmdPool)
    m_vkfDestroyCommandPool(m_vkDevice, m_vkCmdPool, nullptr);
  m_vkCmdPool = VK_NULL_HANDLE;

  if (m_vkStagingBuffer) {
    m_vkfDestroyBuffer(m_vkDevice, m_vkStagingBuffer, nullptr);
    m_vkStagingBuffer = VK_NULL_HANDLE;
  }
  if (m_vkStagingMemory) {
    m_vkfFreeMemory(m_vkDevice, m_vkStagingMemory, nullptr);
    m_vkStagingMemory = VK_NULL_HANDLE;
  }
  m_vkStagingSize = 0;

  m_vkfDestroyDevice(m_vkDevice, nullptr);
  m_vkDevice = VK_NULL_HANDLE;

  if (m_graniteInstance != VK_NULL_HANDLE && m_vkfDestroyInstance) {
    m_vkfDestroyInstance(m_graniteInstance, nullptr);
    m_graniteInstance = VK_NULL_HANDLE;
  }
}
