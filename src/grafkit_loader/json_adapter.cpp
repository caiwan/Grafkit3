#include "stdafx.h"

#include "grafkit_loader/json_adapter.h"
#include "json/json_registry.h"

Grafkit::Asset::JsonAsset::JsonAsset(std::vector<uint8_t> data)
	: m_data(std::move(data))
{
}

void Grafkit::Asset::JsonAsset::Deserialize(const std::type_index& assetType, std::any& object)
{
	nlohmann::json json = nlohmann::json::parse(m_data);
	Serialization::JsonSerializerRegistry::Instance().Deserialize(json, assetType, object);
}

void Grafkit::Asset::JsonAsset::ReadData(std::vector<uint8_t>& data) const
{
	std::copy(m_data.begin(), m_data.end(), std::back_inserter(data));
}
