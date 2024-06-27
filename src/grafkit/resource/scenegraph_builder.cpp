#include "stdafx.h"
#include <grafkit/render/material.h>
#include <grafkit/render/mesh.h>
#include <grafkit/resource/scenegraph_builder.h>

void Grafkit::Resource::MeshBuilder::Build(const Core::DeviceRef& device)
{
	std::vector<Grafkit::Primitive> primitives;
	std::vector<Grafkit::Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto& primitiveDesc : m_descriptor.primitives) {
		primitives.push_back({ .id = static_cast<uint32_t>(primitives.size()),
			.indexOffset = static_cast<uint32_t>(indices.size()),
			.indexCount = static_cast<uint32_t>(primitiveDesc.indices.size()),
			.vertexOffset = static_cast<uint32_t>(vertices.size()),
			.vertexCount = static_cast<uint32_t>(primitiveDesc.vertices.size()),
			.material = primitiveDesc.material });

		if (primitives.back().material == nullptr) {
			throw std::runtime_error("Error: Material is null");
		}

		vertices.insert(vertices.end(), primitiveDesc.vertices.begin(), primitiveDesc.vertices.end());
		std::transform(primitiveDesc.indices.begin(),
			primitiveDesc.indices.end(),
			std::back_inserter(indices),
			[offset = indices.size()](uint32_t index) { return index + offset; });
	}

	m_resource = std::make_shared<Mesh>(device);
	m_resource->Create(vertices, indices);

	for (const auto& primitive : primitives) {
		m_resource->AddPrimitive(primitive);
	}
}
