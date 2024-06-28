#include "stdafx.h"

#include <grafkit/core/descriptor.h>
#include <grafkit/core/device.h>
#include <grafkit/core/framebuffer.h>
#include <grafkit/core/initializers.h>
#include <grafkit/core/pipeline.h>

using namespace Grafkit::Core;

Pipeline::Pipeline(DeviceRef const& m_device,
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
	for (auto& descriptorSetLayout : m_descriptorSetLayouts) {
		vkDestroyDescriptorSetLayout(**m_device, descriptorSetLayout, nullptr);
	}
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, m_pipelineBindPoint, m_pipeline);
}

// -----------------------------------------------------------------------------

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const DeviceRef& device, const FrameBufferRef& frameBuffer)
	: m_device(device)
	, m_frameBuffer(frameBuffer)
{
	// Setup defaults
	SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	SetRasterizer(VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_CLOCKWISE); // TOOD: Clarify default polygon winding order
	SetMultisampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);

	VkPipelineColorBlendAttachmentState mColorBlendAttachment {};
	mColorBlendAttachment.colorWriteMask
		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	mColorBlendAttachment.blendEnable = VK_FALSE;
	SetColorBlending(mColorBlendAttachment);

	SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexShader(const uint8_t* code, size_t len)
{
	VkShaderModule vertexShaderModule = CreateShaderModule(code, len);
	AddShaderStage(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexShader(const std::vector<char>& code)
{
	VkShaderModule vertexShaderModule = CreateShaderModule(code);
	AddShaderStage(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddFragmentShader(const uint8_t* code, size_t len)
{
	VkShaderModule fragmentShaderModule = CreateShaderModule(code, len);
	AddShaderStage(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddFragmentShader(const std::vector<char>& code)
{
	VkShaderModule fragmentShaderModule = CreateShaderModule(code);
	AddShaderStage(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetInputAssembly(
	const VkPrimitiveTopology topology, const VkBool32 primitiveRestartEnable)
{
	m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssembly.topology = topology;
	m_inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputDescription(const VertexDescription& desc)
{
	m_vertexBindingDescriptions = desc.bindings;
	m_vertexInputAttrDescriptions = desc.attributes;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDescriptorSets(const std::vector<SetDescriptor>& descriptorSets)
{
	for (const auto& descriptor : descriptorSets) {
		m_descriptorSets[descriptor.set] = {};
		for (const auto& binding : descriptor.bindings) {
			m_descriptorSets[descriptor.set].push_back(
				Initializers::DescriptorSetLayoutBinding(binding.descriptorType, binding.stageFlags, binding.binding));
		}
	}

	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddPushConstants(
	const VkShaderStageFlags stage, const uint32_t size, const uint32_t offset)
{
	m_pushConstants.push_back(Initializers::PushConstantRange(stage, size, offset));
	return *this;
};

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizer(
	const VkPolygonMode polygonMode, const VkCullModeFlags cullMode, const VkFrontFace frontFace)
{
	m_rasterizer = Initializers::PipelineRasterizationStateCreateInfo(polygonMode, cullMode, frontFace);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMultisampling(
	VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable)
{
	m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampling.sampleShadingEnable = sampleShadingEnable;
	m_multisampling.rasterizationSamples = rasterizationSamples;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlending(
	const VkPipelineColorBlendAttachmentState& m_colorBlendAttachment)
{
	this->m_colorBlendAttachment = m_colorBlendAttachment;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDynamicState(const std::vector<VkDynamicState>& m_dynamicStates)
{
	this->m_dynamicStates = m_dynamicStates;
	return *this;
}

PipelinePtr GraphicsPipelineBuilder::Build()
{
	// --- Create Descriptor Set Layouts from individual binding sets
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_descriptorSets.size());

	for (const auto& [set, bindings] : m_descriptorSets) {
		assert(set < m_descriptorSets.size());
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayoutCreateInfo layoutInfo = Initializers::DescriptorSetLayoutCreateInfo(bindings);
		if (vkCreateDescriptorSetLayout(**m_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		descriptorSetLayouts[set] = descriptorSetLayout;
	}

	// --- Create Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initializers::PipelineLayoutCreateInfo(
		descriptorSetLayouts.data(), static_cast<uint32_t>(descriptorSetLayouts.size()));

	if (!m_pushConstants.empty()) {
		pipelineLayoutInfo.pushConstantRangeCount = m_pushConstants.size();
		pipelineLayoutInfo.pPushConstantRanges = m_pushConstants.data();
	}

	VkPipelineLayout pipelineLayout;

	if (vkCreatePipelineLayout(**m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// --- Pipeline
	VkPipelineViewportStateCreateInfo mViewportState {};
	mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	mViewportState.viewportCount = 1;
	mViewportState.scissorCount = 1;

	VkPipelineColorBlendStateCreateInfo colorBlending {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &m_colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depthStencil
		= Initializers::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);

	VkPipelineDynamicStateCreateInfo dynamicState = Initializers::PipelineDynamicStateCreateInfo(m_dynamicStates);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo
		= Initializers::PipelineVertexInputStateCreateInfo(m_vertexBindingDescriptions, m_vertexInputAttrDescriptions);

	VkGraphicsPipelineCreateInfo pipelineInfo = Initializers::PipelineCreateInfo();
	pipelineInfo.renderPass = m_frameBuffer->GetVkRenderPass();
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
	pipelineInfo.pStages = m_shaderStages.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &m_inputAssembly;
	pipelineInfo.pViewportState = &mViewportState;
	pipelineInfo.pRasterizationState = &m_rasterizer;
	pipelineInfo.pMultisampleState = &m_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pDepthStencilState = &depthStencil;

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(**m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	PipelinePtr pipeline = std::make_shared<Pipeline>(m_device,
		std::make_tuple(graphicsPipeline, pipelineLayout, descriptorSetLayouts),
		VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto shaderStage : m_shaderStages) {
		vkDestroyShaderModule(**m_device, shaderStage.module, nullptr);
	}

	return pipeline;
}

VkShaderModule GraphicsPipelineBuilder::CreateShaderModule(const uint8_t* code, size_t len) const
{
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = len;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(**m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkShaderModule GraphicsPipelineBuilder::CreateShaderModule(const std::vector<char>& code) const
{
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(**m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void GraphicsPipelineBuilder::AddShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStageInfo {};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = stage;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = "main";
	m_shaderStages.push_back(shaderStageInfo);
}
