/**
 * @file json_register_generated.h
 * @brief Json serialization functions
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated at: 2024-12-07 16:44:54
 * Source files:
 *   - animation_desc.gen.yaml
 *   - image_desc.gen.yaml
 *   - material_desc.gen.yaml
 *   - mesh_desc.gen.yaml
 *   - scene_desc.gen.yaml
 * Template file: json_register.j2
 */

#include "json_serializers.h"
#include "json/json_registry.h"

namespace Grafkit::Serialization
{

	inline void RegisterJsonDeserializers(JsonSerializerRegistry &registry)
	{
		// MARK: animation_desc

		// MARK: image_desc

		registry.Register<Grafkit::Resource::ImageDesc>(
			[](const nlohmann::json &json, std::any &object) { object = json.get<Grafkit::Resource::ImageDesc>(); });

		registry.Register<Grafkit::Resource::SolidImageDesc>([](const nlohmann::json &json, std::any &object)
			{ object = json.get<Grafkit::Resource::SolidImageDesc>(); });

		registry.Register<Grafkit::Resource::CheckerImageDesc>([](const nlohmann::json &json, std::any &object)
			{ object = json.get<Grafkit::Resource::CheckerImageDesc>(); });

		// MARK: material_desc

		registry.Register<Grafkit::Resource::MaterialDesc>(
			[](const nlohmann::json &json, std::any &object) { object = json.get<Grafkit::Resource::MaterialDesc>(); });

		// MARK: mesh_desc

		registry.Register<Grafkit::Resource::PrimitiveDesc>([](const nlohmann::json &json, std::any &object)
			{ object = json.get<Grafkit::Resource::PrimitiveDesc>(); });

		registry.Register<Grafkit::Resource::MeshDesc>(
			[](const nlohmann::json &json, std::any &object) { object = json.get<Grafkit::Resource::MeshDesc>(); });

		registry.Register<Grafkit::Resource::PrimitiveDescV2>([](const nlohmann::json &json, std::any &object)
			{ object = json.get<Grafkit::Resource::PrimitiveDescV2>(); });

		registry.Register<Grafkit::Resource::MeshDescV2>(
			[](const nlohmann::json &json, std::any &object) { object = json.get<Grafkit::Resource::MeshDescV2>(); });

		// MARK: scene_desc
	}

} // namespace Grafkit::Serialization
