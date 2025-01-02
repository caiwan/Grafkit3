#ifndef GRAFKIT_CORE_GRAPHICS_PIPELINE_H
#define GRAFKIT_CORE_GRAPHICS_PIPELINE_H

#include <optional>
#include <unordered_map>
#include <vector>
//
#include <grafkit/common.h>

namespace Grafkit::Core
{
	// MARK: Pipeline
	class GKAPI Pipeline
	{
	public:
		explicit Pipeline(const DeviceRef &device,
			std::tuple<VkPipeline, VkPipelineLayout, std::vector<VkDescriptorSetLayout>>
				pipeline, // TODO: Replace with struct
			VkPipelineBindPoint pipelineBindPoint);
		virtual ~Pipeline();

		void Bind(const CommandBufferRef &commandBuffer);

		[[nodiscard]] inline VkPipeline GetPipeline() const noexcept
		{
			return m_pipeline;
		}

		[[nodiscard]] inline VkPipelineLayout GetPipelineLayout() const noexcept
		{
			return m_pipelineLayout;
		}

		[[nodiscard]] inline VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index) noexcept
		{
			assert(index < m_descriptorSetLayouts.size());
			return m_descriptorSetLayouts[index];
		}

	private:
		const DeviceRef m_device;

		VkPipelineBindPoint const m_pipelineBindPoint;
		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
	};

	// MARK: GraphicsPipelineBuilder
	class GKAPI GraphicsPipelineBuilder
	{
	public:
		explicit GraphicsPipelineBuilder(const DeviceRef &device);

		~GraphicsPipelineBuilder() = default;

		GraphicsPipelineBuilder &SetRenderTarget(const Core::RenderTargetPtr &target);

		GraphicsPipelineBuilder &AddVertexShader(const uint8_t *code, size_t len);
		GraphicsPipelineBuilder &AddVertexShader(const std::string &code);
		GraphicsPipelineBuilder &AddFragmentShader(const uint8_t *code, size_t len);
		GraphicsPipelineBuilder &AddFragmentShader(const std::string &code);

		GraphicsPipelineBuilder &SetVertexInputDescription(const VertexDescription &desc);

		GraphicsPipelineBuilder &AddDescriptorSet(const uint32_t set,
			const std::vector<Core::DescriptorBinding> &bindings);
		GraphicsPipelineBuilder &AddDescriptorSets(const DescriptorSetLayoutBindings &bindings);
		GraphicsPipelineBuilder &
		AddPushConstants(const VkShaderStageFlags stage, const uint32_t size, const uint32_t offset = 0);

		// Additional pipeline settings
		GraphicsPipelineBuilder &SetInputAssembly(const VkPrimitiveTopology topology,
			const VkBool32 primitiveRestartEnable = VK_FALSE);
		GraphicsPipelineBuilder &
		SetRasterizer(const VkPolygonMode polygonMode, const VkCullModeFlags cullMode, const VkFrontFace frontFace);
		GraphicsPipelineBuilder &SetMultisampling(const VkSampleCountFlagBits rasterizationSamples,
			const VkBool32 sampleShadingEnable = VK_FALSE);
		GraphicsPipelineBuilder &SetColorBlending(const VkPipelineColorBlendAttachmentState &m_colorBlendAttachment);
		GraphicsPipelineBuilder &SetDynamicState(const std::vector<VkDynamicState> &m_dynamicStates);

		PipelinePtr Build();

	private:
		const DeviceRef m_device;
		RenderTargetPtr m_renderTarget;

		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
		VkPipelineInputAssemblyStateCreateInfo m_inputAssembly{};

		std::vector<VkVertexInputBindingDescription> m_vertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttrDescriptions;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_descriptorSetBindings;

		std::vector<VkPushConstantRange> m_pushConstants;

		VkPipelineViewportStateCreateInfo m_viewportState{};
		VkPipelineRasterizationStateCreateInfo m_rasterizer{};
		VkPipelineMultisampleStateCreateInfo m_multisampling{};
		std::vector<VkDynamicState> m_dynamicStates;
		VkPipelineColorBlendAttachmentState m_colorBlendAttachment{};

		VkShaderModule CreateShaderModule(const uint8_t *code, size_t len) const;
		VkShaderModule CreateShaderModule(const std::string &code) const;

		void AddShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits stage);
	};

	// MARK: ComputePipelineBuilder
	class GKAPI ComputePipelineBuilder
	{
		// TODO: ...
	};

} // namespace Grafkit::Core
#endif // __GRAFKIT_CORE_GRAPHICS_PIPELINE_H__
