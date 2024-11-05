/**
 * @file json_serializers.cpp
 * @brief Json serialization functions
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated at: 2024-09-24 13:10:26
 * Source files:
 *   - animation_desc.gen.yaml
 *   - image_desc.gen.yaml
 *   - material_desc.gen.yaml
 *   - mesh_desc.gen.yaml
 *   - scene_desc.gen.yaml
 * Template file: json_template.j2
 */

#include "stdafx.h"

#include <grafkit/common.h>
#include <map>
#include <span>
#include <string>
#include <vector>

#include "json/generated/json_serializers.h"
#include "json/json_glm.h"

// NOLINTBEGIN(readability-identifier-naming) The naming has to match with nlohmann_json

/* animation_desc */
namespace Grafkit::Resource
{
} // namespace Grafkit::Resource

/* image_desc */
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const ImageDesc &obj)
	{
		j["image"] = obj.image;
		j["size"] = obj.size;
		j["format"] = obj.format;
		j["channels"] = obj.channels;
		j["useMipmap"] = obj.useMipmap;
	}

	void from_json(const nlohmann::json &j, ImageDesc &obj)
	{
		obj.image = j["image"].get<std::vector<uint8_t>>();
		obj.size = j["size"].get<glm::uvec3>();
		obj.format = j["format"].get<ImageFormat>();
		obj.channels = j["channels"].get<uint32_t>();
		obj.useMipmap = j["useMipmap"].get<bool>();
	}

	void to_json(nlohmann::json &j, const SolidImageDesc &obj)
	{
		j["color"] = obj.color;
	}

	void from_json(const nlohmann::json &j, SolidImageDesc &obj)
	{
		obj.color = j["color"].get<glm::u8vec4>();
	}

	void to_json(nlohmann::json &j, const CheckerImageDesc &obj)
	{
		j["size"] = obj.size;
		j["divisions"] = obj.divisions;
		j["color1"] = obj.color1;
		j["color2"] = obj.color2;
		j["useMipmap"] = obj.useMipmap;
	}

	void from_json(const nlohmann::json &j, CheckerImageDesc &obj)
	{
		obj.size = j["size"].get<glm::uvec3>();
		obj.divisions = j["divisions"].get<glm::uvec2>();
		obj.color1 = j["color1"].get<glm::u8vec4>();
		obj.color2 = j["color2"].get<glm::u8vec4>();
		obj.useMipmap = j["useMipmap"].get<bool>();
	}
} // namespace Grafkit::Resource

/* material_desc */
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const MaterialDesc &obj)
	{
		j["name"] = obj.name;
		j["type"] = obj.type;
		j["pipeline"] = obj.pipeline;
		j["textures"] = obj.textures;
	}

	void from_json(const nlohmann::json &j, MaterialDesc &obj)
	{
		obj.name = j["name"].get<std::string>();
		obj.type = j["type"].get<uint32_t>();
		obj.pipeline = j["pipeline"].get<std::string>();
		obj.textures = j["textures"].get<std::map<TextureType, std::string>>();
	}
} // namespace Grafkit::Resource

/* mesh_desc */
namespace Grafkit::Resource
{
} // namespace Grafkit::Resource

/* scene_desc */
namespace Grafkit::Resource
{
} // namespace Grafkit::Resource

// NOLINTEND(readability-identifier-naming)
