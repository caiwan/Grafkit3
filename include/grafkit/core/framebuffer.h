#ifndef GRAFKIT_FRAMEBUFFER_H
#define GRAFKIT_FRAMEBUFFER_H

#include <grafkit/common.h>
#include <optional>
#include <span>

#include <ranges>

namespace Grafkit::Core {

	struct FramebufferAttachment {
		ImagePtr image;
		VkFormat format;
		VkImageSubresourceRange subresourceRange;
		VkAttachmentDescription description;

		bool HasDepth()
		{
			std::vector<VkFormat> formats = {
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_X8_D24_UNORM_PACK32,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::ranges::find(formats, format) != std::end(formats);
		}

		bool HasStencil()
		{
			std::vector<VkFormat> formats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::ranges::find(formats, format) != std::end(formats);
		}

		bool IsDepthStencil() { return (HasDepth() || HasStencil()); }
	};

	struct FrameBufferAttachmentInfo {
		std::optional<VkExtent2D> size;
		uint32_t layerCount;
		VkFormat format;
		VkImageUsageFlags usage;
		VkSampleCountFlagBits imageSampleCount;
	};

	class FrameBuffer {
	public:
		FrameBuffer(const DeviceRef& device,
			const SwapChainRef& swapChain,
			const std::vector<FrameBufferAttachmentInfo>& attachments = {});

		virtual ~FrameBuffer();

		void BeginRenderPass(const CommandBufferRef& commandBuffer,
			const uint32_t imageIndex,
			const std::span<VkClearValue> clearValues) const;

		[[nodiscard]] const VkRenderPass& GetVkRenderPass() const { return m_renderPass; }
		[[nodiscard]] size_t GetFrameBufferCount() const { return m_frameBuffers.size(); }

	private:
		const DeviceRef m_device;
		std::vector<VkFramebuffer> m_frameBuffers;
		std::vector<FramebufferAttachment> m_attachments;

		VkExtent2D m_extent {};
		VkSampler m_sampler = VK_NULL_HANDLE;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
		VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;

		ImagePtr m_depthStancilImage;

		void AddAttachment(const FrameBufferAttachmentInfo& info);

		void CreateDepthAttachment();
		void CreateRenderPass();
		void CreateFramebuffer(const SwapChainRef& swapChain);
		void CreateSampler();
	};

	// TODO: Use a FrameBufferBuilder instead of the constructor + Support resize

} // namespace Grafkit::Core

#endif // FRAMEBUFFER_H
