#include "stdafx.h"

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/core/render_target.h"
#include "grafkit/render/render_graph.h"

constexpr bool ORDER_STAGE_DEPENDENCIES = false;
constexpr bool ORDER_STAGE_CLEAR_FIRST = false;

namespace Grafkit
{
	// MARK: RenderStage
	RenderStage::RenderStage(const Core::DeviceRef &device,
		Core::PipelinePtr pipeline,
		Core::RenderTargetPtr renderTarget,
		StageDescriptorMap descriptorSetBindings,
		std::vector<VkPushConstantRange> pushConstants,
		Core::RenderTargetPtr renderSource,
		std::vector<VkClearValue> clearValues)
		: m_device(device)
		, m_pipeline(std::move(pipeline))
		, m_descriptorMap(std::move(descriptorSetBindings))
		, m_pushConstants(std::move(pushConstants))
		, m_renderTarget(std::move(renderTarget))
		, m_renderSource(std::move(renderSource))
		, m_clearValues(std::move(clearValues))
	{
	}

	RenderStage::~RenderStage() = default;

	void RenderStage::Record(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)
	{
		// Begin render pass
		assert(m_renderTarget != nullptr);
		VkRenderPassBeginInfo renderPassInfo = m_renderTarget->CreateRenderPassBeginInfo(frameIndex);

		// Clear color
		renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
		renderPassInfo.pClearValues = m_clearValues.data();

		vkCmdBeginRenderPass(**commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_renderTarget->SetupViewport(commandBuffer);

		m_pipeline->Bind(commandBuffer);

		// bind inputs here
		// if (m_renderSource)
		// {
		// 	m_renderSource->Bind(commandBuffer);
		// }

		if (m_onRecordCallback)
		{
			m_onRecordCallback(commandBuffer, frameIndex);
		}

		vkCmdEndRenderPass(**commandBuffer);
	}

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

	Core::DescriptorSetLayoutBindingMap Grafkit::RenderStage::GetDescriptorSetLayoutBindings(
		const StageDescriptorType type) const noexcept
	{
		assert(m_pipeline);

		const auto it = m_descriptorMap.find(type);
		assert(it != m_descriptorMap.end());

		// TOOD:L It does not neccesarily need to be in vector-form
		if (it == m_descriptorMap.end())
		{
			return {};
		}

		return it->second;
	}

	Core::DescriptorSetPtr Grafkit::RenderStage::CreateDescriptorSet(const uint32_t set) const
	{
		assert(m_pipeline);

		for (const auto &descriptorSet : m_descriptorMap)
		{
			auto descriptorSetIt = descriptorSet.second.find(set);
			if (descriptorSetIt != descriptorSet.second.end())
			{
				return Core::DescriptorSet::Create(m_device,
					m_pipeline->GetDescriptorSetLayout(set),
					set,
					descriptorSetIt->second);
			}
		}

		assert(false && "Descriptor set not found in the pipeline descriptor set layout bindings.");
		return nullptr;
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

	Core::RenderTargetPtr Grafkit::RenderStage::GetRendderTarget() const noexcept
	{
		return m_renderTarget;
	}

	Core::RenderTargetPtr Grafkit::RenderStage::GetRenderSource() const noexcept
	{
		return m_renderSource;
	}

	// ---
	// MARK: RenderGraph
	void RenderGraph::BuildFromStages(std::vector<RenderStagePtr> inStages)
	{
		// No dependency check and order
		m_stages.clear();
		std::ranges::copy(inStages, std::back_inserter(m_stages));
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

	RenderStageBuilder &RenderStageBuilder::SetRenderSource(const Core::RenderTargetPtr &target)
	{
		m_renderSource = target;
		return *this;
	}

	RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBinding(const uint32_t set,
		const std::vector<Core::DescriptorBinding> &bindings)
	{
		AddDescriptorSetLayoutBinding(StageDescriptorType::None, set, bindings);
		return *this;
	}

	RenderStageBuilder &RenderStageBuilder::AddDescriptorSetLayoutBindings(
		const Core::DescriptorSetLayoutBindings &descriptors)
	{
		for (const auto &descriptor : descriptors)
		{
			AddDescriptorSetLayoutBinding(StageDescriptorType::None, descriptor.set, descriptor.bindings);
		}

		return *this;
	}

	RenderStageBuilder &RenderStageBuilder::AddMaterialDescriptorBinding(const uint32_t set,
		const std::vector<Core::DescriptorBinding> &bindings)
	{
		AddDescriptorSetLayoutBinding(StageDescriptorType::Material, set, bindings);
		return *this;
	}

	RenderStageBuilder &RenderStageBuilder::AddMaterialDescriptorBindings(
		const Core::DescriptorSetLayoutBindings &descriptors)
	{
		for (const auto &descriptor : descriptors)
		{
			AddDescriptorSetLayoutBinding(StageDescriptorType::Material, descriptor.set, descriptor.bindings);
		}
		return *this;
	}

	RenderStageBuilder &RenderStageBuilder::AddRenderStageDescriptorBinding(const uint32_t set,
		const std::vector<Core::DescriptorBinding> &bindings)
	{
		AddDescriptorSetLayoutBinding(StageDescriptorType::RenderStage, set, bindings);
		return *this;
	}

	RenderStageBuilder &RenderStageBuilder::AddRenderStageDescriptorBindings(
		const Core::DescriptorSetLayoutBindings &descriptors)
	{
		for (const auto &descriptor : descriptors)
		{
			AddDescriptorSetLayoutBinding(StageDescriptorType::RenderStage, descriptor.set, descriptor.bindings);
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

	void RenderStageBuilder::AddDescriptorSetLayoutBinding(const StageDescriptorType type,
		const uint32_t set,
		const Core::DescriptorBindings &bindings)
	{
		m_pipelineBuilder->AddDescriptorSet(set, bindings);

		auto it = m_descriptorMap.find(type);
		if (it == m_descriptorMap.end())
		{
			Core::DescriptorSetLayoutBindingMap bindingsMap{{set, bindings}};
			m_descriptorMap.emplace(type, bindingsMap);
		}
		else
		{
			it->second.emplace(set, bindings);
		}
	}

	RenderStagePtr RenderStageBuilder::Build() const
	{
		Core::PipelinePtr pipeline = m_pipelineBuilder->Build();
		// Inpuit check is done by the pipeline builder at this point
		std::vector<VkClearValue> clearValues = {};
		clearValues.reserve(m_renderTarget->GetAttachmentCount());
		for (size_t i = 0; i < m_renderTarget->GetAttachmentCount(); ++i)
		{
			if (m_renderTarget->GetAttachemntIsDepthStencil(i))
			{
				VkClearValue clearValue = {};
				clearValue.depthStencil = {1.0f, 0};
				clearValues.emplace_back(clearValue);
			}
			else
			{
				VkClearValue clearValue = {};
				clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};
				clearValues.emplace_back(clearValue);
			}
		}

		return std::make_shared<RenderStage>(m_device,
			pipeline,
			m_renderTarget,
			m_descriptorMap,
			m_pushConstants,
			m_renderSource,
			clearValues);
	}

} // namespace Grafkit
