#ifndef __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__
#define __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__

#include <vector>
//
#include <grafkit/common.h>
#include <grafkit/core/device.h>

namespace Grafkit::Core
{
	class GKAPI Pipeline
	{
	public:
		Pipeline(Device const& device, std::tuple<VkPipeline, VkPipelineLayout> pipeline, VkPipelineBindPoint pipelineBindPoint);
		virtual ~Pipeline();

		void Bind(VkCommandBuffer commandBuffer);

		[[nodiscard]] const VkPipeline& GetPipeline() const { return pipeline; }
		[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const { return pipelineLayout; }

	private:
		Device const& device;

		VkPipelineBindPoint const pipelineBindPoint;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
	};

	// -----------------------------------------------------------------------------

	class GraphicsPipelineBuilder
	{
	public:
		GraphicsPipelineBuilder(Device const& device, VkRenderPass renderPass);
		~GraphicsPipelineBuilder() = default;

		GraphicsPipelineBuilder& AddVertexShader(unsigned char* const& code, size_t len);
		GraphicsPipelineBuilder& AddVertexShader(const std::vector<char>& code);
		GraphicsPipelineBuilder& AddFragmentShader(unsigned char* const& code, size_t len);
		GraphicsPipelineBuilder& AddFragmentShader(const std::vector<char>& code);

		GraphicsPipelineBuilder& SetVertexInputState();
		GraphicsPipelineBuilder& SetInputAssembly(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable = VK_FALSE);
		GraphicsPipelineBuilder& SetViewportState();
		GraphicsPipelineBuilder& SetRasterizer(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace);
		GraphicsPipelineBuilder& SetMultisampling(VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable = VK_FALSE);
		GraphicsPipelineBuilder& SetColorBlending(const VkPipelineColorBlendAttachmentState& colorBlendAttachment);
		GraphicsPipelineBuilder& SetDynamicState(const std::vector<VkDynamicState>& dynamicStates);

		PipelinePtr Build();

	private:
		Device const& device;
		VkRenderPass renderPass;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
		VkPipelineViewportStateCreateInfo viewportState {};
		VkPipelineRasterizationStateCreateInfo rasterizer {};
		VkPipelineMultisampleStateCreateInfo multisampling {};
		VkPipelineColorBlendStateCreateInfo colorBlending {};
		VkPipelineDynamicStateCreateInfo dynamicState {};

		VkShaderModule CreateShaderModule(unsigned char* const& code, size_t len) const;
		VkShaderModule CreateShaderModule(const std::vector<char>& code) const;

		void AddShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage);
	};
} // namespace Grafkit::Core
#endif // __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__
