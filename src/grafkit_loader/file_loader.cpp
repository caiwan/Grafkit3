#include "stdafx.h"

#include <filesystem>
#include <fstream>

#include <grafkit_loader/file_loader.h>

namespace {
	std::string AssetTypenameLookup(const std::type_index& assetType)
	{
		std::unordered_map<std::string, std::string> typeMap = {};

		const auto it = typeMap.find(assetType.name());
		if (it != typeMap.end()) {
			return it->second;
		}

		return {};
	}
} // namespace

void Grafkit::Asset::FileAssetSource::ReadData(
	[[maybe_unused]] std::type_index assetType, const std::string& assetName, std::vector<uint8_t>& data) const
{
	std::filesystem::path assetTypeStr = std::filesystem::path(AssetTypenameLookup(assetType));
	std::filesystem::path assetPath = assetTypeStr / assetName;

	std::ifstream file(assetPath, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("File not found: " + assetPath.string());
	}

	const auto size = file.tellg(); // position_tpe

	file.seekg(0, std::ios::beg);

	data.resize(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(data.data()), size);
}
