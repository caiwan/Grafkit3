#include "stdafx.h"
#include <grafkit/render/material.h>
#include <grafkit/render/mesh.h>
#include <grafkit/resource/scenegraph_builder.h>

using namespace Grafkit;
using namespace Grafkit::Resource;

MeshBuilder &MeshBuilder::AddPrimitive(const std::vector<Vertex> &vertices,
	const std::vector<uint32_t> &indices,
	const uint32_t materialIndex)
{
	m_descriptor.primitives.push_back({
		.vertices = vertices,
		.indices = indices,
		.materialIndex = materialIndex,
	});
	return *this;
}

MeshBuilder &MeshBuilder::AddPrimitive(const std::vector<Vertex> &vertices,
	const std::vector<uint32_t> &indices,
	const MaterialPtr &material)
{
	// TOOD: Check if material already exists
	const auto materialIndex =
		m_materials.empty() ? 0 : std::max_element(m_materials.begin(), m_materials.end())->first + 1;
	m_materials.emplace(materialIndex, material);
	m_descriptor.primitives.push_back({
		.vertices = vertices,
		.indices = indices,
		.materialIndex = materialIndex,
	});
	return *this;
}

MeshBuilder &MeshBuilder::AddMaterial(const uint32_t index, const MaterialPtr &material)
{
	// TODO: Check if material already exists
	// m_materials[index] = material;
	return *this;
}

bool MeshBuilder::ResolveDependencies(const RefWrapper<ResourceManager> &resources)
{
	bool result = true;
	if (m_materials.empty() && m_descriptor.materials.empty())
	{
		for (const auto &[index, materialName] : m_descriptor.materials)
		{
			const auto material = resources->Get<Material>(materialName);
			if (material != nullptr)
			{
				m_materials[index] = material;
			}
			else
			{
				result = false;
			}
		}
	}
	return result;
}

void MeshBuilder::Build(const Core::DeviceRef &device)
{
	std::vector<Grafkit::Primitive> primitives;
	std::vector<Grafkit::Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto &primitiveDesc : m_descriptor.primitives)
	{
		const auto materialIt = m_materials.find(primitiveDesc.materialIndex);

		if (materialIt == m_materials.end())
		{
			throw std::runtime_error("Error: Material is null");
		}

		const auto materialId = materialIt->first;
		const auto material = materialIt->second;

		primitives.push_back({.id = static_cast<uint32_t>(primitives.size()),
			.firstIndex = static_cast<uint32_t>(indices.size()),
			.indexCount = static_cast<uint32_t>(primitiveDesc.indices.size()),
			.vertexOffset = static_cast<uint32_t>(vertices.size()),
			.vertexCount = static_cast<uint32_t>(primitiveDesc.vertices.size()),
			.materialId = materialId});

		vertices.insert(vertices.end(), primitiveDesc.vertices.begin(), primitiveDesc.vertices.end());
		std::transform(primitiveDesc.indices.begin(),
			primitiveDesc.indices.end(),
			std::back_inserter(indices),
			[offset = indices.size()](uint32_t index) { return index + offset; });
	}

	m_resource = Mesh::Create(device, vertices, indices, std::move(primitives), std::move(m_materials));
}
