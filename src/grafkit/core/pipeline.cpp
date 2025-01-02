#include "stdafx.h"

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor.h"
#include "grafkit/core/device.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/core/render_target.h"

using namespace Grafkit::Core;

Pipeline::Pipeline(DeviceRef const &m_device,
	std::tuple<VkPipeline, VkPipelineLayout, std::vector<VkDescriptorSetLayout>> pipeline,
	VkPipelineBindPoint pipelineBindPoint)
	: m_device(m_device)
	, m_pipelineBindPoint(pipelineBindPoint)
	, m_pipeline(std::get<0>(pipeline))
	, m_pipelineLayout(std::get<1>(pipeline))
	, m_descriptorSetLayouts(std::get<2>(pipeline))
{
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(**m_device, m_pipeline, nullptr);
	vkDestroyPipelineLayout(**m_device, m_pipelineLayout, nullptr);
	for (auto &descriptorSetLayout : m_descriptorSetLayouts)
	{
		if (descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(**m_device, descriptorSetLayout, nullptr);
		}
	}
}

void Pipeline::Bind(const CommandBufferRef &commandBuffer)
{
	vkCmdBindPipeline(**commandBuffer, m_pipelineBindPoint, m_pipeline);
}

// -----------------------------------------------------------------------------

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const DeviceRef &device)
	: m_device(device)
{
	// TOOD: Setup defaults [from struct]
	SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	SetRasterizer(VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_CLOCKWISE); // TOOD: Clarify default polygon winding order
	SetMultisampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);

	VkPipelineColorBlendAttachmentState mColorBlendAttachment{};
	mColorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	mColorBlendAttachment.blendEnable = VK_FALSE;
	SetColorBlending(mColorBlendAttachment);

	SetDynamicState({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
}

GraphicsPipelineBuilder &Grafkit::Core::GraphicsPipelineBuilder::SetRenderTarget(const Core::RenderTargetPtr &target)
{
	m_renderTarget = target;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::AddVertexShader(const uint8_t *code, size_t len)
{
	VkShaderModule vertexShaderModule = CreateShaderModule(code, len);
	AddShaderStage(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::AddVertexShader(const std::string &code)
{
	VkShaderModule vertexShaderModule = CreateShaderModule(code);
	AddShaderStage(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::AddFragmentShader(const uint8_t *code, size_t len)
{
	VkShaderModule fragmentShaderModule = CreateShaderModule(code, len);
	AddShaderStage(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::AddFragmentShader(const std::string &code)
{
	VkShaderModule fragmentShaderModule = CreateShaderModule(code);
	AddShaderStage(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetInputAssembly(const VkPrimitiveTopology topology,
	const VkBool32 primitiveRestartEnable)
{
	m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssembly.topology = topology;
	m_inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetVertexInputDescription(const VertexDescription &desc)
{
	m_vertexBindingDescriptions = desc.bindings;
	m_vertexInputAttrDescriptions = desc.attributes;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::AddDescriptorSet(const uint32_t set,
	const std::vector<DescriptorBinding> &bindings)
{
	std::vector<VkDescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(bindings.size());
	std::transform(bindings.begin(),
		bindings.end(),
		std::back_inserter(vkBindings),
		[](const DescriptorBinding &binding) {
			return Initializers::DescriptorSetLayoutBinding(binding.descriptorType,
				binding.stageFlags,
				binding.binding);
		});
	m_descriptorSetBindings.emplace(set, std::move(vkBindings));

	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::AddDescriptorSets(
	const DescriptorSetLayoutBindings &descriptorSetLayoutBindings)
{
	for (const auto &descriptor : descriptorSetLayoutBindings)
	{
		AddDescriptorSet(descriptor.set, descriptor.bindings);
	}

	return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::AddPushConstants(const VkShaderStageFlags stage, const uint32_t size, const uint32_t offset)
{
	m_pushConstants.push_back(Initializers::PushConstantRange(stage, size, offset));
	return *this;
};

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetRasterizer(const VkPolygonMode polygonMode,
	const VkCullModeFlags cullMode,
	const VkFrontFace frontFace)
{
	m_rasterizer = Initializers::PipelineRasterizationStateCreateInfo(polygonMode, cullMode, frontFace);
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetMultisampling(VkSampleCountFlagBits rasterizationSamples,
	VkBool32 sampleShadingEnable)
{
	m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampling.sampleShadingEnable = sampleShadingEnable;
	m_multisampling.rasterizationSamples = rasterizationSamples;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetColorBlending(
	const VkPipelineColorBlendAttachmentState &m_colorBlendAttachment)
{
	this->m_colorBlendAttachment = m_colorBlendAttachment;
	return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::SetDynamicState(const std::vector<VkDynamicState> &m_dynamicStates)
{
	this->m_dynamicStates = m_dynamicStates;
	return *this;
}

PipelinePtr GraphicsPipelineBuilder::Build()
{
	// --- Create Descriptor Set Layouts from individual binding sets

	// Check if all descriptor sets are allocated and continuous
	if (m_descriptorSetBindings.empty())
	{
		throw std::runtime_error("No descriptor set is allocated!");
	}

	const uint32_t maxDescriptorSlotId = //
		(std::max_element(m_descriptorSetBindings.begin(),
			m_descriptorSetBindings.end(),
			[](const auto &lhs, const auto &rhs) {
				return lhs.first < rhs.first;
			})->first) +
		1;

	if (maxDescriptorSlotId != m_descriptorSetBindings.size())
	{
		throw std::runtime_error("Descriptor set slot id is not continuous!");
	}

	Log::Instance().Trace("Total descriptor set layout binding count: %d", maxDescriptorSlotId);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(maxDescriptorSlotId, VK_NULL_HANDLE);

	for (const auto &[set, bindings] : m_descriptorSetBindings)
	{
		assert(set < m_descriptorSetBindings.size());
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayoutCreateInfo layoutInfo = Initializers::DescriptorSetLayoutCreateInfo(bindings);

		Log::Instance().Trace("Creating descriptor set layout for slot %d With binding size %d", set, bindings.size());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(**m_device, &layoutInfo, nullptr, &descriptorSetLayout));

		descriptorSetLayouts[set] = descriptorSetLayout;
	}

	// This should not happen if the descriptor set layout is created properly at the first place
	if (std::find(descriptorSetLayouts.begin(), descriptorSetLayouts.end(), VK_NULL_HANDLE) !=
		descriptorSetLayouts.end())
	{
		throw std::runtime_error("Descriptor set layout is not created properly!");
	}

	// --- Create Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initializers::PipelineLayoutCreateInfo(descriptorSetLayouts.data(),
		static_cast<uint32_t>(descriptorSetLayouts.size()));

	pipelineLayoutInfo.flags = VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;

	if (!m_pushConstants.empty())
	{
		pipelineLayoutInfo.pushConstantRangeCount = m_pushConstants.size();
		pipelineLayoutInfo.pPushConstantRanges = m_pushConstants.data();
	}

	VkPipelineLayout pipelineLayout;

	if (vkCreatePipelineLayout(**m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// --- Pipeline
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &m_colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depthStencil =
		Initializers::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);

	VkPipelineDynamicStateCreateInfo dynamicState = Initializers::PipelineDynamicStateCreateInfo(m_dynamicStates);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo =
		Initializers::PipelineVertexInputStateCreateInfo(m_vertexBindingDescriptions, m_vertexInputAttrDescriptions);

	VkGraphicsPipelineCreateInfo pipelineInfo = Initializers::PipelineCreateInfo();
	pipelineInfo.renderPass = m_renderTarget->GetRenderPass();
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
	pipelineInfo.pStages = m_shaderStages.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &m_inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &m_rasterizer;
	pipelineInfo.pMultisampleState = &m_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pDepthStencilState = &depthStencil;

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(**m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) !=
		VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	PipelinePtr pipeline = std::make_shared<Pipeline>(m_device,
		std::make_tuple(graphicsPipeline, pipelineLayout, std::move(descriptorSetLayouts)),
		VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto shaderStage : m_shaderStages)
	{
		vkDestroyShaderModule(**m_device, shaderStage.module, nullptr);
	}

	return pipeline;
}

VkShaderModule GraphicsPipelineBuilder::CreateShaderModule(const uint8_t *code, size_t len) const
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = len;
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(**m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkShaderModule GraphicsPipelineBuilder::CreateShaderModule(const std::string &code) const
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(**m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void GraphicsPipelineBuilder::AddShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = stage;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = "main";
	m_shaderStages.push_back(shaderStageInfo);
}
