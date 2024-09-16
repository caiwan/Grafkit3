#include "stdafx.h"

#include "json_registry.h"

using namespace Grafkit::Serialization;

// MARK: JsonDeserializerRegistry
JsonSerializerRegistry::JsonSerializerRegistry()
{
	// // TOOD: This will be a generated code
	// RegisterDeserializer<Grafkit::Resource::MaterialDesc>(
	// 	[](const nlohmann::json& json, std::any& object) { object = json.get<Grafkit::Resource::MaterialDesc>(); });
}

void JsonSerializerRegistry::Register(std::type_index type, DeserializerFunc deserializer)
{
	m_deserializers[type.name()] = std::move(deserializer);
}

void JsonSerializerRegistry::Deserialize(const nlohmann::json& json, std::type_index type, std::any& object)
{
	auto it = m_deserializers.find(type.name());
	if (it != m_deserializers.end()) {
		it->second(json, object);
	} else {
		throw std::runtime_error(std::string("No deserializer found for type: ") + type.name());
	}
}
