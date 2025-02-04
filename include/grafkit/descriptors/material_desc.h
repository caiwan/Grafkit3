/**
 * @file material_desc.h
 * @brief material_desc descriptor
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated on: 2025-03-26 12:50:24
 * Source file:
 */

#ifndef __MATERIAL_DESC_GENERATED_H__
#define __MATERIAL_DESC_GENERATED_H__

#include <grafkit/common.h>
#include <map>
#include <string>
#include <vector>

/* material_desc */
namespace Grafkit::Resource
{
	enum class TextureType
	{
		Diffuse = 0,
		Normal = 1,
		Roughness = 2,
		Metallic = 3,
		AmbientOcclusion = 4,
		Emissive = 5,
	};

	struct MaterialDesc
	{
		std::string name;
		uint32_t type;
		std::string stage;
		std::map<TextureType, std::string> textures;
	};

} // namespace Grafkit::Resource
#endif // __MATERIAL_DESC_GENERATED_H__
