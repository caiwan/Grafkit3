#include "stdafx.h"

#include <grafkit_loader/json_deserializer.h>

namespace {

}

void Grafkit::Asset::JsonAsset::Deserialize(std::any& object) { nlohmann::json json = nlohmann::json::parse(m_data); }
