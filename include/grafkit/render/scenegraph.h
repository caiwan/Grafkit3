#ifndef GRAFKIT_SCENEGRAPH_H
#define GRAFKIT_SCENEGRAPH_H

#include <tuple>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <grafkit/common.h>

namespace Grafkit

{
	struct Node;
	using NodePtr = std::shared_ptr<Node>;
	using NodeRef = std::weak_ptr<Node>;

	struct Node
	{
		uint32_t id = 0;
		NodePtr parent;
		std::vector<NodePtr> children;

		glm::mat4 matrix = glm::mat4(1.0f);
		glm::mat4 modelView = glm::mat4(1.0f);

		glm::vec3 translation = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		int32_t skin = -1;
		bool isHidden = false;

		void UpdateLocalMatrix();
	};

	// MARK: Scenegraph
	class Scenegraph
	{
	public:
		Scenegraph() = default;
		virtual ~Scenegraph() = default;

		NodePtr CreateNode(const NodePtr &parent = nullptr);
		NodePtr CreateNode(const MeshPtr &mesh, const NodePtr &parent = nullptr); // TODO: Add bone

		void AddDescriptorSet(const uint32_t set, const Core::DescriptorSetPtr &descriptorSet);

		void Update(const Grafkit::TimeInfo &deltaTime);
		void
		Draw(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex, const uint32_t stageIndex) const;

	private:
		struct DrawCommand
		{
			std::vector<Core::DescriptorSetPtr> descriptorSets{};
			VkBuffer vertexBuffer = VK_NULL_HANDLE;
			VkBuffer indexBuffer = VK_NULL_HANDLE;
			uint32_t firstIndex = 0;
			uint32_t indexCount = 0;
			uint32_t vertexOffset = 0;
			uint32_t instanceCount = 0;
			NodePtr node = nullptr;
		};

		void UpdateRenderGraph();

		NodePtr m_root;
		std::vector<NodePtr> m_nodes;
		std::vector<std::pair<MeshPtr, NodePtr>> m_meshesToNodes;
		// + bones to nodes
		std::vector<std::pair<RenderStagePtr, std::vector<DrawCommand>>> m_commandList;

		std::map<uint32_t, Core::DescriptorSetPtr> m_descriptorSets;

		bool m_isDirty = true;
	};

} // namespace Grafkit

#endif // SCENEGRAPH_H
