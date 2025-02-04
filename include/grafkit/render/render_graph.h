#ifndef GRAFKIT_RENDER_RENDER_GRAPH_H
#define GRAFKIT_RENDER_RENDER_GRAPH_H

#include <functional>
#include <span>
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

	enum class StageDescriptorType
	{
		None,
		Material,
		RenderStage,
	};

	using StageDescriptorMap = std::unordered_map<StageDescriptorType, Core::DescriptorSetLayoutBindingMap>;

	using OnRecordRenderStageCallbackFn =
		std::function<void(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex)>;

	// MARK: RenderStage
	GKAPI class RenderStage
	{
	public:
		RenderStage(const Core::DeviceRef &device,
			Core::PipelinePtr pipeline,
			Core::RenderTargetPtr renderTarget,
			StageDescriptorMap descriptorSetBindings = {},
			std::vector<VkPushConstantRange> pushConstants = {},
			Core::RenderTargetPtr renderSource = nullptr,
			std::vector<VkClearValue> clearValues = {});

		virtual ~RenderStage();

		RenderStage(const RenderStage &) = delete;
		RenderStage(RenderStage &&) = delete;
		RenderStage &operator=(const RenderStage &) = delete;
		RenderStage &operator=(RenderStage &&) = delete;

		void Record(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex);

		void SetClearFlag(bool flag);
		void SetClearColor(uint32_t slot, const VkClearColorValue &color);
		void SetClearDepth(uint32_t slot, const VkClearDepthStencilValue &depthStencil);

		void SetOnbRecordCallback(const OnRecordRenderStageCallbackFn &callback);

		[[nodiscard]] Core::DescriptorSetLayoutBindingMap GetDescriptorSetLayoutBindings(
			const StageDescriptorType type) const noexcept;

		[[nodiscard]] Core::DescriptorSetPtr CreateDescriptorSet(const uint32_t set) const;

		[[nodiscard]] VkPipelineLayout GetPipelineLayout() const noexcept;
		[[nodiscard]] VkDescriptorSetLayout DescriptorSetLayout(uint32_t index) const noexcept;

		[[nodiscard]] Core::RenderTargetPtr GetRendderTarget() const noexcept;
		[[nodiscard]] Core::RenderTargetPtr GetRenderSource() const noexcept;

		[[nodiscard]] inline bool IsClear() const noexcept
		{
			return m_isClear;
		}

	private:
		const Core::DeviceRef m_device;

		StageDescriptorMap m_descriptorMap;
		std::vector<VkPushConstantRange> m_pushConstants;

		Core::RenderTargetPtr m_renderSource;
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

		void BuildFromStages(std::vector<RenderStagePtr> inStages);

		void Record(const Core::CommandBufferRef &commandBuffer, const uint32_t frameIndex);

		// Internal representation of a render node with dependencies
		struct RenderNode
		{
			RenderStagePtr stage;
			std::vector<RenderStagePtr> dependencies;
		};

	private:
		[[nodiscard]]
		static std::vector<RenderStagePtr> TopologicalSort(std::span<RenderNode> inStages);

		std::vector<RenderStagePtr> m_stages;
	};

	// MARK: Builders
	GKAPI class RenderStageBuilder
	{
	public:
		explicit RenderStageBuilder(const Core::DeviceRef &device);

		virtual ~RenderStageBuilder() = default;

		RenderStageBuilder(const RenderStageBuilder &) = delete;
		RenderStageBuilder(RenderStageBuilder &&) = delete;
		RenderStageBuilder &operator=(const RenderStageBuilder &) = delete;
		RenderStageBuilder &operator=(RenderStageBuilder &&) = delete;

		// MARK: Builder methods
		RenderStageBuilder &SetRenderTarget(const Core::RenderTargetPtr &target);
		RenderStageBuilder &SetRenderSource(const Core::RenderTargetPtr &target);

		RenderStageBuilder &AddDescriptorSetLayoutBinding(const uint32_t set,
			const std::vector<Core::DescriptorBinding> &bindings);
		RenderStageBuilder &AddDescriptorSetLayoutBindings(const Core::DescriptorSetLayoutBindings &descriptors);

		RenderStageBuilder &AddMaterialDescriptorBinding(const uint32_t set,
			const std::vector<Core::DescriptorBinding> &bindings);
		RenderStageBuilder &AddMaterialDescriptorBindings(const Core::DescriptorSetLayoutBindings &descriptors);
		RenderStageBuilder &AddRenderStageDescriptorBinding(const uint32_t set,
			const std::vector<Core::DescriptorBinding> &bindings);
		RenderStageBuilder &AddRenderStageDescriptorBindings(const Core::DescriptorSetLayoutBindings &descriptors);

		RenderStageBuilder &SetVertexInputDescription(const Core::VertexDescription &vertexDescription);
		RenderStageBuilder &AddPushConstantRange(const VkPushConstantRange &range);
		RenderStageBuilder &SetVertexShader(const std::string &shader);
		RenderStageBuilder &SetVertexShader(const uint8_t *code, size_t len);
		RenderStageBuilder &SetFragmentShader(const std::string &shader);
		RenderStageBuilder &SetFragmentShader(const uint8_t *code, size_t len);

		RenderStagePtr Build() const;

	private:
		void AddDescriptorSetLayoutBinding(const StageDescriptorType type,
			const uint32_t set,
			const Core::DescriptorBindings &bindings);

		const Core::DeviceRef m_device;
		Core::GraphicsPipelineBuilderPtr m_pipelineBuilder;
		Core::RenderTargetPtr m_renderTarget;
		Core::RenderTargetPtr m_renderSource;

		std::unordered_map<StageDescriptorType, Core::DescriptorSetLayoutBindingMap> m_descriptorMap;

		std::vector<VkPushConstantRange> m_pushConstants;
	};

} // namespace Grafkit

#endif // GRAFKIT_RENDER_RENDER_GRAPH_H
