// TODO: NEEDS REVIEW
#include "d3d_presenter.hpp"

// clang-format off
#include <d3d11_1.h>
#include <dxgi1_2.h>
// clang-format on
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

D3dPresenter::~D3dPresenter() { destroy(); }

bool D3dPresenter::initialize(QRhi *rhi, void *windowHandle, VulkanHostHandles &host) {
  if (m_device != nullptr) {
    return true;
  }

  const auto *native = static_cast<const QRhiD3D11NativeHandles *>(rhi->nativeHandles());

  if (!native || !native->dev) {
    return false;
  }

  auto *device = static_cast<ID3D11Device *>(native->dev);

  if (FAILED(device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void **>(&m_device)))) {
    spdlog::warn("D3dPresenter: the display's device cannot open shared resources");
    m_device = nullptr;
    return false;
  }

  IDXGIDevice *dxgiDevice = nullptr;

  if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice)))) {
    IDXGIAdapter *adapter = nullptr;

    if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
      DXGI_ADAPTER_DESC desc{};

      if (SUCCEEDED(adapter->GetDesc(&desc))) {
        host.adapterLuid =
            (static_cast<uint64_t>(static_cast<uint32_t>(desc.AdapterLuid.HighPart)) << 32) | desc.AdapterLuid.LowPart;
      }

      adapter->Release();
    }

    dxgiDevice->Release();
  }

  host.windowHandle = windowHandle;
  spdlog::info("D3dPresenter: display adapter LUID {:#x}", host.adapterLuid);
  return true;
}

bool D3dPresenter::show(const SharedFrame &frame, QRhi *rhi, QRhiResourceUpdateBatch *batch, QRhiTexture *target) {
  if (m_device == nullptr || frame.value == 0 || frame.value == m_shownValue) {
    return false;
  }

  if (frame.generation != m_importedGeneration && !openTexture(frame, rhi)) {
    return false;
  }

  if (target->pixelSize() != QSize(static_cast<int>(frame.width), static_cast<int>(frame.height))) {
    return false;
  }

  // TODO
  // The picture is complete before it is published: the core's side waited for its own copy
  batch->copyTexture(target, m_wrapper);
  m_shownValue = frame.value;
  return true;
}

bool D3dPresenter::showAgain(QRhiResourceUpdateBatch *batch, QRhiTexture *target) {
  if (!m_wrapper || m_shownValue == 0 || target->pixelSize() != m_wrapper->pixelSize()) {
    return false;
  }

  batch->copyTexture(target, m_wrapper);
  return true;
}

bool D3dPresenter::openTexture(const SharedFrame &frame, QRhi *rhi) {
  releaseTexture();

  if (frame.handle == nullptr || frame.width == 0 || frame.height == 0) {
    return false;
  }

  if (const auto hr =
          m_device->OpenSharedResource1(frame.handle, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&m_texture));
      FAILED(hr)) {
    spdlog::error("D3dPresenter: OpenSharedResource1 failed ({:#x})", static_cast<uint32_t>(hr));
    m_texture = nullptr;
    return false;
  }

  const auto rhiFormat = frame.format == DXGI_FORMAT_B8G8R8A8_UNORM ? QRhiTexture::BGRA8 : QRhiTexture::RGBA8;
  m_wrapper = rhi->newTexture(rhiFormat, QSize(static_cast<int>(frame.width), static_cast<int>(frame.height)), 1);

  if (!m_wrapper->createFrom({reinterpret_cast<quint64>(m_texture), 0})) {
    spdlog::error("D3dPresenter: QRhiTexture::createFrom failed");
    releaseTexture();
    return false;
  }

  m_importedGeneration = frame.generation;
  spdlog::info("D3dPresenter: opened shared texture {}x{} (generation {})", frame.width, frame.height,
               frame.generation);
  return true;
}

void D3dPresenter::releaseTexture() {
  if (m_wrapper) {
    m_wrapper->destroy();
    delete m_wrapper;
    m_wrapper = nullptr;
  }

  if (m_texture) {
    m_texture->Release();
    m_texture = nullptr;
  }

  m_importedGeneration = 0;
}

void D3dPresenter::destroy() {
  releaseTexture();
  m_shownValue = 0;

  if (m_device) {
    m_device->Release();
    m_device = nullptr;
  }
}
