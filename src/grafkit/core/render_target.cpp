#include "stdafx.h"

#include <deque>

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/device.h"
#include "grafkit/core/image.h"
#include "grafkit/core/render_target.h"
#include "grafkit/core/swap_chain.h"
#include "grafkit/core/vulkan_utils.h"

namespace Grafkit::Core
{

	// MARK: RenderTarget
	RenderTarget::RenderTarget(const DeviceRef &device,
		const VkRenderPass renderPass,
		const VkExtent2D extent,
		std::vector<VkFramebuffer> frameBuffers,
		bool isOffscreen,
		const VkSampler sample,
		std::vector<RenderTargetAttachment> attachments)
		: m_device(device)
		, m_renderPass(renderPass)
		, m_extent(extent)
		, m_frameBuffers(std::move(frameBuffers))
		, m_attachments(std::move(attachments))
		, m_sampler(sample)
		, m_isOffscreen(isOffscreen)
	{
		SetupViewport();
	}

	RenderTarget::~RenderTarget()
	{
		vkDestroySampler(m_device->GetVkDevice(), m_sampler, nullptr);
		vkDestroyRenderPass(m_device->GetVkDevice(), m_renderPass, nullptr);
		for (auto &frameBuffer : m_frameBuffers)
		{
			vkDestroyFramebuffer(m_device->GetVkDevice(), frameBuffer, nullptr);
		}

		for (auto &attachment : m_attachments)
		{
			if (attachment.image)
			{
				attachment.image.reset();
			}
		}
	}

	VkRenderPassBeginInfo Grafkit::Core::RenderTarget::CreateRenderPassBeginInfo(const uint32_t index) const
	{
		if (m_isOffscreen)
		{
			return Core::Initializers::RenderPassBeginInfo(m_renderPass, m_frameBuffers.front(), m_extent);
		}
		assert(index < m_frameBuffers.size());
		return Core::Initializers::RenderPassBeginInfo(m_renderPass, m_frameBuffers[index], m_extent);
	}

	void Grafkit::Core::RenderTarget::SetupViewport(const Core::CommandBufferRef &commandBuffer) const noexcept
	{
		vkCmdSetScissor(**commandBuffer, 0, 1, &m_scissor);
		vkCmdSetViewport(**commandBuffer, 0, 1, &m_viewport);
	}

	VkRenderPass Grafkit::Core::RenderTarget::GetRenderPass() const
	{
		return m_renderPass;
	}

	size_t Grafkit::Core::RenderTarget::GetFrameBufferCount() const
	{
		return m_frameBuffers.size();
	}

	VkFramebuffer Grafkit::Core::RenderTarget::GetFrameBuffer(const uint32_t index) const
	{
		assert(index < m_frameBuffers.size());
		return m_frameBuffers[index];
	}

	size_t Grafkit::Core::RenderTarget::GetAttachmentCount() const
	{
		return m_attachments.size();
	}

	VkFormat Grafkit::Core::RenderTarget::GetAttachmentFormat(const uint32_t index) const
	{
		assert(index < m_attachments.size());
		return m_attachments[index].format;
	}

	bool Grafkit::Core::RenderTarget::GetAttachemntIsDepthStencil(const uint32_t index) const
	{
		assert(index < m_attachments.size());
		return m_attachments[index].IsDepthStencil();
	}

	void Grafkit::Core::RenderTarget::SetupViewport()
	{
		m_viewport.x = 0.0f;
		m_viewport.y = 0.0f;
		m_viewport.width = static_cast<float>(m_extent.width);
		m_viewport.height = static_cast<float>(m_extent.height);
		m_viewport.minDepth = 0.0f;
		m_viewport.maxDepth = 1.0f;

		m_scissor.offset = {0, 0};
		m_scissor.extent = m_extent;
	}

	// MARK: RenderTargetBuilder

	RenderTargetBuilder::RenderTargetBuilder(const DeviceRef &device)
		: m_device(device)
	{
	}

	RenderTargetBuilder &RenderTargetBuilder::UseSwapChain(const SwapChainRef &swapChain)
	{
		m_swapChainAttachments.clear();
		m_swapChainAttachments.reserve(swapChain->GetImageCount());
		for (uint32_t index = 0; index < swapChain->GetImageCount(); ++index)
		{
			RenderTargetAttachmentInfo &info = m_swapChainAttachments.emplace_back();

			info.image = swapChain->GetImage(index);
			info.format = swapChain->GetFormat();
			info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			info.imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
		}

		m_extent = swapChain->GetExtent();

		return *this;
	}

	RenderTargetBuilder &RenderTargetBuilder::CreateFromSwapChain(const SwapChainRef &swapChain)
	{
		m_swapChainAttachments.clear();
		AddAttachment(swapChain->GetFormat(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SAMPLE_COUNT_1_BIT);
		m_extent = swapChain->GetExtent();

		return *this;
	}

	RenderTargetBuilder &RenderTargetBuilder::AddAttachments(const std::vector<RenderTargetAttachmentInfo> &attachments)
	{
		std::ranges::copy(attachments, std::back_inserter(m_attachments));
		return *this;
	}

	RenderTargetBuilder &RenderTargetBuilder::AddAttachment(const RenderTargetAttachmentInfo &attachments)
	{
		m_attachments.push_back(attachments);
		return *this;
	}

	RenderTargetBuilder &Grafkit::Core::RenderTargetBuilder::AddAttachment(const VkFormat format,
		const VkImageUsageFlags usage,
		const VkSampleCountFlagBits sampleCount)
	{
		m_attachments.push_back({nullptr, 1, format, usage, sampleCount});
		return *this;
	}
	RenderTargetBuilder &RenderTargetBuilder::SetSize(const VkExtent2D &size)
	{
		m_extent = size;
		return *this;
	}

	RenderTargetPtr RenderTargetBuilder::Build() const
	{
		// Crate Attachments
		std::vector<RenderTargetAttachment> renderTargetAttachments;
		renderTargetAttachments.reserve(m_attachments.size() + (m_swapChainAttachments.empty() ? 0 : 1));

		// Create swap chain attachment if needed
		if (!m_swapChainAttachments.empty())
		{
			RenderTargetAttachment swapChainAttachment = {};

			const auto &swapChain = m_swapChainAttachments.front();
			swapChainAttachment.format = swapChain.format;
			swapChainAttachment.description.format = swapChain.format;
			swapChainAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
			swapChainAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			swapChainAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			swapChainAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			swapChainAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			swapChainAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			swapChainAttachment.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			swapChainAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			renderTargetAttachments.push_back(swapChainAttachment);
		}

		for (const auto &attachment : m_attachments)
		{
			renderTargetAttachments.push_back(CreateAttachment(attachment));
		}

		// Create color attachment references
		std::vector<VkAttachmentReference> colorAttachmentRefs;
		std::optional<VkAttachmentReference> depthAttachmentRef;
		std::vector<VkAttachmentDescription> attachments;
		for (const auto &attachment : renderTargetAttachments)
		{
			attachments.push_back(attachment.description);
			if (attachment.IsDepthStencil() && !depthAttachmentRef.has_value())
			{
				depthAttachmentRef = {static_cast<uint32_t>(attachments.size() - 1), attachment.layout};
			}
			else
			{
				colorAttachmentRefs.push_back({static_cast<uint32_t>(attachments.size() - 1), attachment.layout});
			}
		}

		// Create subpass
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
		subpassDescription.pColorAttachments = colorAttachmentRefs.data();
		subpassDescription.pDepthStencilAttachment =
			depthAttachmentRef.has_value() ? &depthAttachmentRef.value() : nullptr;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Create render pass
		std::array<VkSubpassDependency, 2> dependencies;

		const bool renderToOffscreen = m_swapChainAttachments.empty();
		if (!renderToOffscreen)
		{
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].dstSubpass = 0;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].srcAccessMask = 0;
			dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			dependencies[1].dependencyFlags = 0;
		}
		else
		{
			// Use subpass dependencies for layout transitions
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VkRenderPass renderPass = VK_NULL_HANDLE;

		if (vkCreateRenderPass(m_device->GetVkDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}

		// Create frame buffers
		const size_t frameBufferCount = m_swapChainAttachments.empty() ? 1 : m_swapChainAttachments.size();
		std::vector<VkFramebuffer> frameBuffers;
		frameBuffers.reserve(frameBufferCount);
		for (size_t index = 0; index < frameBufferCount; index++)
		{
			frameBuffers.emplace_back();

			std::vector<VkImageView> attachmentViews;
			attachmentViews.reserve(renderTargetAttachments.size());

			if (!m_swapChainAttachments.empty())
			{
				assert(index < m_swapChainAttachments.size());
				ImagePtr image = m_swapChainAttachments[index].image;
				if (image != nullptr)
				{
					attachmentViews.emplace_back(image->GetImageView());
				}
			}

			// Then, add the rest of the attachments
			for (const auto &attachment : renderTargetAttachments)
			{
				if (attachment.image != nullptr)
				{
					attachmentViews.emplace_back(attachment.image->GetImageView());
				}
			}

			// Find max number of layers across attachments
			uint32_t maxLayers = 1;
			for (const auto &attachment : renderTargetAttachments)
			{
				if (attachment.subresourceRange.layerCount > maxLayers)
				{
					maxLayers = attachment.subresourceRange.layerCount;
				}
			}

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.pAttachments = attachmentViews.data();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
			framebufferInfo.width = m_extent.width;
			framebufferInfo.height = m_extent.height;
			framebufferInfo.layers = maxLayers;
			if (vkCreateFramebuffer(m_device->GetVkDevice(), &framebufferInfo, nullptr, &frameBuffers.back()) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}

		// Create sampler
		VkSampler sampler = VK_NULL_HANDLE;
		const VkSamplerCreateInfo samplerInfo = Initializers::SamplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		if (vkCreateSampler(m_device->GetVkDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create sampler!");
		}

		// Create render target
		return std::make_unique<RenderTarget>(m_device,
			renderPass,
			m_extent,
			std::move(frameBuffers),
			renderToOffscreen,
			sampler,
			std::move(renderTargetAttachments));
	}

	RenderTargetAttachment RenderTargetBuilder::CreateAttachment(const RenderTargetAttachmentInfo &info) const
	{
		// TOOD: Use Image::CreateImage
		RenderTargetAttachment attachment;
		attachment.format = info.format;

		// Determine image layout
		VkImageAspectFlags aspectMask = 0;

		// Select aspect mask and layout depending on usage
		// Color attachment
		if (info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		// Depth (and/or stencil) attachment
		if (info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			if (attachment.HasDepth())
			{
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			if (attachment.HasStencil())
			{
				aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

		assert(aspectMask > 0);
		assert(info.layerCount > 0);

		// Attachment description
		// TODO: Add mip levels if needed later
		attachment.subresourceRange = {};
		attachment.subresourceRange.aspectMask = aspectMask;
		attachment.subresourceRange.baseMipLevel = 0;
		attachment.subresourceRange.levelCount = 1;
		attachment.subresourceRange.baseArrayLayer = 0;
		attachment.subresourceRange.layerCount = info.layerCount;

		// Create image
		// TOOD: Add image attachment creation to the image utility functions
		const bool isCreatedImage = info.image != nullptr;
		if (info.image == nullptr)
		{
			assert(info.format != VK_FORMAT_UNDEFINED);
			assert(info.usage != 0);
			assert(m_extent.width > 0 && m_extent.height > 0);

			const VkExtent3D extent = {m_extent.width, m_extent.height, 1};

			VkImageCreateInfo imageInfo =
				Initializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, extent, info.format, info.usage);
			imageInfo.arrayLayers = info.layerCount;
			imageInfo.samples = info.imageSampleCount;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			allocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkImage image = VK_NULL_HANDLE;
			VmaAllocation allocation = VK_NULL_HANDLE;

			VK_CHECK_RESULT(
				vmaCreateImage(m_device->GetVmaAllocator(), &imageInfo, &allocInfo, &image, &allocation, nullptr));

			VkImageView imageView = VK_NULL_HANDLE;

			// Create image view
			VkImageViewCreateInfo viewInfo = Initializers::ImageViewCreateInfo();
			viewInfo.viewType = (info.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = info.format;
			viewInfo.subresourceRange = attachment.subresourceRange;
			viewInfo.subresourceRange.aspectMask = (attachment.HasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
			viewInfo.image = image;

			VK_CHECK_RESULT(vkCreateImageView(m_device->GetVkDevice(), &viewInfo, nullptr, &imageView));

			attachment.image =
				std::make_shared<Image>(m_device, image, imageView, VK_IMAGE_LAYOUT_UNDEFINED, allocation);
		}
		else
		{
			attachment.image = info.image;
		}

		// Fill attachment description
		attachment.description = {};
		attachment.description.samples = info.imageSampleCount;
		attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.description.storeOp =
			(info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.description.format = info.format;
		attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// Final layout
		// If not, final layout depends on attachment type
		if (attachment.IsDepthStencil())
		{
			attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		return attachment;
	}

} // namespace Grafkit::Core
