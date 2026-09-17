// TODO: NEEDS REVIEW
#include "vulkan_presenter.hpp"

#include <QVulkanInstance>
#include <cstring>
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

VulkanPresenter::~VulkanPresenter() { destroy(); }

bool VulkanPresenter::initialize(QRhi *rhi, void *windowHandle, VulkanHostHandles &host) {
  if (m_device != VK_NULL_HANDLE) {
    return true;
  }

  const auto *native = static_cast<const QRhiVulkanNativeHandles *>(rhi->nativeHandles());

  if (!native || native->dev == VK_NULL_HANDLE || native->inst == nullptr) {
    return false;
  }

  m_device = native->dev;

#ifdef _WIN32
  if (HMODULE vulkanLib = GetModuleHandleA("vulkan-1.dll")) {
    m_getDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(GetProcAddress(vulkanLib, "vkGetDeviceProcAddr"));
  }
#endif
  if (!m_getDeviceProcAddr) {
    m_getDeviceProcAddr =
        reinterpret_cast<PFN_vkGetDeviceProcAddr>(native->inst->getInstanceProcAddr("vkGetDeviceProcAddr"));
  }
  if (!m_getDeviceProcAddr) {
    spdlog::warn("VulkanPresenter: vkGetDeviceProcAddr missing; hardware-rendered cores cannot be shown");
    return false;
  }

  auto d = [&](const char *name) { return m_getDeviceProcAddr(m_device, name); };
  m_fnCreateImage = reinterpret_cast<PFN_vkCreateImage>(d("vkCreateImage"));
  m_fnDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(d("vkDestroyImage"));
  m_fnAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(d("vkAllocateMemory"));
  m_fnFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(d("vkFreeMemory"));
  m_fnBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(d("vkBindImageMemory"));
  m_fnGetImageMemoryRequirements =
      reinterpret_cast<PFN_vkGetImageMemoryRequirements>(d("vkGetImageMemoryRequirements"));
#ifdef _WIN32
  m_fnGetMemoryWin32HandleProperties =
      reinterpret_cast<PFN_vkGetMemoryWin32HandlePropertiesKHR>(d("vkGetMemoryWin32HandlePropertiesKHR"));
#endif

  if (!m_fnCreateImage || !m_fnAllocateMemory || !m_fnBindImageMemory || !m_fnGetImageMemoryRequirements) {
    spdlog::warn("VulkanPresenter: the display's device lacks functions the import needs");
    return false;
  }

  auto getProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
      native->inst->getInstanceProcAddr("vkGetPhysicalDeviceProperties2"));

  if (!getProperties2) {
    getProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
        native->inst->getInstanceProcAddr("vkGetPhysicalDeviceProperties2KHR"));
  }

  if (getProperties2) {
    VkPhysicalDeviceIDProperties idProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
    VkPhysicalDeviceProperties2 properties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    properties.pNext = &idProperties;
    getProperties2(native->physDev, &properties);

    if (idProperties.deviceLUIDValid) {
      std::memcpy(&host.adapterLuid, idProperties.deviceLUID, sizeof(host.adapterLuid));
    }
  }

  host.windowHandle = windowHandle;
  spdlog::info("VulkanPresenter: display adapter LUID {:#x}", host.adapterLuid);
  return true;
}

bool VulkanPresenter::show(const SharedFrame &frame, QRhi *rhi, QRhiResourceUpdateBatch *batch, QRhiTexture *target) {
  if (m_device == VK_NULL_HANDLE || frame.value == 0 || frame.value == m_shownValue) {
    return false;
  }

  if (frame.generation != m_importedGeneration && !importImage(frame, rhi)) {
    return false;
  }

  if (target->pixelSize() != QSize(static_cast<int>(frame.width), static_cast<int>(frame.height))) {
    return false;
  }

  // TODO
  // The picture is complete before it is published: the core's side waited for its own copy
  batch->copyTexture(target, m_texture);
  m_shownValue = frame.value;
  return true;
}

bool VulkanPresenter::showAgain(QRhiResourceUpdateBatch *batch, QRhiTexture *target) {
  if (!m_texture || m_shownValue == 0 || target->pixelSize() != m_texture->pixelSize()) {
    return false;
  }

  batch->copyTexture(target, m_texture);
  return true;
}

bool VulkanPresenter::importImage(const SharedFrame &frame, QRhi *rhi) {
#ifdef _WIN32
  releaseImage();

  if (frame.handle == nullptr || frame.width == 0 || frame.height == 0 || !m_fnGetMemoryWin32HandleProperties) {
    return false;
  }

  const auto isBgra = frame.format == DXGI_FORMAT_B8G8R8A8_UNORM;
  const auto vkFormat = isBgra ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;

  VkExternalMemoryImageCreateInfo extImg{VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO};
  extImg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

  VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  ici.pNext = &extImg;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = vkFormat;
  ici.extent = {frame.width, frame.height, 1};
  ici.mipLevels = 1;
  ici.arrayLayers = 1;
  ici.samples = VK_SAMPLE_COUNT_1_BIT;
  ici.tiling = VK_IMAGE_TILING_OPTIMAL;
  ici.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (m_fnCreateImage(m_device, &ici, nullptr, &m_image) != VK_SUCCESS) {
    spdlog::error("VulkanPresenter: image creation failed");
    m_image = VK_NULL_HANDLE;
    return false;
  }

  VkMemoryRequirements memReqs{};
  m_fnGetImageMemoryRequirements(m_device, m_image, &memReqs);

  VkMemoryWin32HandlePropertiesKHR handleProperties{VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR};

  if (m_fnGetMemoryWin32HandleProperties(m_device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, frame.handle,
                                         &handleProperties) != VK_SUCCESS) {
    spdlog::error("VulkanPresenter: vkGetMemoryWin32HandlePropertiesKHR failed");
    releaseImage();
    return false;
  }

  const auto allowedTypes = memReqs.memoryTypeBits & handleProperties.memoryTypeBits;

  if (allowedTypes == 0) {
    spdlog::error("VulkanPresenter: no memory type can hold the shared texture");
    releaseImage();
    return false;
  }

  uint32_t memType = 0;

  while (!(allowedTypes & (1u << memType))) {
    memType++;
  }

  VkMemoryDedicatedAllocateInfo dedicated{VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};
  dedicated.image = m_image;

  VkImportMemoryWin32HandleInfoKHR importMem{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
  importMem.pNext = &dedicated;
  importMem.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
  importMem.handle = frame.handle;

  VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  mai.pNext = &importMem;
  mai.allocationSize = memReqs.size;
  mai.memoryTypeIndex = memType;

  if (const auto res = m_fnAllocateMemory(m_device, &mai, nullptr, &m_memory); res != VK_SUCCESS) {
    spdlog::error("VulkanPresenter: memory import failed ({})", static_cast<int>(res));
    m_memory = VK_NULL_HANDLE;
    releaseImage();
    return false;
  }

  m_fnBindImageMemory(m_device, m_image, m_memory, 0);

  m_texture = rhi->newTexture(isBgra ? QRhiTexture::BGRA8 : QRhiTexture::RGBA8,
                              QSize(static_cast<int>(frame.width), static_cast<int>(frame.height)), 1);

  if (!m_texture->createFrom({reinterpret_cast<quint64>(m_image), VK_IMAGE_LAYOUT_GENERAL})) {
    spdlog::error("VulkanPresenter: QRhiTexture::createFrom failed");
    releaseImage();
    return false;
  }

  m_importedGeneration = frame.generation;
  spdlog::info("VulkanPresenter: imported shared texture {}x{} (generation {})", frame.width, frame.height,
               frame.generation);
  return true;
#else
  (void)frame;
  (void)rhi;
  return false;
#endif
}

void VulkanPresenter::releaseImage() {
  if (m_texture) {
    m_texture->destroy();
    delete m_texture;
    m_texture = nullptr;
  }
  if (m_image != VK_NULL_HANDLE && m_fnDestroyImage) {
    m_fnDestroyImage(m_device, m_image, nullptr);
    m_image = VK_NULL_HANDLE;
  }
  if (m_memory != VK_NULL_HANDLE && m_fnFreeMemory) {
    m_fnFreeMemory(m_device, m_memory, nullptr);
    m_memory = VK_NULL_HANDLE;
  }
  m_importedGeneration = 0;
}

void VulkanPresenter::destroy() {
  releaseImage();

  m_shownValue = 0;
  m_device = VK_NULL_HANDLE;
}
