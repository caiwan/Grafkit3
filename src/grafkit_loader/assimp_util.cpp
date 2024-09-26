#include "stdafx.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wimplicit-float-conversion"
#pragma clang diagnostic ignored "-Wdeprecated-copy"
#endif

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "assimp_util.h"

std::string Grafkit::Asset::Assimp::GetMaterialName(aiScene* aiscene, uint32_t index)
{
	if (index < aiscene->mNumMaterials) {
		aiString name;
		aiMaterial* srcMaterial = aiscene->mMaterials[index];

		if (srcMaterial->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
			return name.C_Str();
		}

		std::stringstream ss;
		ss << "material_" << index;
		return ss.str();
	}

	return {};
}

glm::vec4 Grafkit::Asset::Assimp::Matkey4ToFloat4(
	[[maybe_unused]] const aiMaterial* mat, [[maybe_unused]] const char* key)
{
	// TODO: Implement this
	// aiColor4D color;
	// if (mat->Get(key, color) == AI_SUCCESS) {
	// 	return glm::vec4(color.r, color.g, color.b, color.a);
	// }
	return glm::vec4(0.0f);
}
