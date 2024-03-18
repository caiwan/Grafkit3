#ifndef __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__
#define __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__

#include <vector>
//
#include <grafkit/common.h>
#include <grafkit/core/device.h>

namespace Grafkit::Core {
	class GKAPI Pipeline {
	public:
		Pipeline(Device const& device,
			std::tuple<VkPipeline, VkPipelineLayout, VkDescriptorSetLayout> pipeline,
			VkPipelineBindPoint pipelineBindPoint);
		virtual ~Pipeline();

		void Bind(VkCommandBuffer commandBuffer);

		[[nodiscard]] const VkPipeline& GetPipeline() const { return pipeline; }
		[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const { return pipelineLayout; }

	private:
		Device const& device;

		VkPipelineBindPoint const pipelineBindPoint;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout descriptorSetLayout;
	};

	// -----------------------------------------------------------------------------

	class GKAPI GraphicsPipelineBuilder {
	public:
		GraphicsPipelineBuilder(Device const& device, VkRenderPass renderPass);
		~GraphicsPipelineBuilder() = default;

		GraphicsPipelineBuilder& AddVertexShader(const uint8_t* code, size_t len);
		GraphicsPipelineBuilder& AddVertexShader(const std::vector<char>& code);
		GraphicsPipelineBuilder& AddFragmentShader(const uint8_t* code, size_t len);
		GraphicsPipelineBuilder& AddFragmentShader(const std::vector<char>& code);

		GraphicsPipelineBuilder& SetVertexInputDescription(const VertexDescription desc);

		GraphicsPipelineBuilder& AddLayoutBinding(
			std::uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags shaderStageFlags);

		// GraphicsPipelineBuilder& AddVertexUniformBufferDescription(const uint32_t binding);
		// GraphicsPipelineBuilder& AddFragmentUniformBufferDescription(const uint32_t binding);

		GraphicsPipelineBuilder& SetInputAssembly(
			const VkPrimitiveTopology topology, const VkBool32 primitiveRestartEnable = VK_FALSE);
		GraphicsPipelineBuilder& SetRasterizer(
			const VkPolygonMode polygonMode, const VkCullModeFlags cullMode, const VkFrontFace frontFace);
		GraphicsPipelineBuilder& SetMultisampling(
			const VkSampleCountFlagBits rasterizationSamples, const VkBool32 sampleShadingEnable = VK_FALSE);
		GraphicsPipelineBuilder& SetColorBlending(const VkPipelineColorBlendAttachmentState& m_colorBlendAttachment);
		GraphicsPipelineBuilder& SetDynamicState(const std::vector<VkDynamicState>& m_dynamicStates);

		PipelinePtr Build();

	private:
		Device const& m_device;
		VkRenderPass m_renderPass;

		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
		VkPipelineInputAssemblyStateCreateInfo m_inputAssembly {};

		std::vector<VkVertexInputBindingDescription> m_vertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttrDescriptions;

		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;

		std::vector<VkDescriptorSetLayoutBinding> m_descriptorSetLayoutBindings;

		VkPipelineViewportStateCreateInfo m_viewportState {};
		VkPipelineRasterizationStateCreateInfo m_rasterizer {};
		VkPipelineMultisampleStateCreateInfo m_multisampling {};
		std::vector<VkDynamicState> m_dynamicStates;
		VkPipelineColorBlendAttachmentState m_colorBlendAttachment {};

		VkShaderModule CreateShaderModule(const uint8_t* code, size_t len) const;
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;

		void AddShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage);
	};
} // namespace Grafkit::Core
#endif // __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__
