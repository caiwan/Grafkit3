#ifndef GRAFKIT_SCENEGRAPH_BUILDER_H
#define GRAFKIT_SCENEGRAPH_BUILDER_H

#include <grafkit/common.h>
#include <grafkit/render/mesh.h>
#include <grafkit/resource/resource.h>

namespace Grafkit {
	namespace Resource {

		struct PrimitiveDesc {
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			Grafkit::MaterialPtr material;
		};

		struct MeshDesc {
			std::vector<PrimitiveDesc> primitives;
		};

		class MeshBuilder : public ResourceBuilder<MeshDesc, Grafkit::Mesh> {
		public:
			explicit MeshBuilder(const MeshDesc& desc = {})
				: ResourceBuilder(desc)
			{
			}

			virtual void Build(const Core::DeviceRef& device) override;
		};

		// // ...
		// struct SceneGraphDesc { };

		// class SceneGraphBuilder : public ResourceBuilder<SceneGraphDesc, Grafkit::SceneGraph> {
		// public:
		// 	explicit SceneGraphBuilder(const SceneGraphDesc& desc = {})
		// 		: ResourceBuilder(desc)
		// 	{
		// 	}

		// 	virtual void Build(const Core::DeviceRef& device, const ResoureMangerRef& resources) override;
		// };

	} // namespace Resource

} // namespace Grafkit

#endif // SCENEGRAPH_BUILDER_H
