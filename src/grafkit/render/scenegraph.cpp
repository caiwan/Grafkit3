#include "stdafx.h"

#include <unordered_set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/render/animation.h"
#include "grafkit/render/material.h"
#include "grafkit/render/mesh.h"
#include "grafkit/render/render_graph.h"
#include "grafkit/render/scenegraph.h"

using namespace Grafkit;

constexpr bool USE_BUFFER_BINDING_OPTIMALIZATION = false;

void Node::UpdateLocalMatrix()
{
	matrix =
		glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
}

NodePtr Scenegraph::CreateNode(const NodePtr &parent)
{
	NodePtr node = std::make_shared<Node>();
	if (parent != nullptr)
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

	node->id = m_nodes.size();
	m_nodes.push_back(node);

	m_isDirty = true;

	return node;
}

NodePtr Scenegraph::CreateNode(const MeshPtr &mesh, const NodePtr &parent)
{
	NodePtr node = CreateNode(parent);

	if (mesh != nullptr)
	{
		m_meshesToNodes.emplace_back(mesh, node); // TODO: Move?
	}

	return node;
}

void Grafkit::Scenegraph::AddDescriptorSet(const uint32_t set, const Core::DescriptorSetPtr &descriptorSet)
{
	m_descriptorSets.emplace(set, descriptorSet);
	m_isDirty = true;
}

void Scenegraph::Update(const TimeInfo &timeInfo)
{
	if (m_isDirty)
	{
		UpdateRenderGraph();
	}

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

void Scenegraph::Draw(const Core::CommandBufferRef &commandBuffer,
	const uint32_t frameIndex,
	const uint32_t stageIndex) const
{
	VkBuffer lastVertexBuffer = VK_NULL_HANDLE;
	VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
	const auto &renderStage = m_commandList[stageIndex].first;

	// Dind common descriptor sets
	for (const auto &descriptorSet : m_descriptorSets)
	{
		descriptorSet.second->Bind(commandBuffer, renderStage->GetPipelineLayout(), frameIndex);
	}

	// Execute draw commands
	for (const auto &command : m_commandList[stageIndex].second)
	{
		if (command.node->isHidden)
		{
			continue;
		}

		// TODO: Add instance support
		if constexpr (USE_BUFFER_BINDING_OPTIMALIZATION)
		{
			if (command.vertexBuffer != lastVertexBuffer)
			{
				std::array<VkDeviceSize, 1> offsets = {0};
				vkCmdBindVertexBuffers(**commandBuffer, 0, 1, &command.vertexBuffer, offsets.data());
				lastVertexBuffer = command.vertexBuffer;
			}

			if (command.indexBuffer != lastIndexBuffer)
			{
				vkCmdBindIndexBuffer(**commandBuffer, command.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				lastIndexBuffer = command.indexBuffer;
			}
		}
		else
		{
			std::array<VkDeviceSize, 1> offsets = {0};
			vkCmdBindVertexBuffers(**commandBuffer, 0, 1, &command.vertexBuffer, offsets.data());
			vkCmdBindIndexBuffer(**commandBuffer, command.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}

		// Bind descriptor sets
		// TOOO: This list should be passed to the render stage

		// THIS IS FAULTY !!!!!!!!

		for (const auto &descriptorSet : command.descriptorSets)
		{
			descriptorSet->Bind(commandBuffer, renderStage->GetPipelineLayout(), frameIndex);
		}

		// Push constants
		// TODO: Render stage should be able to bind push constants
		vkCmdPushConstants(**commandBuffer,
			m_commandList[stageIndex].first->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(glm::mat4),
			&command.node->modelView);

		vkCmdDrawIndexed(**commandBuffer,
			command.indexCount,
			command.instanceCount,
			command.firstIndex,
			command.vertexOffset,
			0);
	}
}

void Scenegraph::UpdateRenderGraph()
{
	/**
	 * TODO:
	 * - Update list in an optimal way that is does not need to be rebuild every change
	 * - Minimise memory allocations and copies as much as possible
	 * - Check for mesh ids respectively.
	 * - This is a naive implementation
	 * - Use mutex once if multithreading is implemented
	 */
	m_commandList.clear();

	for (const auto &meshToNode : m_meshesToNodes)
	{
		const MeshPtr &mesh = meshToNode.first;
		const Core::Buffer &vertexBuffer = mesh->GetVertexBuffer();
		const Core::Buffer &indexBuffer = mesh->GetIndexBuffer();

		for (const auto &primitive : mesh->GetPrimitives())
		{
			const MaterialPtr material = mesh->GetMaterial(primitive.materialId);

			if (material == nullptr)
			{
				continue;
			}

			const RenderStagePtr stage = material->stage;

			// Find the corresponding stage in the command list
			auto listIt = std::find_if(m_commandList.begin(),
				m_commandList.end(),
				[&stage](const std::pair<RenderStagePtr, std::vector<DrawCommand>> &item)
				{
					return item.first == stage; //
				});

			// Create new stage in command list if not found
			if (listIt == m_commandList.end())
			{
				const uint32_t stageIndex = static_cast<uint32_t>(m_commandList.size());

				stage->SetOnbRecordCallback(
					[this, stageIndex](const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
					{ Draw(commandBuffer, frameIndex, stageIndex); });

				m_commandList.emplace_back(stage, std::vector<DrawCommand>{});
				listIt = m_commandList.end() - 1;
			}

			std::vector<Core::DescriptorSetPtr> descriptorSets;
			descriptorSets.reserve(material->descriptorSets.size());

			for (const auto &[set, descriptorSet] : material->descriptorSets)
			{
				descriptorSets.push_back(descriptorSet);
			}

			// Add draw command to the stage
			listIt->second.push_back(DrawCommand{
				.descriptorSets = std::move(descriptorSets),
				.vertexBuffer = vertexBuffer.buffer,
				.indexBuffer = indexBuffer.buffer,
				.firstIndex = primitive.firstIndex,
				.indexCount = primitive.indexCount,
				.vertexOffset = primitive.vertexOffset,
				.instanceCount = 1,
				.node = meshToNode.second,
			});
		}
	}

	// Order by vertex and index buffer [pointers] to ensure the least amount of buffer switches
	if constexpr (USE_BUFFER_BINDING_OPTIMALIZATION)
	{
		for (auto &stage : m_commandList)
		{
			std::sort(stage.second.begin(),
				stage.second.end(),
				[](const DrawCommand &a, const DrawCommand &b) {
					return a.vertexBuffer < b.vertexBuffer ||
						   (a.vertexBuffer == b.vertexBuffer && a.indexBuffer < b.indexBuffer);
				});
		}
	}

	m_isDirty = false;
}
