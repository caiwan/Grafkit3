#include "stdafx.h"

#include <filesystem>
#include <fstream>

#include <grafkit_loader/file_loader.h>

void Grafkit::Asset::FileAssetSource::ReadData(const std::string& assetName, std::vector<uint8_t>& data) const
{
	std::filesystem::path assetPath = assetName;

	std::ifstream file(assetPath, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("File not found: " + assetPath.string());
	}

	std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(data));
}
