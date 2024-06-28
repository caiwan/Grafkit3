#ifndef GRAFKIT_CORE_GRAPHICS_PIPELINE_H
#define GRAFKIT_CORE_GRAPHICS_PIPELINE_H

#include <unordered_map>
#include <vector>
//
#include <grafkit/common.h>

namespace Grafkit::Core {
	class GKAPI Pipeline {
	public:
		explicit Pipeline(const DeviceRef& device,
			std::tuple<VkPipeline, VkPipelineLayout, std::vector<VkDescriptorSetLayout>>
				pipeline, // TODO: Replace with struct
			VkPipelineBindPoint pipelineBindPoint);
		virtual ~Pipeline();

		void Bind(VkCommandBuffer commandBuffer);

		[[nodiscard]] const VkPipeline& GetPipeline() const { return m_pipeline; }
		[[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const { return m_pipelineLayout; }

		// TOOD: Get descriptor set layout

	private:
		const DeviceRef m_device;

		VkPipelineBindPoint const m_pipelineBindPoint;
		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
	};

	// -----------------------------------------------------------------------------

	class GKAPI GraphicsPipelineBuilder {
	public:
		explicit GraphicsPipelineBuilder(const DeviceRef& device, const FrameBufferRef& renderPass); // REPLACE !!!!
		~GraphicsPipelineBuilder() = default;

		GraphicsPipelineBuilder& AddVertexShader(const uint8_t* code, size_t len);
		GraphicsPipelineBuilder& AddVertexShader(const std::vector<char>& code);
		GraphicsPipelineBuilder& AddFragmentShader(const uint8_t* code, size_t len);
		GraphicsPipelineBuilder& AddFragmentShader(const std::vector<char>& code);

		GraphicsPipelineBuilder& SetVertexInputDescription(const VertexDescription& desc);

		GraphicsPipelineBuilder& AddDescriptorSets(const std::vector<SetDescriptor>& bindings);
		GraphicsPipelineBuilder& AddPushConstants(
			const VkShaderStageFlags stage, const uint32_t size, const uint32_t offset = 0);

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
		const DeviceRef m_device;
		const FrameBufferRef m_frameBuffer; // REPLACE !!!!

		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
		VkPipelineInputAssemblyStateCreateInfo m_inputAssembly {};

		std::vector<VkVertexInputBindingDescription> m_vertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttrDescriptions;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_descriptorSets;

		std::vector<VkPushConstantRange> m_pushConstants;

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
