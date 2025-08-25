#pragma once
#include "media/image.hpp"

namespace firelight::media {

enum GalleryItemType { IMAGE, VIDEO };

struct GalleryItem {
  std::string contentHash;
  int saveSlotNumber;

  GalleryItemType type = IMAGE;
  Image image;
};

class GalleryService {
public:
  bool saveToGallery(Image &image, std::string contentHash, int saveSlotNumber);
  std::vector<GalleryItem> listGalleryItems();
};

} // namespace firelight::media