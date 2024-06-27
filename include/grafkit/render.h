#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>
//
#include <grafkit/common.h>
#include <grafkit/core/descriptor.h>
#include <grafkit/core/pipeline.h>
#include <grafkit/render/texture.h>

namespace Grafkit {

	namespace Core {
		using InstancePtr = std::unique_ptr<Instance>;
		using DevicePtr = std::unique_ptr<Device>;
		using SwapChainPtr = std::unique_ptr<SwapChain>;
		using FrameBufferPtr = std::unique_ptr<FrameBuffer>;
		using CommandBufferPtr = std::unique_ptr<CommandBuffer>;
	} // namespace Core

	class GKAPI RenderContext {
	public:
		explicit RenderContext(const Core::WindowRef& window);
		virtual ~RenderContext();

		[[nodiscard]] const Core::DeviceRef GetDevice() const;

		[[nodiscard]] Core::CommandBufferRef BeginCommandBuffer();
		void BeginFrame(const Core::CommandBufferRef& commandBuffer);
		void EndFrame(const Core::CommandBufferRef& commandBuffer);

		void Flush();

		[[nodiscard]] Grafkit::Core::DescriptorBuilder DescriptorBuilder() const;
		[[nodiscard]] Grafkit::Core::GraphicsPipelineBuilder PipelineBuilder() const;

		[[nodiscard]] float GetAspectRatio() const;

		[[nodiscard]] uint32_t GetCurrentFrameIndex() const { return m_currentImageIndex; }
		[[nodiscard]] uint32_t GetNextFrameIndex() const { return m_nextFrameIndex; }

	private:
		const Core::WindowRef window;

		Core::InstancePtr m_instance;
		Core::DevicePtr m_device;
		Core::SwapChainPtr m_swapChain;

		Core::FrameBufferPtr m_frameBuffer;

		std::vector<Core::CommandBufferPtr> m_commandBuffers;

		uint32_t m_currentImageIndex = 0;
		uint32_t m_nextFrameIndex = 0;

		VkViewport m_viewport {};
		VkRect2D m_scissor {};

		void InitializeCommandBuffers();
		void SetupViewport();
	};
} // namespace Grafkit
#endif // __RENDER_CONTEXT_H__
