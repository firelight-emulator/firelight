// TODO: NEEDS REVIEW
#include "vulkan_core_context.hpp"

#include <QByteArray>
#include <QtGlobal>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#ifdef _WIN32
// clang-format off
#include <d3d11_1.h>
#include <dxgi1_2.h>
// clang-format on
#endif

namespace {

/**
 * Sets VK_LOADER_LAYERS_DISABLE to keep implicit layers off a second instance in the process, and
 * puts the variable back when it goes out of scope
 */
class ImplicitLayerGuard {
public:
  ImplicitLayerGuard() {
    if (const char *value = getenv("VK_LOADER_LAYERS_DISABLE")) {
      m_saved = value;
    }

    qputenv("VK_LOADER_LAYERS_DISABLE", QByteArray("~implicit~"));
  }

  ~ImplicitLayerGuard() {
    if (m_saved.empty()) {
      qunsetenv("VK_LOADER_LAYERS_DISABLE");
    } else {
      qputenv("VK_LOADER_LAYERS_DISABLE", QByteArray::fromStdString(m_saved));
    }
  }

  ImplicitLayerGuard(const ImplicitLayerGuard &) = delete;

  ImplicitLayerGuard &operator=(const ImplicitLayerGuard &) = delete;

private:
  std::string m_saved;
};

#ifdef _WIN32
uint64_t packLuid(const LUID &luid) {
  return (static_cast<uint64_t>(static_cast<uint32_t>(luid.HighPart)) << 32) | luid.LowPart;
}
#endif

} // namespace

VulkanCoreContext::~VulkanCoreContext() { destroy(); }

void VulkanCoreContext::setRenderDimensions(const uint32_t width, const uint32_t height) {
  if (width >= 2 && height >= 2) {
    m_pendingWidth.store(width, std::memory_order_release);
    m_pendingHeight.store(height, std::memory_order_release);
  }

  m_renderWidth = width;
  m_renderHeight = height;
  m_frameReady = true;
}

SharedFrame VulkanCoreContext::getLatestFrame() const {
  std::lock_guard lock(m_latestMutex);
  return m_latest;
}

bool VulkanCoreContext::initialize(const VulkanHostHandles &host,
                                   const retro_hw_render_context_negotiation_interface_vulkan *negotiation,
                                   const std::function<void()> &resetCallback) {
#ifdef _WIN32
  m_adapterLuid = host.adapterLuid;

  const auto vulkanLib = LoadLibraryW(L"vulkan-1.dll");

  if (vulkanLib == nullptr) {
    spdlog::error("VulkanCoreContext: vulkan-1.dll is not available");
    return false;
  }

  m_vulkanLibrary = vulkanLib;
  m_getInstanceProcAddr =
      reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(vulkanLib, "vkGetInstanceProcAddr"));
#endif
  if (!m_getInstanceProcAddr) {
    spdlog::error("VulkanCoreContext: could not obtain vkGetInstanceProcAddr");
    return false;
  }

  // Steam/OBS/Overwolf overlay layers crash or return null function pointers when a second
  // VkInstance is made in the same process
  const ImplicitLayerGuard layerGuard;

  if (!createInstance(negotiation) || !pickPhysicalDevice()) {
    return false;
  }

#ifdef _WIN32
  // PPSSPP asserts on a null surface and queries swapchain capabilities from it; parallel-RDP
  // ignores it. The surface is only ever queried
  if (const auto createSurface =
          reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(m_getInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR"));
      createSurface && host.windowHandle) {
    VkWin32SurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    sci.hinstance = GetModuleHandleW(nullptr);
    sci.hwnd = static_cast<HWND>(host.windowHandle);

    if (const auto res = createSurface(m_instance, &sci, nullptr, &m_surface); res != VK_SUCCESS) {
      spdlog::warn("VulkanCoreContext: vkCreateWin32SurfaceKHR failed ({}); cores that query a surface may fail",
                   static_cast<int>(res));
      m_surface = VK_NULL_HANDLE;
    }
  } else {
    spdlog::warn("VulkanCoreContext: no window handle; cores that query a surface may fail");
  }
#endif

  // VkPhysicalDeviceFeatures must not be nullptr: parallel_create_device
  // SIMD-copies the features struct without a null guard, causing a crash
  retro_vulkan_context ctx{};
  VkPhysicalDeviceFeatures features{};

  // Device extensions the core must enable so the shared texture can be imported. The core creates
  // the VkDevice, and cores that only enable what they need (PPSSPP) would leave the import
  // functions null. Filtered to what the physical device advertises so create_device cannot fail
  // with VK_ERROR_EXTENSION_NOT_PRESENT
  std::vector<const char *> requiredDeviceExts;
#ifdef _WIN32
  {
    static const char *const CANDIDATES[] = {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    };
    auto enumDevExt = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(
        m_getInstanceProcAddr(m_instance, "vkEnumerateDeviceExtensionProperties"));
    if (enumDevExt) {
      uint32_t count = 0;
      enumDevExt(m_physicalDevice, nullptr, &count, nullptr);
      std::vector<VkExtensionProperties> props(count);
      if (count) {
        enumDevExt(m_physicalDevice, nullptr, &count, props.data());
      }
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

  const bool deviceOk =
      negotiation->create_device(&ctx, m_instance, m_physicalDevice, m_surface, m_getInstanceProcAddr,
                                 requiredDeviceExts.empty() ? nullptr : requiredDeviceExts.data(),
                                 static_cast<unsigned>(requiredDeviceExts.size()), nullptr, 0, &features);

  if (!deviceOk) {
    spdlog::error("VulkanCoreContext: create_device failed");
    return false;
  }

  // Store destroy_device as a bare function pointer now, before the core DLL could
  // be unloaded. negotiation itself becomes a dangling pointer after coreLib->unload(),
  // so destroy() must call it directly
  m_destroyDeviceThroughCore = negotiation->destroy_device;

  m_device = ctx.device;
  m_queue = ctx.queue;
  m_queueFamilyIndex = ctx.queue_family_index;

  if (ctx.gpu != VK_NULL_HANDLE && ctx.gpu != m_physicalDevice) {
    spdlog::warn("VulkanCoreContext: the core picked a different physical device than the display's");
    m_physicalDevice = ctx.gpu;
  }

  m_getDeviceProcAddr =
      reinterpret_cast<PFN_vkGetDeviceProcAddr>(m_getInstanceProcAddr(m_instance, "vkGetDeviceProcAddr"));

  auto d = [&](const char *name) { return m_getDeviceProcAddr(m_device, name); };
  m_fnQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(d("vkQueueSubmit"));
  m_fnCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(d("vkCreateCommandPool"));
  m_fnAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(d("vkAllocateCommandBuffers"));
  m_fnBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(d("vkBeginCommandBuffer"));
  m_fnEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(d("vkEndCommandBuffer"));
  m_fnCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(d("vkCmdPipelineBarrier"));
  m_fnCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(d("vkCmdBlitImage"));
  m_fnCreateFence = reinterpret_cast<PFN_vkCreateFence>(d("vkCreateFence"));
  m_fnDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(d("vkDestroyFence"));
  m_fnWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(d("vkWaitForFences"));
  m_fnResetFences = reinterpret_cast<PFN_vkResetFences>(d("vkResetFences"));
  m_fnDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(d("vkDestroyCommandPool"));
  m_fnDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(d("vkDestroyDevice"));
  m_fnDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(d("vkDeviceWaitIdle"));
  m_fnAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(d("vkAllocateMemory"));
  m_fnFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(d("vkFreeMemory"));
  m_fnCreateImage = reinterpret_cast<PFN_vkCreateImage>(d("vkCreateImage"));
  m_fnDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(d("vkDestroyImage"));
  m_fnBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(d("vkBindImageMemory"));
  m_fnGetImageMemoryRequirements =
      reinterpret_cast<PFN_vkGetImageMemoryRequirements>(d("vkGetImageMemoryRequirements"));
#ifdef _WIN32
  m_fnGetMemoryWin32HandleProperties =
      reinterpret_cast<PFN_vkGetMemoryWin32HandlePropertiesKHR>(d("vkGetMemoryWin32HandlePropertiesKHR"));
#endif

  auto getMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(
      m_getInstanceProcAddr(m_instance, "vkGetPhysicalDeviceMemoryProperties"));
  getMemoryProperties(m_physicalDevice, &m_memoryProperties);

  createPerFrameResources();
  buildInterface();

  // context_reset: the core calls GET_HW_RENDER_INTERFACE from inside here,
  // which returns &m_interface, and finishes its device setup
  if (resetCallback) {
    resetCallback();
  }

  m_initialized = true;
  spdlog::info("VulkanCoreContext: Vulkan initialized on a private instance");
  return true;
}

bool VulkanCoreContext::createInstance(const retro_hw_render_context_negotiation_interface_vulkan *negotiation) {
  const auto createInstance =
      reinterpret_cast<PFN_vkCreateInstance>(m_getInstanceProcAddr(nullptr, "vkCreateInstance"));
  const auto enumerateExtensions = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
      m_getInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties"));

  if (!createInstance || !enumerateExtensions) {
    spdlog::error("VulkanCoreContext: the loader lacks instance creation");
    return false;
  }

  VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.pApplicationName = "Firelight";
  appInfo.pEngineName = "Firelight";
  appInfo.apiVersion = VK_API_VERSION_1_1;

  if (negotiation->get_application_info) {
    if (const auto *coreInfo = negotiation->get_application_info()) {
      appInfo = *coreInfo;
      appInfo.apiVersion = std::max<uint32_t>(appInfo.apiVersion, VK_API_VERSION_1_1);
    }
  }

  static const char *const CANDIDATES[] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
      VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
  };
  uint32_t count = 0;
  enumerateExtensions(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> available(count);

  if (count) {
    enumerateExtensions(nullptr, &count, available.data());
  }

  std::vector<const char *> extensions;

  for (const char *candidate : CANDIDATES) {
    for (const auto &property : available) {
      if (std::strcmp(property.extensionName, candidate) == 0) {
        extensions.push_back(candidate);
        break;
      }
    }
  }

  VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  info.pApplicationInfo = &appInfo;
  info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  info.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

  if (const auto res = createInstance(&info, nullptr, &m_instance); res != VK_SUCCESS) {
    spdlog::error("VulkanCoreContext: vkCreateInstance failed ({})", static_cast<int>(res));
    m_instance = VK_NULL_HANDLE;
    return false;
  }

  m_fnDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(m_getInstanceProcAddr(m_instance, "vkDestroyInstance"));
  m_fnDestroySurface =
      reinterpret_cast<PFN_vkDestroySurfaceKHR>(m_getInstanceProcAddr(m_instance, "vkDestroySurfaceKHR"));
  m_fnGetPhysicalDeviceImageFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(
      m_getInstanceProcAddr(m_instance, "vkGetPhysicalDeviceImageFormatProperties2"));

  if (!m_fnGetPhysicalDeviceImageFormatProperties2) {
    m_fnGetPhysicalDeviceImageFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(
        m_getInstanceProcAddr(m_instance, "vkGetPhysicalDeviceImageFormatProperties2KHR"));
  }

  return true;
}

bool VulkanCoreContext::pickPhysicalDevice() {
  const auto enumerate =
      reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(m_getInstanceProcAddr(m_instance, "vkEnumeratePhysicalDevices"));
  auto getProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
      m_getInstanceProcAddr(m_instance, "vkGetPhysicalDeviceProperties2"));

  if (!getProperties2) {
    getProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
        m_getInstanceProcAddr(m_instance, "vkGetPhysicalDeviceProperties2KHR"));
  }

  if (!enumerate || !getProperties2) {
    spdlog::error("VulkanCoreContext: cannot enumerate physical devices");
    return false;
  }

  uint32_t count = 0;
  enumerate(m_instance, &count, nullptr);
  std::vector<VkPhysicalDevice> devices(count);

  if (count) {
    enumerate(m_instance, &count, devices.data());
  }

  for (const auto device : devices) {
    VkPhysicalDeviceIDProperties idProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
    VkPhysicalDeviceProperties2 properties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    properties.pNext = &idProperties;
    getProperties2(device, &properties);

    uint64_t luid = 0;

    if (idProperties.deviceLUIDValid) {
      std::memcpy(&luid, idProperties.deviceLUID, sizeof(luid));
    }

    spdlog::info("VulkanCoreContext: physical device {} (LUID {:#x})", properties.properties.deviceName, luid);

    if (m_adapterLuid == 0 || (idProperties.deviceLUIDValid && luid == m_adapterLuid)) {
      m_physicalDevice = device;
      return true;
    }
  }

  spdlog::error("VulkanCoreContext: no physical device matches the display's adapter (LUID {:#x})", m_adapterLuid);
  return false;
}

void VulkanCoreContext::createPerFrameResources() {
  VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (auto &fence : m_fences) {
    m_fnCreateFence(m_device, &fi, nullptr, &fence);
  }

  VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pi.queueFamilyIndex = m_queueFamilyIndex;
  pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  m_fnCreateCommandPool(m_device, &pi, nullptr, &m_commandPool);

  VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  ai.commandPool = m_commandPool;
  ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  ai.commandBufferCount = FRAMES_IN_FLIGHT;
  m_fnAllocateCommandBuffers(m_device, &ai, m_commandBuffers.data());
}

void VulkanCoreContext::buildInterface() {
  m_interface = {};
  m_interface.interface_type = RETRO_HW_RENDER_INTERFACE_VULKAN;
  m_interface.interface_version = RETRO_HW_RENDER_INTERFACE_VULKAN_VERSION;
  m_interface.instance = m_instance;
  m_interface.gpu = m_physicalDevice;
  m_interface.device = m_device;
  m_interface.queue = m_queue;
  m_interface.queue_index = m_queueFamilyIndex;
  m_interface.handle = this;
  m_interface.get_instance_proc_addr = m_getInstanceProcAddr;
  m_interface.get_device_proc_addr = m_getDeviceProcAddr;

  m_interface.get_sync_index = [](void *handle) -> unsigned {
    return static_cast<VulkanCoreContext *>(handle)->m_syncIndex;
  };
  m_interface.get_sync_index_mask = [](void *) -> unsigned { return (1u << FRAMES_IN_FLIGHT) - 1u; };
  m_interface.wait_sync_index = [](void *handle) {
    auto *context = static_cast<VulkanCoreContext *>(handle);
    context->waitSlot(context->m_syncIndex);
  };

  m_interface.set_image = [](void *handle, const retro_vulkan_image *image, uint32_t, const VkSemaphore *, uint32_t) {
    auto *context = static_cast<VulkanCoreContext *>(handle);
    context->m_coreImage = image->create_info.image;
    context->m_coreImageFormat = image->create_info.format;
    context->m_coreImageLayout = image->image_layout;
    context->m_frameReady = true;
  };

  m_interface.set_signal_semaphore = [](void *handle, VkSemaphore semaphore) {
    static_cast<VulkanCoreContext *>(handle)->m_coreSignalSemaphore = semaphore;
  };

  m_interface.lock_queue = [](void *handle) { static_cast<VulkanCoreContext *>(handle)->m_queueMutex.lock(); };

  m_interface.unlock_queue = [](void *handle) { static_cast<VulkanCoreContext *>(handle)->m_queueMutex.unlock(); };

  m_interface.set_command_buffers = nullptr;
}

void VulkanCoreContext::waitSlot(const uint32_t slot) {
  m_fnWaitForFences(m_device, 1, &m_fences[slot], VK_TRUE, UINT64_MAX);
}

bool VulkanCoreContext::blitLatest() {
  if (!m_frameReady || m_coreImage == VK_NULL_HANDLE) {
    return false;
  }

  if (m_renderWidth < 2 || m_renderHeight < 2) {
    spdlog::debug("blitLatest: degenerate dims {}x{}, skipping", m_renderWidth, m_renderHeight);
    return false;
  }

  if (!ensureSharedImage()) {
    return false;
  }

  const auto slot = m_syncIndex;

  {
    const firelight::monitoring::ScopedSpan waitSpan(m_blitWaitPrevSpan);
    waitSlot(slot);
  }

  m_fnResetFences(m_device, 1, &m_fences[slot]);

  const auto submitStartNs = m_blitSubmitSpan.begin();
  VkCommandBuffer cmd = m_commandBuffers[slot];
  VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  m_fnBeginCommandBuffer(cmd, &bi);

  // Core image SHADER_READ_ONLY -> TRANSFER_SRC; shared image UNDEFINED -> TRANSFER_DST, dropping
  // what it held
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

  m_fnCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 2, preCopy);

  VkImageBlit region{};
  region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  region.srcOffsets[0] = {0, 0, 0};
  region.srcOffsets[1] = {static_cast<int32_t>(m_renderWidth), static_cast<int32_t>(m_renderHeight), 1};
  region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  region.dstOffsets[0] = {0, 0, 0};
  region.dstOffsets[1] = {static_cast<int32_t>(m_renderWidth), static_cast<int32_t>(m_renderHeight), 1};
  m_fnCmdBlitImage(cmd, m_coreImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_sharedImage,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);

  // TODO
  // Core image back to where it was; shared image released to whoever opens the texture next
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
  postCopy[1].dstAccessMask = 0;
  postCopy[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  postCopy[1].newLayout = VK_IMAGE_LAYOUT_GENERAL;
  postCopy[1].srcQueueFamilyIndex = m_queueFamilyIndex;
  postCopy[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
  postCopy[1].image = m_sharedImage;
  postCopy[1].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  m_fnCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, 2, postCopy);

  m_fnEndCommandBuffer(cmd);

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  si.commandBufferCount = 1;
  si.pCommandBuffers = &cmd;

  if (m_coreSignalSemaphore != VK_NULL_HANDLE) {
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &m_coreSignalSemaphore;
    si.pWaitDstStageMask = &waitStage;
  }

  {
    std::lock_guard lock(m_queueMutex);
    m_fnQueueSubmit(m_queue, 1, &si, m_fences[slot]);
  }

  m_blitSubmitSpan.end(submitStartNs);
  m_syncIndex = (slot + 1) % FRAMES_IN_FLIGHT;

  // TODO
  // Both are consumed: a frame that hands over no new picture copies the one it has without waiting
  // on a semaphore the core will not signal again
  m_frameReady = false;
  m_coreSignalSemaphore = VK_NULL_HANDLE;

  // TODO
  // Waited for here, so a published picture is complete: when the GPU cannot keep up the game slows
  // to its pace, rather than the display's own frames queueing behind the core's
  {
    const firelight::monitoring::ScopedSpan doneSpan(m_blitWaitDoneSpan);
    waitSlot(slot);
  }

  {
    std::lock_guard lock(m_latestMutex);
    m_latest.value++;
  }

  return true;
}

bool VulkanCoreContext::ensureAllocationDevice() {
#ifdef _WIN32
  if (m_allocationDevice != nullptr) {
    return true;
  }

  IDXGIFactory1 *factory = nullptr;

  if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&factory)))) {
    spdlog::error("VulkanCoreContext: CreateDXGIFactory1 failed");
    return false;
  }

  IDXGIAdapter1 *chosen = nullptr;

  for (UINT index = 0;; index++) {
    IDXGIAdapter1 *adapter = nullptr;

    if (factory->EnumAdapters1(index, &adapter) != S_OK) {
      break;
    }

    DXGI_ADAPTER_DESC1 desc{};
    adapter->GetDesc1(&desc);
    const auto isMatch = m_adapterLuid == 0 ? (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0
                                            : packLuid(desc.AdapterLuid) == m_adapterLuid;

    if (isMatch) {
      chosen = adapter;
      break;
    }

    adapter->Release();
  }

  factory->Release();

  if (chosen == nullptr) {
    spdlog::error("VulkanCoreContext: no DXGI adapter matches LUID {:#x}", m_adapterLuid);
    return false;
  }

  ID3D11DeviceContext *context = nullptr;
  const auto hr = D3D11CreateDevice(chosen, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
                                    &m_allocationDevice, nullptr, &context);
  chosen->Release();

  if (context != nullptr) {
    context->Release();
  }

  if (FAILED(hr)) {
    spdlog::error("VulkanCoreContext: D3D11CreateDevice failed ({:#x})", static_cast<uint32_t>(hr));
    m_allocationDevice = nullptr;
    return false;
  }

  return true;
#else
  return false;
#endif
}

bool VulkanCoreContext::ensureSharedImage() {
#ifdef _WIN32
  auto vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
  auto dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

  if (m_coreImageFormat == VK_FORMAT_B8G8R8A8_UNORM || m_coreImageFormat == VK_FORMAT_B8G8R8A8_SRGB) {
    vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
    dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
  }

  if (m_sharedImage != VK_NULL_HANDLE && m_latest.width == m_renderWidth && m_latest.height == m_renderHeight &&
      m_latest.format == dxgiFormat) {
    return true;
  }

  destroySharedImage();

  if (!ensureAllocationDevice()) {
    return false;
  }

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = m_renderWidth;
  desc.Height = m_renderHeight;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = dxgiFormat;
  desc.SampleDesc = {1, 0};
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;

  if (const auto hr = m_allocationDevice->CreateTexture2D(&desc, nullptr, &m_sharedTexture); FAILED(hr)) {
    spdlog::error("ensureSharedImage: CreateTexture2D failed ({:#x})", static_cast<uint32_t>(hr));
    m_sharedTexture = nullptr;
    return false;
  }

  HANDLE handle = nullptr;
  {
    IDXGIResource1 *resource = nullptr;

    if (FAILED(m_sharedTexture->QueryInterface(__uuidof(IDXGIResource1), reinterpret_cast<void **>(&resource)))) {
      spdlog::error("ensureSharedImage: the texture is not a DXGI resource");
      return false;
    }

    const auto hr =
        resource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, &handle);
    resource->Release();

    if (FAILED(hr)) {
      spdlog::error("ensureSharedImage: CreateSharedHandle failed ({:#x})", static_cast<uint32_t>(hr));
      return false;
    }
  }

  {
    std::lock_guard lock(m_latestMutex);
    m_latest.handle = handle;
  }

  constexpr VkImageUsageFlags USAGE = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  if (m_fnGetPhysicalDeviceImageFormatProperties2) {
    VkPhysicalDeviceExternalImageFormatInfo externalInfo{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO};
    externalInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
    VkPhysicalDeviceImageFormatInfo2 formatInfo{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2};
    formatInfo.pNext = &externalInfo;
    formatInfo.format = vkFormat;
    formatInfo.type = VK_IMAGE_TYPE_2D;
    formatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    formatInfo.usage = USAGE;
    VkExternalImageFormatProperties externalProperties{VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES};
    VkImageFormatProperties2 properties{VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2};
    properties.pNext = &externalProperties;
    const auto res = m_fnGetPhysicalDeviceImageFormatProperties2(m_physicalDevice, &formatInfo, &properties);

    if (res != VK_SUCCESS || !(externalProperties.externalMemoryProperties.externalMemoryFeatures &
                               VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
      spdlog::error("ensureSharedImage: this device cannot import a D3D11 texture of format {} ({})",
                    static_cast<int>(vkFormat), static_cast<int>(res));
      return false;
    }
  }

  VkExternalMemoryImageCreateInfo extImg{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  extImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

  VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  ici.pNext = &extImg;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = vkFormat;
  ici.extent = {m_renderWidth, m_renderHeight, 1};
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = VK_SAMPLE_COUNT_1_BIT;
  ici.tiling = VK_IMAGE_TILING_OPTIMAL;
  ici.usage = USAGE;
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (m_fnCreateImage(m_device, &ici, nullptr, &m_sharedImage) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: image creation failed");
    m_sharedImage = VK_NULL_HANDLE;
    return false;
  }

  VkMemoryRequirements memReqs{};
  m_fnGetImageMemoryRequirements(m_device, m_sharedImage, &memReqs);

  if (!m_fnGetMemoryWin32HandleProperties) {
    spdlog::error("ensureSharedImage: vkGetMemoryWin32HandlePropertiesKHR unavailable "
                  "(core device missing VK_KHR_external_memory_win32)");
    return false;
  }

  VkMemoryWin32HandlePropertiesKHR handleProperties{VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR};

  if (m_fnGetMemoryWin32HandleProperties(m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, handle,
                                         &handleProperties) != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: vkGetMemoryWin32HandlePropertiesKHR failed");
    return false;
  }

  const auto allowedTypes = memReqs.memoryTypeBits & handleProperties.memoryTypeBits;
  uint32_t memType = UINT32_MAX;

  for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
    if (!(allowedTypes & (1u << i))) {
      continue;
    }

    const auto isDeviceLocal =
        (m_memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;

    if (memType == UINT32_MAX || isDeviceLocal) {
      memType = i;
    }

    if (isDeviceLocal) {
      break;
    }
  }

  if (memType == UINT32_MAX) {
    spdlog::error("ensureSharedImage: no memory type can hold the shared texture");
    return false;
  }

  VkMemoryDedicatedAllocateInfo dedicated{VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};
  dedicated.image = m_sharedImage;

  VkImportMemoryWin32HandleInfoKHR importMem{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
  importMem.pNext = &dedicated;
  importMem.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
  importMem.handle = handle;

  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.pNext = &importMem;
  mai.allocationSize = memReqs.size;
  mai.memoryTypeIndex = memType;

  if (const auto res = m_fnAllocateMemory(m_device, &mai, nullptr, &m_sharedImageMemory); res != VK_SUCCESS) {
    spdlog::error("ensureSharedImage: importing the shared texture failed ({})", static_cast<int>(res));
    m_sharedImageMemory = VK_NULL_HANDLE;
    return false;
  }

  m_fnBindImageMemory(m_device, m_sharedImage, m_sharedImageMemory, 0);

  {
    std::lock_guard lock(m_latestMutex);
    m_latest.generation++;
    m_latest.width = m_renderWidth;
    m_latest.height = m_renderHeight;
    m_latest.format = dxgiFormat;
  }

  spdlog::info("ensureSharedImage: shared texture {}x{} format {} (generation {})", m_renderWidth, m_renderHeight,
               static_cast<int>(dxgiFormat), m_latest.generation);
  return true;
#else
  return false;
#endif
}

void VulkanCoreContext::destroySharedImage() {
#ifdef _WIN32
  if (m_sharedImage == VK_NULL_HANDLE && m_sharedTexture == nullptr) {
    return;
  }

  // TODO
  // A copy into the old image may still be running
  if (m_device != VK_NULL_HANDLE) {
    m_fnDeviceWaitIdle(m_device);
  }

  HANDLE handle = nullptr;
  {
    std::lock_guard lock(m_latestMutex);
    handle = m_latest.handle;
    m_latest.handle = nullptr;
    m_latest.width = 0;
    m_latest.height = 0;
    m_latest.format = DXGI_FORMAT_UNKNOWN;
  }

  if (m_sharedImage != VK_NULL_HANDLE && m_fnDestroyImage) {
    m_fnDestroyImage(m_device, m_sharedImage, nullptr);
  }

  m_sharedImage = VK_NULL_HANDLE;

  if (m_sharedImageMemory != VK_NULL_HANDLE && m_fnFreeMemory) {
    m_fnFreeMemory(m_device, m_sharedImageMemory, nullptr);
    m_sharedImageMemory = VK_NULL_HANDLE;
  }

  if (handle) {
    CloseHandle(handle);
  }

  if (m_sharedTexture != nullptr) {
    m_sharedTexture->Release();
    m_sharedTexture = nullptr;
  }
#endif
}

void VulkanCoreContext::destroy() {
  if (m_device != VK_NULL_HANDLE) {
    m_fnDeviceWaitIdle(m_device);
  }

  destroySharedImage();

#ifdef _WIN32
  if (m_allocationDevice != nullptr) {
    m_allocationDevice->Release();
    m_allocationDevice = nullptr;
  }
#endif

  {
    std::lock_guard lock(m_latestMutex);
    m_latest = {};
  }

  if (m_destroyDeviceThroughCore) {
    m_destroyDeviceThroughCore();
    m_destroyDeviceThroughCore = nullptr;
  }

  if (m_device != VK_NULL_HANDLE) {
    for (auto &fence : m_fences) {
      if (fence != VK_NULL_HANDLE) {
        m_fnDestroyFence(m_device, fence, nullptr);
        fence = VK_NULL_HANDLE;
      }
    }

    if (m_commandPool != VK_NULL_HANDLE) {
      m_fnDestroyCommandPool(m_device, m_commandPool, nullptr);
      m_commandPool = VK_NULL_HANDLE;
    }

    m_fnDestroyDevice(m_device, nullptr);
    m_device = VK_NULL_HANDLE;
  }

  if (m_surface != VK_NULL_HANDLE && m_fnDestroySurface) {
    m_fnDestroySurface(m_instance, m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }

  if (m_instance != VK_NULL_HANDLE && m_fnDestroyInstance) {
    m_fnDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
  }

  m_physicalDevice = VK_NULL_HANDLE;

#ifdef _WIN32
  if (m_vulkanLibrary != nullptr) {
    FreeLibrary(static_cast<HMODULE>(m_vulkanLibrary));
    m_vulkanLibrary = nullptr;
  }
#endif

  m_getInstanceProcAddr = nullptr;
  m_initialized = false;
}
