#include <grafkit/core/pipeline.h>

using namespace Grafkit::Core;

Pipeline::Pipeline(Device const& m_device,
	std::tuple<VkPipeline, VkPipelineLayout, VkDescriptorSetLayout> pipeline,
	VkPipelineBindPoint pipelineBindPoint)
	: m_device(m_device)
	, pipelineBindPoint(pipelineBindPoint)
	, pipeline(std::get<0>(pipeline))
	, pipelineLayout(std::get<1>(pipeline))
	, descriptorSetLayout(std::get<2>(pipeline))
{
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(m_device.GetVkDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(m_device.GetVkDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device.GetVkDevice(), descriptorSetLayout, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline); }

// -----------------------------------------------------------------------------

GraphicsPipelineBuilder::GraphicsPipelineBuilder(Device const& m_device, VkRenderPass renderPass)
	: m_device(m_device)
	, m_renderPass(renderPass)
{
	// Setup defaults
	SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	SetRasterizer(VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_CLOCKWISE); // TOOD: Clarify default polygon winding order
	SetMultisampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);

	VkPipelineColorBlendAttachmentState m_colorBlendAttachment {};
	m_colorBlendAttachment.colorWriteMask
		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_colorBlendAttachment.blendEnable = VK_FALSE;
	SetColorBlending(m_colorBlendAttachment);

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

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputDescription(const VertexDescription desc)
{
	m_vertexBindingDescriptions = std::get<0>(desc);
	m_vertexInputAttrDescriptions = std::get<1>(desc);
	return *this;
}

// TOOD: Move this into a separate builder
GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddLayoutBinding(
	std::uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags shaderStageFlags)
{
	m_descriptorSetLayoutBindings.emplace_back(
		Initializers::DescriptorSetLayoutBinding(descriptorType, shaderStageFlags, binding));
	return *this;
}

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
	// --- Pipeline layout
	VkDescriptorSetLayout descriptorSetLayout { VK_NULL_HANDLE };
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo
		= Initializers::DescriptorSetLayoutCreateInfo(m_descriptorSetLayoutBindings);

	if (vkCreateDescriptorSetLayout(m_device.GetVkDevice(), &descriptorLayoutInfo, nullptr, &descriptorSetLayout)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initializers::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);

	if (vkCreatePipelineLayout(m_device.GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// --- Pipeline
	VkPipelineViewportStateCreateInfo m_viewportState {};
	m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportState.viewportCount = 1;
	m_viewportState.scissorCount = 1;

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

	VkPipelineDynamicStateCreateInfo dynamicState {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
	dynamicState.pDynamicStates = m_dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = m_vertexBindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = m_vertexBindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = m_vertexInputAttrDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = m_vertexInputAttrDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
	pipelineInfo.pStages = m_shaderStages.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &m_inputAssembly;
	pipelineInfo.pViewportState = &m_viewportState;
	pipelineInfo.pRasterizationState = &m_rasterizer;
	pipelineInfo.pMultisampleState = &m_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(m_device.GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	PipelinePtr pipeline = std::make_shared<Pipeline>(m_device,
		std::make_tuple(graphicsPipeline, pipelineLayout, descriptorSetLayout),
		VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto shaderStage : m_shaderStages) {
		vkDestroyShaderModule(m_device.GetVkDevice(), shaderStage.module, nullptr);
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
	if (vkCreateShaderModule(m_device.GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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
	if (vkCreateShaderModule(m_device.GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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
