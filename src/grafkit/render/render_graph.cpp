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
	std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings,
	std::vector<VkPushConstantRange> pushConstants)
	: m_device(device)
	, m_pipeline(std::move(pipeline))
	, m_bindings(std::move(bindings))
	, m_pushConstants(std::move(pushConstants))
{
}

RenderStage::~RenderStage() = default;

void RenderStage::GetDescriporSetLayoutBindings(std::vector<uint32_t> &slots) const noexcept
{
	slots.reserve(m_bindings.size());
	for (const auto &[set, bindings] : m_bindings)
	{
		slots.push_back(set);
	}
}

Core::DescriptorSetPtr RenderStage::CrateDescriptorSet(const uint32_t set) const
{
	if (m_bindings.find(set) == m_bindings.end())
	{
		throw std::runtime_error("Descriptor set " + std::to_string(set) + " is not defined!");
	}
	return Core::DescriptorSet::Create(m_device, set, m_bindings.at(set));
}

// ---

void RenderStage::Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
{
	// Begin render pass
	VkRenderPassBeginInfo renderPassInfo = m_renderTarget->CreateRenderPassBeginInfo(frameIndex);

	// Clear color
	if (m_isClear)
	{
		renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
		renderPassInfo.pClearValues = m_clearValues.data();
	}

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

// MARK: RenderGraph
void RenderGraph::BuildFromStages(const std::vector<RenderStagePtr> &stages)
{
	m_stages = stages;
}

void RenderGraph::Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
{
	for (const auto &stage : m_stages)
	{
		stage->Record(commandBuffer, frameIndex);
	}
}

// MARK: RenderStageBuilder
RenderStageBuilder::RenderStageBuilder(const Core::DeviceRef &device)
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
	std::vector<VkDescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(bindings.size());
	std::transform(bindings.begin(),
		bindings.end(),
		std::back_inserter(vkBindings),
		[](const Core::DescriptorBinding &binding)
		{
			return Core::Initializers::DescriptorSetLayoutBinding(binding.descriptorType,
				binding.stageFlags,
				binding.binding);
		});
	m_bindings.emplace(set, std::move(vkBindings));

	m_pipelineBuilder->AddDescriptorSet(set, bindings);
	return *this;
}

RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBindings(
	const Core::DescriptorSetLayoutBindings &descriptors)
{
	for (const auto &descriptor : descriptors)
	{
		m_bindings.emplace(descriptor.set, std::vector<VkDescriptorSetLayoutBinding>());
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
		std::move(m_bindings),
		std::move(m_pushConstants));
}
