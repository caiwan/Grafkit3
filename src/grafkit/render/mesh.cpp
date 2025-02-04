#include "stdafx.h"

#include "grafkit/core/command_buffer.h"
#include "grafkit/render/mesh.h"

namespace Grafkit
{

	// MARK: Mesh
	Mesh::Mesh(const Core::DeviceRef &device,
		uint32_t id,
		const Core::Buffer &vertexBuffer,
		const Core::Buffer &indexBuffer,
		std::vector<Primitive> primitives,
		std::unordered_map<uint32_t, MaterialPtr> materials)
		: m_device(device)
		, m_id(id)
		, m_vertexBuffer(vertexBuffer)
		, m_indexBuffer(indexBuffer)
		, m_primitives(std::move(primitives))
		, m_materials(std::move(materials))
	{
	}

	Mesh::~Mesh()
	{
		m_indexBuffer.Destroy(m_device);
		m_vertexBuffer.Destroy(m_device);
	}

	MeshPtr Mesh::Create(const Core::DeviceRef &device,
		const std::vector<Vertex> &vertices,
		const std::vector<uint32_t> &indices,
		std::vector<Primitive> primitives,
		std::unordered_map<uint32_t, MaterialPtr> materials)
	{
		// Create vertex buffer
		Core::Buffer vertexBuffer = Core::Buffer::CreateBuffer(device,
			sizeof(vertices[0]) * vertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);
		vertexBuffer.Update(device, vertices.data(), sizeof(vertices[0]) * vertices.size());

		// Create index buffer
		Core::Buffer indexBuffer = Core::Buffer::CreateBuffer(device,
			sizeof(indices[0]) * indices.size(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);
		indexBuffer.Update(device, indices.data(), sizeof(indices[0]) * indices.size());

		return std::make_shared<Mesh>(device,
			0,
			vertexBuffer,
			indexBuffer,
			std::move(primitives),
			std::move(materials));
	}

	// MARK: FullScreenQuad
	FullScreenQuad::FullScreenQuad(const Core::DeviceRef &device, Core::Buffer &vertexBuffer)
		: m_device(device)
		, m_vertexBuffer(vertexBuffer)
	{
	}

	FullScreenQuad::~FullScreenQuad()
	{
		m_vertexBuffer.Destroy(m_device);
	}

	std::shared_ptr<FullScreenQuad> FullScreenQuad::Create(const Core::DeviceRef &device)
	{
		std::vector<FullScreenQuad::Vertex> vertices = {{}, {}, {}};
		Core::Buffer vertexBuffer = Core::Buffer::CreateBuffer(device,
			sizeof(vertices[0]) * sizeof(vertices),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU);
		vertexBuffer.Update(device, vertices.data(), sizeof(vertices[0]) * sizeof(vertices));

		return std::make_shared<FullScreenQuad>(device, vertexBuffer);
	}

	void Grafkit::FullScreenQuad::Draw(const Core::CommandBufferRef &commandBuffer) const
	{
		std::array<VkDeviceSize, 1> offsets = {0};
		vkCmdBindVertexBuffers(**commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets.data());
		vkCmdDraw(**commandBuffer, 3, 1, 0, 0);
	}
} // namespace Grafkit
