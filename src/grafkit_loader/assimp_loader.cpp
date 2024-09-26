#include "stdafx.h"

#include <grafkit/core/log.h>

#include "assimp_util.h"
#include "grafkit_loader/assimp_loader.h"

using namespace Grafkit::Asset;
using namespace Grafkit::Asset::Assimp;
using Grafkit::Core::Log;

Grafkit::ScenegraphPtr AssimpLoader::Load([[maybe_unused]] const std::string& filename)
{

	// // Load materials
	// if (aiscene->HasMaterials()) {
	// 	Log::Instance().Info("Loading materials");
	// 	for (uint32_t materialIndex = 0; materialIndex < aiscene->mNumMaterials; materialIndex++) {
	// 		// aiMaterial* srcMaterial = aiscene->mMaterials[i];

	// 		// material->SetName(GetMaterialName(i));

	// 		// Log::Instance().Info("Loading material #%d", i);
	// 		// // LOGGER(Log::Logger().Trace("- #%d %s", i, material->GetName().c_str()));

	// 		// aiReturn texFound = AI_FAILURE;

	// 		// // --- SNIP ---
	// 		// // -- -- load textures
	// 		// // textura -> material
	// 		// for (size_t k = 0; k < sizeof(texture_load_map) / sizeof(texture_load_map[0]); k++) {
	// 		// 	for (size_t j = 0; j < srcMaterial->GetTextureCount(texture_load_map[k].ai); j++) {
	// 		// 		LOGGER(Log::Logger().Trace("-- texture #%s #%d", texture_load_map[k].tt, j));
	// 		// 		// TODO: does it work at all ?
	// 		// 		// material->AddTexture(assimpTexture(texture_load_map[k].ai, curr_mat, j, resman),
	// 		// 		// texture_load_map[k].tt);
	// 		// 	}
	// 		// }
	// 		// // --- SMAP ---

	// 		// // there is not much to set up atm
	// 		// material->Colors().diffuse = aiMatkey4ToFloat4(srcMaterial, AI_MATKEY_COLOR_DIFFUSE);
	// 		// material->Colors().specular = aiMatkey4ToFloat4(srcMaterial, AI_MATKEY_COLOR_SPECULAR);

	// 		// env->GetBuilder().AddMaterial(material);
	// 	}
	// }

	return {};
}
