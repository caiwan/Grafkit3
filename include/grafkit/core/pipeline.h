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

		GraphicsPipelineBuilder& AddVertexShader(unsigned char* const& code, size_t len);
		GraphicsPipelineBuilder& AddVertexShader(const std::vector<char>& code);
		GraphicsPipelineBuilder& AddFragmentShader(unsigned char* const& code, size_t len);
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
		GraphicsPipelineBuilder& SetColorBlending(const VkPipelineColorBlendAttachmentState& colorBlendAttachment);
		GraphicsPipelineBuilder& SetDynamicState(const std::vector<VkDynamicState>& dynamicStates);

		PipelinePtr Build();

	private:
		Device const& device;
		VkRenderPass renderPass;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly {};

		std::vector<VkVertexInputBindingDescription> vertBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> vertInputAttrDescriptions;

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

		VkPipelineViewportStateCreateInfo viewportState {};
		VkPipelineRasterizationStateCreateInfo rasterizer {};
		VkPipelineMultisampleStateCreateInfo multisampling {};
		std::vector<VkDynamicState> dynamicStates;
		VkPipelineColorBlendAttachmentState colorBlendAttachment {};

		VkShaderModule CreateShaderModule(unsigned char* const& code, size_t len) const;
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;

		void AddShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage);
	};
} // namespace Grafkit::Core
#endif // __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__
