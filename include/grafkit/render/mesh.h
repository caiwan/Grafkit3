#ifndef GRAFKIT_MESH_H
#define GRAFKIT_MESH_H

#include <memory>
#include <tuple>
#include <vector>

#include "grafkit/core/buffer.h"
#include "grafkit/render/mesh.h"
#include <grafkit/common.h>

namespace Grafkit {
	struct CameraView {
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 camera;
	};

	struct ModelView {
		alignas(16) glm::mat4 model;
	};

	struct Vertex {
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec2 uv = glm::vec2(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);

		// TODO: This will be generated
		static inline Core::VertexDescription GetVertexDescription(
			const uint32_t inputBinding = 0, const uint32_t vertexbinding = 0)
		{
			return { .bindings = {
				{
					inputBinding,
					sizeof(Vertex), // stride
					VK_VERTEX_INPUT_RATE_VERTEX // inputRate
				},
			}, .attributes = {
				{
					0, // position
					vertexbinding,
					VK_FORMAT_R32G32B32_SFLOAT, // format
					offsetof(Vertex, position), // offset
				},
				{
					1, // color
					vertexbinding,
					VK_FORMAT_R32G32B32_SFLOAT, // format
					offsetof(Vertex, color), // offset
				},
				{
					2, // uv
					vertexbinding,
					VK_FORMAT_R32G32_SFLOAT, // format
					offsetof(Vertex, uv), // offset
				},
				{
					3, // normal
					vertexbinding,
					VK_FORMAT_R32G32B32_SFLOAT, // format
					offsetof(Vertex, normal),   // offset
				},
			} };
		}
	};

	struct Primitive {
		uint32_t id;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t vertexOffset;
		uint32_t vertexCount;

		std::shared_ptr<Material> material;

		void Draw(const Core::CommandBufferRef& commandBuffer) const;
	};

	class Mesh {
	public:
		friend class Scenegraph;
		Mesh(const Core::DeviceRef& device);
		~Mesh();

		void Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		void AddPrimitive(const Primitive& primitive) { m_primitives.push_back(primitive); }

		void Bind(const Core::CommandBufferRef& commandBuffer, uint32_t vertexOffset) const;

	private:
		void Destroy(const Core::DeviceRef& device);

		const Core::DeviceRef m_device;

		uint32_t m_id;
		std::vector<Primitive> m_primitives;
		Core::Buffer m_vertexBuffer {};
		Core::Buffer m_indexBuffer {};
	};

	class FullScreenQuad {
	public:
		FullScreenQuad(const Core::DeviceRef& device);
		~FullScreenQuad();

		void Create();
		void Bind(const Core::CommandBufferRef& commandBuffer) const;
		void Draw(const Core::CommandBufferRef& commandBuffer) const;

		struct Vertex {
			glm::vec3 position {};

			static inline Core::VertexDescription GetVertexDescription(
				const uint32_t inputBinding = 0, const uint32_t vertexbinding = 0)
			{
				return { .bindings = {
					{
						inputBinding,
						sizeof(Vertex), // stride
						VK_VERTEX_INPUT_RATE_VERTEX // inputRate
					},
				}, .attributes = {
					{
						0, // position
						vertexbinding,
						VK_FORMAT_R32G32_SFLOAT, // format
						offsetof(Vertex, position), // offset
					},
				} };
			}
		};

	private:
		const Core::DeviceRef m_device;
		Core::Buffer m_vertexBuffer {};
	};

	using FullScreenQuadPtr = std::shared_ptr<FullScreenQuad>;

} // namespace Grafkit

#endif // _GRAFKIT_MESH_H_
