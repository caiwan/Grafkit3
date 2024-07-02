#ifndef GRAFKIT_SCENEGRAPH_BUILDER_H
#define GRAFKIT_SCENEGRAPH_BUILDER_H

#include <unordered_map>

#include <grafkit/common.h>
#include <grafkit/render/material.h>
#include <grafkit/render/mesh.h>
#include <grafkit/resource/resource.h>

namespace Grafkit::Resource {

	struct PrimitiveDesc {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t materialIndex;
	};

	struct MeshDesc {
		std::vector<PrimitiveDesc> primitives;
		std::unordered_map<uint32_t, std::string> materials;
	};

	class MeshBuilder : public ResourceBuilder<MeshDesc, Grafkit::Mesh> {
	public:
		explicit MeshBuilder(const MeshDesc& desc = {})
			: ResourceBuilder(desc)
		{
		}

		MeshBuilder& AddPrimitive(const PrimitiveDesc& primitive)
		{
			m_descriptor.primitives.push_back(primitive);
			return *this;
		}

		MeshBuilder& AddPrimitive(
			const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t materialIndex)
		{
			m_descriptor.primitives.push_back({ vertices, indices, materialIndex });
			return *this;
		}

		MeshBuilder& AddMaterial(const uint32_t index, const MaterialPtr& material)
		{
			m_materials[index] = material;
			return *this;
		}

		// TODO: Combine primitives to a single mesh - vertices + indice, then index range + material

		[[nodiscard]] bool ResolveDependencies(const RefWrapper<ResourceManager>& resources) final;
		void Build(const Core::DeviceRef& device) final;

	private:
		std::unordered_map<uint32_t, MaterialPtr> m_materials;
	};

	struct NodeDesc {
		std::string name;
		std::vector<uint32_t> meshIndices;
		std::vector<NodeDesc> children;
	};

	struct SceneGraphDesc {
		std::vector<NodeDesc> nodes;
	};

	// class SceneGraphBuilder : public ResourceBuilder<SceneGraphDesc, Grafkit::Scenegraph> {
	// public:
	// 	explicit SceneGraphBuilder(const SceneGraphDesc& desc = {})
	// 		: ResourceBuilder(desc)
	// 	{
	// 	}

	// 	SceneGraphBuilder & AddMesh(const Mesh& mesh)
	// 	{

	// 		return *this;
	// 	}

	// 	void Build(const Core::DeviceRef& device) override;
	// };

} // namespace Grafkit::Resource

#endif // SCENEGRAPH_BUILDER_H
