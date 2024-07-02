#include "stdafx.h"

#include <grafkit_loader/json_deserializer.h>

namespace {
	std::any LoadJson(const nlohmann::json& json, const std::type_index& type)
	{
		if (type == typeid(nlohmann::json)) {
			return json;
		}
		if (type == typeid(std::string)) {
			return json.dump();
		}
		if (type == typeid(int)) {
			return json.get<int>();
		}
		if (type == typeid(float)) {
			return json.get<float>();
		}
		if (type == typeid(double)) {
			return json.get<double>();
		}
		if (type == typeid(bool)) {
			return json.get<bool>();
		}
		if (type == typeid(std::vector<int>)) {
			return json.get<std::vector<int>>();
		}
		if (type == typeid(std::vector<float>)) {
			return json.get<std::vector<float>>();
		}
		if (type == typeid(std::vector<double>)) {
			return json.get<std::vector<double>>();
		}
		if (type == typeid(std::vector<bool>)) {
			return json.get<std::vector<bool>>();
		}
		if (type == typeid(std::vector<std::string>)) {
			return json.get<std::vector<std::string>>();
		}
		if (type == typeid(std::vector<nlohmann::json>)) {
			return json.get<std::vector<nlohmann::json>>();
		}
		if (type == typeid(std::vector<std::any>)) {
			std::vector<std::any> result;
			for (const auto& item : json) {
				result.push_back(LoadJson(item, typeid(nlohmann::json)));
			}
			return result;
		}
		throw std::runtime_error(std::string("Unsupported type: ") + type.name());
	}
} // namespace

void Grafkit::Asset::JsonAsset::Deserialize(std::any& object)
{
	nlohmann::json json = nlohmann::json::parse(m_data);
	object = LoadJson(json, m_assetType);
}
