#include <grafkit/core/pipeline.h>

using namespace Grafkit::Core;

Pipeline::Pipeline(Device const& device, std::tuple<VkPipeline, VkPipelineLayout, VkDescriptorSetLayout> pipeline,
	VkPipelineBindPoint pipelineBindPoint)
	: device(device)
	, pipelineBindPoint(pipelineBindPoint)
	, pipeline(std::get<0>(pipeline))
	, pipelineLayout(std::get<1>(pipeline))
	, descriptorSetLayout(std::get<2>(pipeline))
{
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(device.GetVkDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device.GetVkDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device.GetVkDevice(), descriptorSetLayout, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline); }

// -----------------------------------------------------------------------------

GraphicsPipelineBuilder::GraphicsPipelineBuilder(Device const& device, VkRenderPass renderPass)
	: device(device)
	, renderPass(renderPass)
{
	// Setup defaults
	SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	SetRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_CLOCKWISE); // TOOD: Clarify default polygon winding order
	SetMultisampling(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);

	VkPipelineColorBlendAttachmentState colorBlendAttachment {};
	colorBlendAttachment.colorWriteMask
		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	SetColorBlending(colorBlendAttachment);

	SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
}

// TOOD: Only one shader per stage -> SetVertexShader, SetFragmentShader
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

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddFragmentShader(unsigned char* const& code, size_t len)
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
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputDescription(const VertexDescription desc)
{
	vertBindingDescriptions = std::get<0>(desc);
	vertInputAttrDescriptions = std::get<1>(desc);
	return *this;
}

// TOOD: Move this into a separate builder
GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddLayoutBinding(
	std::uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags shaderStageFlags)
{
	descriptorSetLayoutBindings.emplace_back(
		Initializers::DescriptorSetLayoutBinding(descriptorType, shaderStageFlags, binding));
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizer(
	const VkPolygonMode polygonMode, const VkCullModeFlags cullMode, const VkFrontFace frontFace)
{
	rasterizer = Initializers::PipelineRasterizationStateCreateInfo(polygonMode, cullMode, frontFace);
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMultisampling(
	VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable)
{
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = sampleShadingEnable;
	multisampling.rasterizationSamples = rasterizationSamples;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlending(
	const VkPipelineColorBlendAttachmentState& colorBlendAttachment)
{
	this->colorBlendAttachment = colorBlendAttachment;
	return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDynamicState(const std::vector<VkDynamicState>& dynamicStates)
{
	this->dynamicStates = dynamicStates;
	return *this;
}

PipelinePtr GraphicsPipelineBuilder::Build()
{
	// --- Pipeline layout
	VkDescriptorSetLayout descriptorSetLayout { VK_NULL_HANDLE };
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo
		= Initializers::DescriptorSetLayoutCreateInfo(descriptorSetLayoutBindings);

	if (vkCreateDescriptorSetLayout(device.GetVkDevice(), &descriptorLayoutInfo, nullptr, &descriptorSetLayout)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = Initializers::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);

	if (vkCreatePipelineLayout(device.GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// --- Pipeline
	VkPipelineViewportStateCreateInfo viewportState {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineColorBlendStateCreateInfo colorBlending {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDynamicStateCreateInfo dynamicState {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = vertBindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = vertBindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = vertInputAttrDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = vertInputAttrDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(device.GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	PipelinePtr pipeline = std::make_shared<Pipeline>(device,
		std::make_tuple(graphicsPipeline, pipelineLayout, descriptorSetLayout), VK_PIPELINE_BIND_POINT_GRAPHICS);

	for (auto shaderStage : shaderStages) {
		vkDestroyShaderModule(device.GetVkDevice(), shaderStage.module, nullptr);
	}

	return pipeline;
}

VkShaderModule GraphicsPipelineBuilder::CreateShaderModule(unsigned char* const& code, size_t len) const
{
	VkShaderModuleCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = len;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device.GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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
	if (vkCreateShaderModule(device.GetVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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
	shaderStages.push_back(shaderStageInfo);
}