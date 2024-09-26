#ifndef GRAFKIT_LOADER_ASSIMP_PRIVATE_H
#define GRAFKIT_LOADER_ASSIMP_PRIVATE_H

#include <cstdint>
#include <string>

#include <grafkit/common.h>

struct aiScene;
struct aiMaterial;

namespace Grafkit ::Asset ::Assimp {
	std::string GetMaterialName(aiScene* aiscene, uint32_t index);
	glm::vec4 Matkey4ToFloat4(const aiMaterial* mat, const char* key);

} // namespace Grafkit::Asset::Assimp

#endif // GRAFKIT_LOADER_ASSIMP_PRIVATE_H
