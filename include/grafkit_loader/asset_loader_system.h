#ifndef ASSET_LOADER_SYSTEM_H
#define ASSET_LOADER_SYSTEM_H

#include <grafkit/common.h>

#include <grafkit_loader/file_loader.h>
#include <grafkit_loader/json_adapter.h>

namespace Grafkit::Asset {
	class GKAPI JsonAssetLoader final : virtual public AssetLoader<FileAssetSource, JsonAsset> {
	public:
		JsonAssetLoader() = default;
		JsonAssetLoader(const JsonAssetLoader&) = delete; // Delete copy constructor
		JsonAssetLoader& operator=(const JsonAssetLoader&) = delete; // Delete copy assignment operator

		~JsonAssetLoader() override = default;
	};
} // namespace Grafkit::Asset

#endif // ASSET_LOADER_SYSTEM_H
