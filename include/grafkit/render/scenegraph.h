#ifndef GRAFKIT_SCENEGRAPH_H
#define GRAFKIT_SCENEGRAPH_H

#include <glm/gtc/quaternion.hpp>
#include <grafkit/common.h>

namespace Grafkit {

	struct Node;
	using NodePtr = std::shared_ptr<Node>;
	using NodeRef = std::weak_ptr<Node>;

	struct Node {
		uint32_t id = 0;
		NodePtr parent;
		std::vector<NodePtr> children;

		std::weak_ptr<Mesh> mesh;

		glm::mat4 matrix = glm::mat4(1.0f);
		glm::mat4 modelView = glm::mat4(1.0f);

		glm::vec3 translation = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		int32_t skin = -1;
		bool isHidden = false;

		void UpdateLocalMatrix();
	};

	// TODO: IResource + GRAFKIT_RESOURCE_KIND("Scenegraph")
	class Scenegraph {
	public:
		Scenegraph() = default;
		virtual ~Scenegraph();

		NodePtr CreateNode(const NodePtr& parent = nullptr, const MeshPtr& mesh = nullptr);

		void AddMaterial(const MaterialPtr& material);
		void AddTexture(const TexturePtr& texture);
		void AddMesh(const MeshPtr& mesh);
		void AddAnimation(const Animation::AnimationPtr& animation);

		void Update(const Grafkit::TimeInfo& deltaTime);
		void Draw(const Grafkit::Core::CommandBufferRef& commandBuffer);

	private:
		NodePtr m_root;
		std::vector<MaterialPtr> m_materials;
		std::vector<TexturePtr> m_textures;
		std::vector<MeshPtr> m_meshes;
		std::vector<Animation::AnimationPtr> m_animations;

		// + Camera node?

		// + Camera view
	};

} // namespace Grafkit

#endif // SCENEGRAPH_H
