#include "stdafx.h"
#include <stack>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/render/animation.h"
#include "grafkit/render/material.h"
#include "grafkit/render/mesh.h"
#include "grafkit/render/scenegraph.h"

void Grafkit::Node::UpdateLocalMatrix()
{
	matrix =
		glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
}

Grafkit::NodePtr Grafkit::Scenegraph::CreateNode(const NodePtr &parent, const MeshPtr &mesh)
{
	NodePtr node = std::make_shared<Node>();
	if (parent)
	{
		node->parent = parent;
		parent->children.push_back(node);
	}
	else
	{
		if (m_root)
		{
			throw std::runtime_error("Root node already exists");
		}
		m_root = node;
	}

	node->mesh = mesh;

	return node;
}

Grafkit::Scenegraph::~Scenegraph()
{
	m_animations.clear();
	m_materials.clear();
	m_meshes.clear();
	m_textures.clear();
}

void Grafkit::Scenegraph::AddMaterial(const MaterialPtr &material)
{
	m_materials.push_back(material);
}
void Grafkit::Scenegraph::AddTexture(const TexturePtr &texture)
{
	m_textures.push_back(texture);
}
void Grafkit::Scenegraph::AddMesh(const MeshPtr &mesh)
{
	m_meshes.push_back(mesh);
}
void Grafkit::Scenegraph::AddAnimation(const Animation::AnimationPtr &animation)
{
	m_animations.push_back(animation);
}

void Grafkit::Scenegraph::Update(const Grafkit::TimeInfo &timeInfo)
{
	std::stack<NodePtr> stack;
	stack.push(m_root);

	std::stack<glm::mat4> matrixStack;
	matrixStack.emplace(1.0f);

	while (!stack.empty())
	{
		NodePtr currentNode = stack.top();
		stack.pop();

		// Update current node
		currentNode->UpdateLocalMatrix();
		currentNode->modelView = matrixStack.top() * currentNode->matrix;

		matrixStack.pop();

		// Process children nodes
		for (const auto &child : currentNode->children)
		{
			matrixStack.push(currentNode->modelView);
			stack.push(child);
		}
	}
}

void Grafkit::Scenegraph::Draw(const Grafkit::Core::CommandBufferRef &commandBuffer)
{
	std::stack<NodePtr> stack;
	stack.push(m_root);

	std::stack<glm::mat4> matrixStack;
	matrixStack.push(m_root->matrix);

	while (!stack.empty())
	{
		NodePtr currentNode = stack.top();
		stack.pop();

		Core::PipelinePtr pipeline = nullptr;

		const MeshPtr mesh = currentNode->mesh.lock();
		if (!currentNode->isHidden && mesh != nullptr)
		{
			for (const auto &primitive : mesh->m_primitives)
			{

				//		// Bind material
				if (primitive.material != nullptr)
				{
					if (pipeline != primitive.material->pipeline)
					{
						pipeline = primitive.material->pipeline;
						pipeline->Bind(**commandBuffer);
					}

					for (const auto &descriptorSet : primitive.material->descriptorSets)
					{
						descriptorSet->Bind(commandBuffer,
							primitive.material->pipeline->GetPipelineLayout(),
							commandBuffer->GetFrameIndex());
					}
				}

				if (pipeline != nullptr)
				{
					vkCmdPushConstants(**commandBuffer,
						pipeline->GetPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT,
						0,
						sizeof(ModelView),
						&currentNode->modelView);

					mesh->Bind(commandBuffer, primitive.vertexOffset);
					primitive.Draw(commandBuffer);
				}
			}
		}

		// Process children nodes
		for (const auto &child : currentNode->children)
		{
			stack.push(child);
		}
	}
}
