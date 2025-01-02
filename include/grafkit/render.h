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

namespace Grafkit
{
	namespace Core
	{
		using InstancePtr = std::unique_ptr<Instance>;
		using DevicePtr = std::unique_ptr<Device>;
		using SwapChainPtr = std::unique_ptr<SwapChain>;
		using CommandBufferPtr = std::unique_ptr<CommandBuffer>;
	} // namespace Core

	// MARK: RenderContext Interface
	class GKAPI IRenderContext
	{
	public:
		virtual ~IRenderContext() = default;

		[[nodiscard]] virtual const Core::DeviceRef GetDevice() const = 0;
		[[nodiscard]] virtual const Core::RenderTargetPtr GetRenderTarget() const = 0;
		[[nodiscard]] virtual Core::CommandBufferRef BeginCommandBuffer() = 0;
		virtual void EndFrame(const Core::CommandBufferRef &commandBuffer) = 0;
		virtual void Flush() = 0;
	};

	// MARK: BaseRenderContext
	class GKAPI BaseRenderContext : public IRenderContext
	{
	public:
		explicit BaseRenderContext(const Core::WindowRef &window);
		~BaseRenderContext() override;

		[[nodiscard]] const Core::DeviceRef GetDevice() const final
		{
			return MakeReference(*m_device);
		}

		[[nodiscard]] const Core::RenderTargetPtr GetRenderTarget() const final
		{
			return m_renderTarget;
		}

		[[nodiscard]] Core::CommandBufferRef BeginCommandBuffer() final;

		void EndFrame(const Core::CommandBufferRef &commandBuffer) final;

		void Flush() final;

		[[nodiscard]] float GetAspectRatio() const;
		[[nodiscard]] VkExtent2D GetExtent() const;

		[[nodiscard]] uint32_t GetNextFrameIndex() const
		{
			return m_frameIndex;
		}

	private:
		const Core::WindowRef m_window;

		Core::InstancePtr m_instance;
		Core::DevicePtr m_device;
		Core::SwapChainPtr m_swapChain;

		Core::RenderTargetPtr m_renderTarget;

		std::vector<Core::CommandBufferPtr> m_commandBuffers;

		uint32_t m_frameIndex = 0;

		void InitializeCommandBuffers();
	};

	// MARK: Render context

	class GKAPI RenderContext : public BaseRenderContext
	{
	public:
		explicit RenderContext(const Core::WindowRef &window);
		~RenderContext() override = default;

	private:
	};

} // namespace Grafkit

#endif // __RENDER_CONTEXT_H__
