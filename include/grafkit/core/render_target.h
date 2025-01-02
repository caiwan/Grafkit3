#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include <ranges>
#include <vector>

#include <grafkit/common.h>

namespace Grafkit::Core
{
	struct RenderTargetAttachment
	{
		ImagePtr image;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageSubresourceRange subresourceRange = {};
		VkAttachmentDescription description = {};
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		inline bool HasDepth() const
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

		inline bool HasStencil() const
		{
			std::vector<VkFormat> formats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::ranges::find(formats, format) != std::end(formats);
		}

		inline bool IsDepthStencil() const
		{
			return (HasDepth() || HasStencil());
		}
	};

	struct RenderTargetAttachmentInfo
	{
		ImagePtr image;
		uint32_t layerCount = 1;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSampleCountFlagBits imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
	};

	// MARK: RenderTarget
	class RenderTarget
	{
	public:
		explicit RenderTarget(const DeviceRef &device,
			const VkRenderPass renderPass,
			const VkExtent2D extent,
			std::vector<VkFramebuffer> frameBuffers,
			const VkSampler sampler,
			std::vector<RenderTargetAttachment> attachments);

		RenderTarget(const RenderTarget &other) = delete;
		RenderTarget &operator=(const RenderTarget &other) = delete;

		virtual ~RenderTarget();

		[[nodiscard]] VkRenderPassBeginInfo CreateRenderPassBeginInfo(const uint32_t index) const;
		void SetupViewport(const Core::CommandBufferRef &commandBuffer) const noexcept;

		// Getters
		[[nodiscard]] VkRenderPass GetRenderPass() const;
		[[nodiscard]] size_t GetFrameBufferCount() const;
		[[nodiscard]] VkFramebuffer GetFrameBuffer(const uint32_t index) const;
		[[nodiscard]] size_t GetAttachmentCount() const;
		[[nodiscard]] VkFormat GetAttachmentFormat(const uint32_t index) const;
		[[nodiscard]] bool GetAttachemntIsDepthStencil(const uint32_t index) const;

		[[nodiscard]] inline VkExtent2D GetExtent() const noexcept
		{
			return m_extent;
		}

		[[nodiscard]] inline VkViewport GetViewport() const noexcept
		{
			return m_viewport;
		}

		[[nodiscard]] inline VkRect2D GetScissor() const noexcept
		{
			return m_scissor;
		}

		[[nodiscard]] inline VkSampler GetSampler() const noexcept
		{
			return m_sampler;
		}

	private:
		const DeviceRef m_device;
		VkSampler m_sampler = VK_NULL_HANDLE;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		VkExtent2D m_extent = {};
		VkViewport m_viewport{};
		VkRect2D m_scissor{};

		std::vector<VkFramebuffer> m_frameBuffers;
		std::vector<RenderTargetAttachment> m_attachments;

		void SetupViewport();
	};

	// MARK: RenderTargetBuilder
	class RenderTargetBuilder
	{
	public:
		explicit RenderTargetBuilder(const DeviceRef &device);

		RenderTargetBuilder &CreateAttachments(const SwapChainRef &swapChain);
		RenderTargetBuilder &AddAttachments(const std::vector<RenderTargetAttachmentInfo> &attachments);
		RenderTargetBuilder &AddAttachment(const RenderTargetAttachmentInfo &attachment);
		RenderTargetBuilder &AddAttachment(const VkFormat format,
			const VkImageUsageFlags usage,
			const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
		RenderTargetBuilder &SetSize(const VkExtent2D &size);

		RenderTargetPtr Build() const;

	private:
		const DeviceRef m_device;

		VkExtent2D m_extent = {};

		std::vector<RenderTargetAttachmentInfo> m_swapChainAttachments;
		std::vector<RenderTargetAttachmentInfo> m_attachments;

		RenderTargetAttachment CreateAttachment(const RenderTargetAttachmentInfo &info) const;
	};

} // namespace Grafkit::Core

#endif // RENDER_TARGET_H
