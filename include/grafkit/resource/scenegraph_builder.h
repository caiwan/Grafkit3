#ifndef GRAFKIT_SCENEGRAPH_BUILDER_H
#define GRAFKIT_SCENEGRAPH_BUILDER_H

#include <grafkit/common.h>
#include <grafkit/render/mesh.h>
#include <grafkit/resource/resource.h>

namespace Grafkit::Resource {

	struct PrimitiveDesc {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct MeshDesc {
		std::vector<PrimitiveDesc> primitives;
	};

	struct NodeDesc {
		std::string name;
		std::vector<uint32_t> meshIndices;
		std::vector<NodeDesc> children;
	};

	struct SceneGraphDesc {
		std::vector<MeshDesc> meshes;
		std::vector<NodeDesc> nodes;
	};

	class SceneGraphBuilder : public ResourceBuilder<SceneGraphDesc, Grafkit::Scenegraph> {
	public:
		explicit SceneGraphBuilder(const SceneGraphDesc& desc = {})
			: ResourceBuilder(desc)
		{
		}

		void Build(const Core::DeviceRef& device) override;
	};

} // namespace Grafkit::Resource

#endif // SCENEGRAPH_BUILDER_H
