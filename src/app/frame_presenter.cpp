// TODO: NEEDS REVIEW
#include "frame_presenter.hpp"

#include "vulkan_presenter.hpp"
#ifdef _WIN32
#include "d3d_presenter.hpp"
#endif

std::unique_ptr<IFramePresenter> makeFramePresenter(const QSGRendererInterface::GraphicsApi api) {
#ifdef _WIN32
  if (api == QSGRendererInterface::Direct3D11) {
    return std::make_unique<D3dPresenter>();
  }

  if (api == QSGRendererInterface::Vulkan) {
    return std::make_unique<VulkanPresenter>();
  }
#else
  (void)api;
#endif

  return nullptr;
}
