#include "stdafx.h"

#include "grafkit/core/descriptor.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/core/render_target.h"
#include "grafkit/render/render_graph.h"

using namespace Grafkit;

// MARK: RenderStage
Grafkit::RenderStage::RenderStage(const Core::DeviceRef &device)
	: m_device(device)
{
}

Grafkit::RenderStage::~RenderStage() = default;

void Grafkit::RenderStage::Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
{
	// Clear color
	VkRenderPassBeginInfo renderPassInfo = Core::Initializers::RenderPassBeginInfo(m_renderTarget->GetRenderPass(),
		m_renderTarget->GetFrameBuffer(frameIndex),
		m_renderTarget->GetExtent());

	if (m_isClear)
	{
		renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
		renderPassInfo.pClearValues = m_clearValues.data();
	}

	vkCmdBeginRenderPass(**commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetScissor(**commandBuffer, 0, 1, &m_renderTarget->GetScissor());
	vkCmdSetViewport(**commandBuffer, 0, 1, &m_renderTarget->GetViewport());

	// ...

	vkCmdEndRenderPass(**commandBuffer);
}

void Grafkit::RenderStage::SetClearFlag(bool flag)
{
	m_isClear = flag;
}

void Grafkit::RenderStage::SetClearColor(uint32_t slot, const VkClearColorValue &color)
{
	assert(m_clearValues.size() > slot);
	m_clearValues[slot].color = color;
}

void Grafkit::RenderStage::SetClearDepth(uint32_t slot, const VkClearDepthStencilValue &depthStencil)
{
	assert(m_clearValues.size() > slot);
	m_clearValues[slot].depthStencil = depthStencil;
}

Grafkit::Core::DescriptorSetPtr Grafkit::RenderStage::CrateDescriptorSet(const uint32_t set)
{
	assert(m_bindings.find(set) != m_bindings.end());
	return Core::DescriptorSet::Create(m_device, set, m_bindings[set]);
}

// MARK: RenderGraph

void Grafkit::RenderGraph::BuildFromStages(const std::vector<RenderStagePtr> &stages)
{
	m_stages = stages;
}

void Grafkit::RenderGraph::Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
{
	for (const auto &stage : m_stages)
	{
		stage->Record(commandBuffer, frameIndex);
	}
}

// MARK: RenderStageBuilder
Grafkit::RenderStageBuilder::RenderStageBuilder(const Core::DeviceRef &device)
	: m_device(device)
	, m_pipelineBuilder(std::make_unique<Core::GraphicsPipelineBuilder>(device))
{
}

RenderStageBuilder &RenderStageBuilder::SetRenderTarget(const Core::RenderTargetPtr &target)
{
	m_pipelineBuilder->SetRenderTarget(target);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBinding(const uint32_t set,
	const std::vector<Core::DescriptorBinding> &bindings)
{
	m_bindings[set] = {};
	for (const auto &binding : bindings)
	{
		m_bindings[set].push_back(Core::Initializers::DescriptorSetLayoutBinding(binding.descriptorType,
			binding.stageFlags,
			binding.binding));
	}
	m_pipelineBuilder->AddDescriptorSet(set, bindings);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBindings(
	const Core::DescriptorSetLayoutBindings &descriptors)
{
	for (const auto &descriptor : descriptors)
	{
		m_bindings[descriptor.set] = {};
		for (const auto &binding : descriptor.bindings)
		{
			AddDescriptorSetLayoutBinding(descriptor.set, descriptor.bindings);
		}
	}

	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddVertexDescription(const Core::VertexDescription &vertexDescription)
{
	m_vertexDescriptions.push_back(vertexDescription);
	m_pipelineBuilder->SetVertexInputDescription(vertexDescription);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddPushConstantRange(const VkPushConstantRange &range)
{
	m_pushConstants.push_back(range);
	m_pipelineBuilder->AddPushConstants(range.stageFlags, range.size, range.offset);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::SetVertexShader(const std::string &shader)
{
	m_pipelineBuilder->AddVertexShader(shader);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::SetVertexShader(const uint8_t *code, size_t len)
{
	m_pipelineBuilder->AddVertexShader(code, len);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::SetFragmentShader(const std::string &shader)
{
	m_pipelineBuilder->AddFragmentShader(shader);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::SetFragmentShader(const uint8_t *code, size_t len)
{
	m_pipelineBuilder->AddFragmentShader(code, len);
	return *this;
}

RenderStagePtr RenderStageBuilder::Build()
{
	auto pipeline = m_pipelineBuilder->Build();
	return std::make_shared<RenderStage>(m_device);
}
