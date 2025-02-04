/**
 * @file mesh_desc.h
 * @brief mesh_desc descriptor
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated on: 2025-03-26 12:50:24
 * Source file:
 */

#ifndef __MESH_DESC_GENERATED_H__
#define __MESH_DESC_GENERATED_H__

#include <grafkit/common.h>
#include <map>
#include <string>
#include <vector>

/* mesh_desc */
namespace Grafkit::Resource
{
	enum class MeshType
	{
		Static = 1,
		Skinned = 2,
	};

	struct PrimitiveDesc
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<glm::vec2> texCoords;
		std::vector<uint32_t> indices;
		uint32_t materialIndex;
	};

	struct MeshDesc
	{
		std::string name;
		std::vector<PrimitiveDesc> primitives;
		std::map<std::string, uint32_t> materials;
		MeshType type;
	};

	struct PrimitiveDescV2
	{
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t materialIndex;
	};

	struct MeshDescV2
	{
		std::string name;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<glm::vec2> texCoords;
		std::vector<uint32_t> indices;
		std::vector<PrimitiveDescV2> primitives;
		std::map<std::string, uint32_t> materials;
		MeshType type;
	};

} // namespace Grafkit::Resource
#endif // __MESH_DESC_GENERATED_H__
