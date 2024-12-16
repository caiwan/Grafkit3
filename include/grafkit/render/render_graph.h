#ifndef GRAFKIT_RENDER_RENDER_GRAPH_H
#define GRAFKIT_RENDER_RENDER_GRAPH_H

#include <functional>
#include <unordered_map>
#include <vector>

#include <grafkit/common.h>

namespace Grafkit
{
	namespace Core
	{
		class GraphicsPipelineBuilder;
		using GraphicsPipelineBuilderPtr = std::unique_ptr<GraphicsPipelineBuilder>;

		class RenderPass;
		class Pipeline;
	} // namespace Core

	using OnRecordRenderStageCallbackFn =
		std::function<void(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)>;

	// MARK: RenderStage
	GKAPI class RenderStage
	{
	public:
		RenderStage(const Core::DeviceRef &device,
			Core::PipelinePtr pipeline,
			std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings,
			std::vector<VkPushConstantRange> pushConstants = {});

		virtual ~RenderStage();

		void GetDescriporSetLayoutBindings(std::vector<uint32_t> &sets) const noexcept;
		[[nodiscard]] Core::DescriptorSetPtr CrateDescriptorSet(const uint32_t set) const;

		void Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex);

		void SetClearFlag(bool flag);
		void SetClearColor(uint32_t slot, const VkClearColorValue &color);
		void SetClearDepth(uint32_t slot, const VkClearDepthStencilValue &depthStencil);
		void SetOnbRecordCallback(const OnRecordRenderStageCallbackFn &callback);

	private:
		const Core::DeviceRef m_device;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_bindings;
		std::vector<VkPushConstantRange> m_pushConstants;

		Core::RenderTargetPtr m_renderTarget;
		Core::PipelinePtr m_pipeline;

		OnRecordRenderStageCallbackFn m_onRecordCallback = nullptr;

		std::vector<VkClearValue> m_clearValues = {};
		bool m_isClear = false;
	};

	// MARK: RenderGraph
	GKAPI class RenderGraph
	{
	public:
		RenderGraph() = default;
		virtual ~RenderGraph() = default;

		void BuildFromStages(const std::vector<RenderStagePtr> &stages);

		void Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex);

	private:
		std::vector<RenderStagePtr> m_stages;
	};

	// MARK: Builders
	GKAPI class RenderStageBuilder
	{
	public:
		explicit RenderStageBuilder(const Core::DeviceRef &device);

		virtual ~RenderStageBuilder() = default;

		RenderStageBuilder &SetRenderTarget(const Core::RenderTargetPtr &target);

		RenderStageBuilder &AddDescriptorSetLayoutBinding(const uint32_t set,
			const std::vector<Core::DescriptorBinding> &bindings);
		RenderStageBuilder &AddDescriptorSetLayoutBindings(const Core::DescriptorSetLayoutBindings &descriptors);

		RenderStageBuilder &SetVertexInputDescription(const Core::VertexDescription &vertexDescription);
		RenderStageBuilder &AddPushConstantRange(const VkPushConstantRange &range);

		RenderStageBuilder &SetVertexShader(const std::string &shader);
		RenderStageBuilder &SetVertexShader(const uint8_t *code, size_t len);
		RenderStageBuilder &SetFragmentShader(const std::string &shader);
		RenderStageBuilder &SetFragmentShader(const uint8_t *code, size_t len);

		RenderStagePtr Build();

	private:
		const Core::DeviceRef m_device;
		Core::GraphicsPipelineBuilderPtr m_pipelineBuilder;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_bindings;
		std::vector<VkPushConstantRange> m_pushConstants;
	};

} // namespace Grafkit

#endif // GRAFKIT_RENDER_RENDER_GRAPH_H
