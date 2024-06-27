#include "stdafx.h"
#include <grafkit/render/mesh.h>

using namespace Grafkit;

Mesh::Mesh(const Core::DeviceRef& device)
	: m_device(device)
{
}

Mesh::~Mesh() { Destroy(m_device); }

void Mesh::Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	// Create vertex buffer
	m_vertexBuffer = Core::Buffer::CreateBuffer(m_device,
		sizeof(vertices[0]) * vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU);
	m_vertexBuffer.Update(m_device, vertices.data(), sizeof(vertices[0]) * vertices.size());

	// Create index buffer
	m_indexBuffer = Core::Buffer::CreateBuffer(
		m_device, sizeof(indices[0]) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	m_indexBuffer.Update(m_device, indices.data(), sizeof(indices[0]) * indices.size());
}

void Grafkit::Mesh::Destroy(const Core::DeviceRef& device)
{
	for (auto& primitive : m_primitives) {
		primitive.material.reset();
	}

	m_primitives.clear();
	m_vertexBuffer.Destroy(device);
	m_indexBuffer.Destroy(device);
}
