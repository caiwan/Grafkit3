#ifndef ASSIMP_H
#define ASSIMP_H

#include <grafkit/common.h>
#include <grafkit/interface/resource.h>

namespace Grafkit::Asset {

	// TODO: This has to be an extension to asset/resource loader

	class AssimpLoader {
	public:
		AssimpLoader() = default;
		~AssimpLoader() = default;

		Grafkit::ScenegraphPtr Load(const std::string& filename);
	};
} // namespace Grafkit::Asset
#endif // ASSIMP_H
