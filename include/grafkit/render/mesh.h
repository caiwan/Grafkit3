#ifndef GRAFKIT_MESH_H
#define GRAFKIT_MESH_H

#include <grafkit/common.h>

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "grafkit/core/buffer.h"
#include "grafkit/render/mesh.h"

namespace Grafkit
{
	GKAPI struct CameraView
	{
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 camera;
	};

	GKAPI struct ModelView
	{
		alignas(16) glm::mat4 model;
	};

	GKAPI struct Vertex
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec2 uv = glm::vec2(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);

		// TODO: This will be generated
		static inline Core::VertexDescription GetVertexDescription(const uint32_t inputBinding = 0,
			const uint32_t vertexbinding = 0)
		{
			return {
				.bindings =
					{
						{
							inputBinding,
							sizeof(Vertex),				// stride
							VK_VERTEX_INPUT_RATE_VERTEX // inputRate
						},
					},
				.attributes =
					{
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
							offsetof(Vertex, color),	// offset
						},
						{
							2, // uv
							vertexbinding,
							VK_FORMAT_R32G32_SFLOAT, // format
							offsetof(Vertex, uv),	 // offset
						},
						{
							3, // normal
							vertexbinding,
							VK_FORMAT_R32G32B32_SFLOAT, // format
							offsetof(Vertex, normal),	// offset
						},
					},
			};
		}
	};

	GKAPI struct Primitive
	{
		uint32_t id = 0;
		uint32_t firstIndex = 0;
		uint32_t indexCount = 0;
		uint32_t vertexOffset = 0;
		uint32_t vertexCount = 0;
		uint32_t materialId = 0;
	};

	GKAPI class Mesh
	{
	public:
		// friend class Scenegraph;
		Mesh(const Core::DeviceRef &device,
			uint32_t id,
			const Core::Buffer &vertexBuffer,
			const Core::Buffer &indexBuffer,
			std::vector<Primitive> primitives,
			std::unordered_map<uint32_t, MaterialPtr> materials);

		virtual ~Mesh();

		// void Draw(const Core::CommandBufferRef &commandBuffer, uint32_t primitiveIndex) const;

		[[nodiscard]] inline const std::vector<Primitive> &GetPrimitives() const noexcept
		{
			return m_primitives;
		}

		[[nodiscard]] inline const Core::Buffer GetVertexBuffer() const noexcept
		{
			return m_vertexBuffer;
		}

		[[nodiscard]] inline const Core::Buffer GetIndexBuffer() const noexcept
		{
			return m_indexBuffer;
		}

		[[nodiscard]] inline const MaterialPtr GetMaterial(const uint32_t materalId) const noexcept
		{
			return m_materials.find(materalId) != m_materials.end() ? m_materials.at(materalId) : nullptr;
		}

		static MeshPtr Create(const Core::DeviceRef &device,
			const std::vector<Vertex> &vertices,
			const std::vector<uint32_t> &indices,
			std::vector<Primitive> primitives,
			std::unordered_map<uint32_t, MaterialPtr> materials = {});

	private:
		const Core::DeviceRef m_device;
		uint32_t m_id = 0;
		Core::Buffer m_vertexBuffer{};
		Core::Buffer m_indexBuffer{};

		std::vector<Primitive> m_primitives = {};
		std::unordered_map<uint32_t, MaterialPtr> m_materials = {};
	};

	GKAPI class FullScreenQuad
	{
	public:
		FullScreenQuad(const Core::DeviceRef &device, Core::Buffer &vertexBuffer);
		~FullScreenQuad();

		void Draw(const Core::CommandBufferRef &commandBuffer) const;

		static std::shared_ptr<FullScreenQuad> Create(const Core::DeviceRef &device);

		struct Vertex
		{
			glm::vec3 position{};

			static inline Core::VertexDescription GetVertexDescription(const uint32_t inputBinding = 0,
				const uint32_t vertexbinding = 0)
			{
				return {
					.bindings =
						{
							{
								inputBinding,
								sizeof(Vertex),				// stride
								VK_VERTEX_INPUT_RATE_VERTEX // inputRate
							},
						},
					.attributes =
						{
							{
								0, // position
								vertexbinding,
								VK_FORMAT_R32G32_SFLOAT,	// format
								offsetof(Vertex, position), // offset
							},
						},
				};
			}
		};

	private:
		const Core::DeviceRef m_device;
		Core::Buffer m_vertexBuffer{};
	};

	using FullScreenQuadPtr = std::shared_ptr<FullScreenQuad>;

} // namespace Grafkit

#endif // _GRAFKIT_MESH_H_
