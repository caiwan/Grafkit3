#ifndef ASSET_JSON_DESERIALIZER_H
#define ASSET_JSON_DESERIALIZER_H

#include <any>
#include <typeindex>
#include <vector>

#include <grafkit/common.h>
#include <grafkit/interface/asset.h>

namespace Grafkit::Asset {
	class GKAPI JsonAsset : virtual public ISerializedAsset {
	public:
		JsonAsset(std::vector<uint8_t> data);
		~JsonAsset() override = default;

		void Deserialize(const std::type_index& assetType, std::any& object) override;

		void ReadData(std::vector<uint8_t>& data) const override;

	private:
		const std::vector<uint8_t> m_data;
	};
} // namespace Grafkit::Asset
#endif // ASSET_JSON_DESERIALIZER_H
