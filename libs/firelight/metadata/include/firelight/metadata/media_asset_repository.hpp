#pragma once

#include <firelight/metadata/media_asset.hpp>

#include <optional>
#include <string>
#include <vector>

namespace firelight::metadata {
/**
 * Stores and retrieves media assets for games. You can get all the user's assets for a game, or just the currently
 * selected one for a type. Only one asset can be selected for a given (game, type) at a time. Assets are
 * identified by their content hash, type, and source
 */
class IMediaAssetRepository {
public:
  virtual ~IMediaAssetRepository() = default;

  /**
   * @param contentHash The content hash of the game to list assets for
   * @return The list of all assets for the given content hash, regardless of type or selection status
   */
  [[nodiscard]] virtual std::vector<MediaAsset> listForContent(const std::string &contentHash) = 0;

  /**
   * @param contentHash The content hash of the game to list assets for
   * @param type The type of asset to list
   * @return The list of all assets for the given content hash matching the given type
   */
  [[nodiscard]] virtual std::vector<MediaAsset> listForContentAndType(const std::string &contentHash,
                                                                      MediaType type) = 0;

  /**
   * @param contentHash The content hash of the game to get the selected asset for
   * @param type The type of asset to get the selected asset for
   * @return The currently selected asset for the given content hash and type, or std::nullopt if none is selected
   */
  [[nodiscard]] virtual std::optional<MediaAsset> selectedFor(const std::string &contentHash, MediaType type) = 0;

  /**
   * Upserts a media asset. If an asset with the same content hash, type, and source already exists, it will be
   * updated with the new data. Otherwise, a new asset will be created. The id field of the asset will be set to the
   * id of the existing or newly created asset. If the asset is marked as selected, it will become the selected asset
   * for its (contentHash, type)
   *
   * @param asset The asset to add or update. Its id field will be set to the id of the existing or newly created
   *   asset
   * @return True if the asset was added or updated successfully, false otherwise
   */
  virtual bool add(MediaAsset &asset) = 0;

  /**
   * Mark a specific asset as selected for its (contentHash, type). This will unselect any other asset that is
   * currently selected for the same (contentHash, type)
   *
   * @param assetId The asset id to mark as selected
   * @return True if the asset was marked as selected successfully, false otherwise
   */
  virtual bool setSelected(int assetId) = 0;

  /**
   * Removes a media asset by its id. If the asset is currently selected, it will be unselected
   *
   * @param assetId The id of the asset to remove
   * @return True if the asset was removed successfully, false otherwise
   */
  virtual bool remove(int assetId) = 0;
};
} // namespace firelight::metadata
