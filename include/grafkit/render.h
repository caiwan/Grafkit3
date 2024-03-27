#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__

#include <memory>
#include <optional>
#include <vector>
//
#include <grafkit/common.h>
#include <grafkit/core/device.h>
#include <grafkit/core/pipeline.h>
#include <grafkit/core/window.h>

namespace Grafkit {
	class GKAPI RenderContext {
	public:
		explicit RenderContext(const Core::Window& window);
		virtual ~RenderContext();

		[[nodiscard]] inline Core::Device& GetDevice() const { return *m_device; }

		[[nodiscard]] VkCommandBuffer BeginCommandBuffer();
		void BeginFrame(VkCommandBuffer& commandBuffer);
		void EndFrame(VkCommandBuffer& commandBuffer);

		void Flush();

		Grafkit::Core::GraphicsPipelineBuilder CreateGraphicsPipelineBuilder() const;

	private:
		const Core::Window& window;

		Core::InstancePtr m_instance;
		Core::DevicePtr m_device;
		Core::SwapChainPtr swapChain;

		std::vector<VkFramebuffer> frameBuffers;
		VkRenderPass renderPass;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentFrameIndex = 0;
		uint32_t nextFrameIndex = 0;

		void InitializeCommandBuffers();
		void InitializeFrameBuffers();
		void InitializeRenderPass();
	};
} // namespace Grafkit
#endif // __RENDER_CONTEXT_H__
