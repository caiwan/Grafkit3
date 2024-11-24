#ifndef GRAFKIT_RENDER_RENDER_GRAPH_H
#define GRAFKIT_RENDER_RENDER_GRAPH_H

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

	class RenderStage;
	using RenderStagePtr = std::shared_ptr<RenderStage>;

	// MARK: RenderStage
	GKAPI class RenderStage
	{
	public:
		RenderStage(const Core::DeviceRef &device);
		virtual ~RenderStage();

		void Record(Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex);

		void SetClearFlag(bool flag);
		void SetClearColor(uint32_t slot, const VkClearColorValue &color);
		void SetClearDepth(uint32_t slot, const VkClearDepthStencilValue &depthStencil);

		Core::DescriptorSetPtr CrateDescriptorSet(const uint32_t set);

	private:
		const Core::DeviceRef m_device;
		std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_bindings;

		Core::RenderTargetPtr m_renderTarget;
		Core::PipelinePtr m_pipeline;

		// + dependencies
		// + render order

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

		RenderStageBuilder &AddVertexDescription(const Core::VertexDescription &vertexDescription);
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
		std::vector<Core::VertexDescription> m_vertexDescriptions;
		std::vector<VkPushConstantRange> m_pushConstants;
	};

} // namespace Grafkit

#endif // GRAFKIT_RENDER_RENDER_GRAPH_H
