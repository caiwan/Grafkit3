/**
 * @file json_serializers.cpp
 * @brief Json serialization functions
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated at: 2025-03-26 12:50:25
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

// MARK: animation_desc
namespace Grafkit::Resource
{
} // namespace Grafkit::Resource

// MARK: image_desc
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

// MARK: material_desc
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const MaterialDesc &obj)
	{
		j["name"] = obj.name;
		j["type"] = obj.type;
		j["stage"] = obj.stage;
		j["textures"] = obj.textures;
	}

	void from_json(const nlohmann::json &j, MaterialDesc &obj)
	{
		obj.name = j["name"].get<std::string>();
		obj.type = j["type"].get<uint32_t>();
		obj.stage = j["stage"].get<std::string>();
		obj.textures = j["textures"].get<std::map<TextureType, std::string>>();
	}
} // namespace Grafkit::Resource

// MARK: mesh_desc
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const PrimitiveDesc &obj)
	{
		j["positions"] = obj.positions;
		j["normals"] = obj.normals;
		j["tangents"] = obj.tangents;
		j["bitangents"] = obj.bitangents;
		j["texCoords"] = obj.texCoords;
		j["indices"] = obj.indices;
		j["materialIndex"] = obj.materialIndex;
	}

	void from_json(const nlohmann::json &j, PrimitiveDesc &obj)
	{
		obj.positions = j["positions"].get<std::vector<glm::vec3>>();
		obj.normals = j["normals"].get<std::vector<glm::vec3>>();
		obj.tangents = j["tangents"].get<std::vector<glm::vec3>>();
		obj.bitangents = j["bitangents"].get<std::vector<glm::vec3>>();
		obj.texCoords = j["texCoords"].get<std::vector<glm::vec2>>();
		obj.indices = j["indices"].get<std::vector<uint32_t>>();
		obj.materialIndex = j["materialIndex"].get<uint32_t>();
	}

	void to_json(nlohmann::json &j, const MeshDesc &obj)
	{
		j["name"] = obj.name;
		j["primitives"] = obj.primitives;
		j["materials"] = obj.materials;
		j["type"] = obj.type;
	}

	void from_json(const nlohmann::json &j, MeshDesc &obj)
	{
		obj.name = j["name"].get<std::string>();
		obj.primitives = j["primitives"].get<std::vector<PrimitiveDesc>>();
		obj.materials = j["materials"].get<std::map<std::string, uint32_t>>();
		obj.type = j["type"].get<MeshType>();
	}

	void to_json(nlohmann::json &j, const PrimitiveDescV2 &obj)
	{
		j["indexOffset"] = obj.indexOffset;
		j["indexCount"] = obj.indexCount;
		j["vertexOffset"] = obj.vertexOffset;
		j["vertexCount"] = obj.vertexCount;
		j["materialIndex"] = obj.materialIndex;
	}

	void from_json(const nlohmann::json &j, PrimitiveDescV2 &obj)
	{
		obj.indexOffset = j["indexOffset"].get<uint32_t>();
		obj.indexCount = j["indexCount"].get<uint32_t>();
		obj.vertexOffset = j["vertexOffset"].get<uint32_t>();
		obj.vertexCount = j["vertexCount"].get<uint32_t>();
		obj.materialIndex = j["materialIndex"].get<uint32_t>();
	}

	void to_json(nlohmann::json &j, const MeshDescV2 &obj)
	{
		j["name"] = obj.name;
		j["positions"] = obj.positions;
		j["normals"] = obj.normals;
		j["tangents"] = obj.tangents;
		j["bitangents"] = obj.bitangents;
		j["texCoords"] = obj.texCoords;
		j["indices"] = obj.indices;
		j["primitives"] = obj.primitives;
		j["materials"] = obj.materials;
		j["type"] = obj.type;
	}

	void from_json(const nlohmann::json &j, MeshDescV2 &obj)
	{
		obj.name = j["name"].get<std::string>();
		obj.positions = j["positions"].get<std::vector<glm::vec3>>();
		obj.normals = j["normals"].get<std::vector<glm::vec3>>();
		obj.tangents = j["tangents"].get<std::vector<glm::vec3>>();
		obj.bitangents = j["bitangents"].get<std::vector<glm::vec3>>();
		obj.texCoords = j["texCoords"].get<std::vector<glm::vec2>>();
		obj.indices = j["indices"].get<std::vector<uint32_t>>();
		obj.primitives = j["primitives"].get<std::vector<PrimitiveDescV2>>();
		obj.materials = j["materials"].get<std::map<std::string, uint32_t>>();
		obj.type = j["type"].get<MeshType>();
	}
} // namespace Grafkit::Resource

// MARK: scene_desc
namespace Grafkit::Resource
{
} // namespace Grafkit::Resource

// NOLINTEND(readability-identifier-naming)
