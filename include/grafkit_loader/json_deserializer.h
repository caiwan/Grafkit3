#ifndef ASSET_JSON_DESERIALIZER_H
#define ASSET_JSON_DESERIALIZER_H

#include <any>
#include <typeindex>
#include <vector>

#include <grafkit/common.h>
#include <grafkit/resource/asset.h>

namespace Grafkit::Asset {
	class GKAPI JsonAsset : virtual public ISerializedAsset {
	public:
		JsonAsset(const std::type_index assetType, const std::vector<uint8_t>& data)
			: m_data(data)
			, m_assetType(assetType)
		{
		}
		~JsonAsset() override = default;

		void Deserialize(std::any& object) override;

	private:
		const std::vector<uint8_t> m_data;
		const std::type_index m_assetType;
	};
} // namespace Grafkit::Asset
#endif // ASSET_JSON_DESERIALIZER_H
