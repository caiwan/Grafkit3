#ifndef ASSET_FILE_LOADER_H
#define ASSET_FILE_LOADER_H

#include <typeindex>

#include <grafkit/common.h>
#include <grafkit/renderer/asset.h>

namespace Grafkit::Asset {

	class GKAPI FileAssetSource : virtual public IAssetSource {
	public:
		FileAssetSource() = default;
		FileAssetSource(const FileAssetSource&) = delete; // Delete copy constructor
		FileAssetSource& operator=(const FileAssetSource&) = delete; // Delete copy assignment operator

		~FileAssetSource() override = default;

		void ReadData(
			std::type_index assetType, const std::string& assetName, std::vector<uint8_t>& data) const override;
	};
} // namespace Grafkit::Asset
#endif // ASSET_FILE_LOADER_H
