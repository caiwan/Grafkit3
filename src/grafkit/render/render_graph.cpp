#include "stdafx.h"

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/core/render_target.h"
#include "grafkit/render/render_graph.h"

using namespace Grafkit;

// MARK: RenderStage
RenderStage::RenderStage(const Core::DeviceRef &device,
	Core::PipelinePtr pipeline,
	Core::RenderTargetPtr renderTarget,
	Core::DescriptorSetLayoutBindingMap descriptorSetBindings,
	std::vector<VkPushConstantRange> pushConstants)
	: m_device(device)
	, m_pipeline(std::move(pipeline))
	, m_descriptorSetBindings(std::move(descriptorSetBindings))
	, m_pushConstants(std::move(pushConstants))
	, m_renderTarget(std::move(renderTarget))
{
	m_clearValues.reserve(m_renderTarget->GetAttachmentCount());
	for (size_t i = 0; i < m_renderTarget->GetAttachmentCount(); ++i)
	{
		if (m_renderTarget->GetAttachemntIsDepthStencil(i))
		{
			VkClearValue clearValue = {};
			clearValue.depthStencil = {1.0f, 0};
			m_clearValues.emplace_back(clearValue);
		}
		else
		{
			VkClearValue clearValue = {};
			clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
			m_clearValues.emplace_back(clearValue);
		}
	}
}

RenderStage::~RenderStage() = default;

// ---

void RenderStage::Record(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
{
	// Begin render pass
	assert(m_renderTarget != nullptr);
	VkRenderPassBeginInfo renderPassInfo = m_renderTarget->CreateRenderPassBeginInfo(frameIndex);

	// Clear color
	// if (m_isClear)
	// {
	renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
	renderPassInfo.pClearValues = m_clearValues.data();
	// }

	vkCmdBeginRenderPass(**commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_renderTarget->SetupViewport(commandBuffer);

	m_pipeline->Bind(commandBuffer);

	if (m_onRecordCallback)
	{
		m_onRecordCallback(commandBuffer, frameIndex);
	}

	vkCmdEndRenderPass(**commandBuffer);
}

// ---

void RenderStage::SetClearFlag(bool flag)
{
	m_isClear = flag;
}

void RenderStage::SetClearColor(uint32_t slot, const VkClearColorValue &color)
{
	assert(m_clearValues.size() > slot);
	m_clearValues[slot].color = color;
}

void RenderStage::SetClearDepth(uint32_t slot, const VkClearDepthStencilValue &depthStencil)
{
	assert(m_clearValues.size() > slot);
	m_clearValues[slot].depthStencil = depthStencil;
}

void RenderStage::SetOnbRecordCallback(const OnRecordRenderStageCallbackFn &callback)
{
	m_onRecordCallback = callback;
}

Core::DescriptorSetPtr Grafkit::RenderStage::CreateDescriptorSet(const uint32_t set) const
{
	assert(m_pipeline);

	const auto descriptorSetIt = m_descriptorSetBindings.find(set);
	assert(descriptorSetIt != m_descriptorSetBindings.end());

	return Core::DescriptorSet::Create(m_device, m_pipeline->GetDescriptorSetLayout(set), set, descriptorSetIt->second);
}

VkPipelineLayout RenderStage::GetPipelineLayout() const noexcept
{
	assert(m_pipeline);
	return m_pipeline->GetPipelineLayout();
}

VkDescriptorSetLayout RenderStage::DescriptorSetLayout(uint32_t index) const noexcept
{
	assert(m_pipeline);
	return m_pipeline->GetDescriptorSetLayout(index);
}

// ---
// MARK: RenderGraph
void RenderGraph::BuildFromStages(std::vector<RenderStagePtr> stages)
{
	m_stages = std::move(stages);
	// TODO: Issue 138
}

void RenderGraph::Record(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
{
	for (const auto &stage : m_stages)
	{
		stage->Record(commandBuffer, frameIndex);
	}
}

// ---
// MARK: RenderStageBuilder
RenderStageBuilder::RenderStageBuilder(const Core::DeviceRef &device)
	: m_device(device)
	, m_pipelineBuilder(std::make_unique<Core::GraphicsPipelineBuilder>(device))
{
}

RenderStageBuilder &RenderStageBuilder::SetRenderTarget(const Core::RenderTargetPtr &target)
{
	m_pipelineBuilder->SetRenderTarget(target);
	m_renderTarget = target;
	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBinding(const uint32_t set,
	const std::vector<Core::DescriptorBinding> &bindings)
{
	m_pipelineBuilder->AddDescriptorSet(set, bindings);
	m_descriptorSetBindings.emplace(set, bindings);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBindings(
	const Core::DescriptorSetLayoutBindings &descriptors)
{
	for (const auto &descriptor : descriptors)
	{
		AddDescriptorSetLayoutBinding(descriptor.set, descriptor.bindings);
	}

	return *this;
}

RenderStageBuilder &RenderStageBuilder::SetVertexInputDescription(const Core::VertexDescription &vertexDescription)
{
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
	Core::PipelinePtr pipeline = m_pipelineBuilder->Build();
	// Inpuit check is done by the pipeline builder at this point

	return std::make_shared<RenderStage>(m_device,
		std::move(pipeline),
		std::move(m_renderTarget),
		std::move(m_descriptorSetBindings),
		std::move(m_pushConstants));
}
