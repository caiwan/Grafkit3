#include "stdafx.h"

#include <fstream>

#include <grafkit_loader/file_loader.h>

void Grafkit::Asset::FileAssetSource::ReadData(
	std::type_index assetType, const std::string& assetName, std::vector<uint8_t>& data) const
{
	std::string assetTypeStr = assetType.name();
	std::string assetPath = assetTypeStr + "/" + assetName; // TODO: use path library

	std::ifstream file(assetPath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("File not found: " + assetName);
	}

	const auto size = file.tellg(); // position_tpe

	file.seekg(0, std::ios::beg);

	data.resize(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(data.data()), size);
}
