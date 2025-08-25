#include "gallery_service.hpp"

namespace firelight::media {
bool GalleryService::saveToGallery(Image &image, std::string contentHash,
                                   int saveSlotNumber) {
  return true;
}
std::vector<GalleryItem> GalleryService::listGalleryItems() { return {}; }
} // namespace firelight::media