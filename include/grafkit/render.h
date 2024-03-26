#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__

#include <memory>
#include <optional>
#include <vector>
//
#include <grafkit/common.h>
#include <grafkit/core/window.h>
#include <grafkit/core/pipeline.h>

namespace Grafkit
{
	class GKAPI RenderContext
	{
	public:
		explicit RenderContext(const Core::Window& window);
		virtual ~RenderContext();

		[[nodiscard]] VkCommandBuffer BeginCommandBuffer();
		void BeginFrame(VkCommandBuffer& commandBuffer);
		void DrawFrame(VkCommandBuffer& commandBuffer);

		Grafkit::Core::GraphicsPipelineBuilder CreateGraphicsPipelineBuilder() const;

	private:
		const Core::Window& window;

		Core::InstancePtr instance;
		Core::DevicePtr device;
		Core::SwapChainPtr swapChain;

		std::vector<VkFramebuffer> frameBuffers;
		VkRenderPass renderPass;
		VkCommandBuffer commandBuffer;

		size_t currentFrameIndex = 0;

		void InitializeCommandBuffers();
		void InitializeFrameBuffers();
		void InitializeRenderPass();
	};
}
#endif // __RENDER_CONTEXT_H__
