#include "stdafx.h"
#include <grafkit/core/command_buffer.h>
#include <grafkit/core/device.h>
#include <grafkit/core/framebuffer.h>
#include <grafkit/core/image.h>
#include <grafkit/core/initializers.h>
#include <grafkit/core/swap_chain.h>
#include <vk_mem_alloc.h>

using namespace Grafkit;
using namespace Grafkit::Core;

FrameBuffer::FrameBuffer(
	const DeviceRef& device, const SwapChainRef& swapChain, const std::vector<FrameBufferAttachmentInfo>& attachments)
	: m_device(device)
{
	m_extent = swapChain->GetExtent();
	m_colorFormat = swapChain->GetFormat();
	m_depthFormat = device->ChooseDepthFormat();

	CreateDepthAttachment();

	// Add other attachments if any
	for (const auto& attachment : attachments) {
		AddAttachment(attachment);
	}

	CreateRenderPass();
	CreateFramebuffer(swapChain);
	CreateSampler();
}

FrameBuffer::~FrameBuffer()
{
	vkDestroyRenderPass(**m_device, m_renderPass, nullptr);

	for (auto* framebuffer : m_frameBuffers) {
		vkDestroyFramebuffer(m_device->GetVkDevice(), framebuffer, nullptr);
	}

	for (auto attachment : m_attachments) {
		attachment.image.reset();
	}

	m_depthStancilImage.reset();

	vkDestroySampler(m_device->GetVkDevice(), m_sampler, nullptr);
}

void Grafkit::Core::FrameBuffer::BeginRenderPass(
	const CommandBufferRef& commandBuffer, const uint32_t imageIndex, const std::span<VkClearValue> clearValues) const
{
	VkRenderPassBeginInfo renderPassInfo = Core::Initializers::RenderPassBeginInfo();
	renderPassInfo.pNext = nullptr;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_extent;
	renderPassInfo.framebuffer = m_frameBuffers[imageIndex];

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(**commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Grafkit::Core::FrameBuffer::AddAttachment(const FrameBufferAttachmentInfo& info)
{
	FramebufferAttachment attachment {};
	attachment.format = info.format;

	VkImageAspectFlags aspectMask = 0;

	// Select aspect mask and layout depending on usage
	// Color attachment
	if (info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// Depth (and/or stencil) attachment
	if (info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		if (attachment.HasDepth()) {
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (attachment.HasStencil()) {
			aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}

	assert(aspectMask > 0);

	// Attachment description
	attachment.subresourceRange = {};
	attachment.subresourceRange.aspectMask = aspectMask;
	attachment.subresourceRange.levelCount = 1;
	attachment.subresourceRange.layerCount = info.layerCount;

	// Create image
	// TOOD: Add image attachment creation to the image utility functions

	const VkExtent3D extent = info.size.has_value() ? VkExtent3D({ info.size->width, info.size->height, 1 })
													: VkExtent3D({ m_extent.width, m_extent.height, 1 });

	VkImageCreateInfo imageInfo = Initializers::ImageCreateInfo(VK_IMAGE_TYPE_2D, extent, info.format, info.usage);
	imageInfo.arrayLayers = info.layerCount;
	imageInfo.samples = info.imageSampleCount;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	if (vmaCreateImage(m_device->GetVmaAllocator(), &imageInfo, &allocInfo, &image, &allocation, nullptr)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create image");
	}

	VkImageView imageView = VK_NULL_HANDLE;

	// Create image view
	VkImageViewCreateInfo viewInfo = Initializers::ImageViewCreateInfo();
	viewInfo.viewType = (info.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	viewInfo.format = info.format;
	viewInfo.subresourceRange = attachment.subresourceRange;
	viewInfo.subresourceRange.aspectMask = (attachment.HasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
	viewInfo.image = image;

	if (vkCreateImageView(m_device->GetVkDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view");
	}

	attachment.image = std::make_shared<Image>(m_device, image, imageView, VK_IMAGE_LAYOUT_UNDEFINED, allocation);

	// Fill attachment description
	attachment.description = {};
	attachment.description.samples = info.imageSampleCount;
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.description.storeOp
		= (info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.format = info.format;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Final layout
	// If not, final layout depends on attachment type
	if (attachment.HasDepth() || attachment.HasStencil()) {
		attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	} else {
		attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	// Add to the attachment list
	m_attachments.push_back(attachment);
}

void Grafkit::Core::FrameBuffer::CreateDepthAttachment()
{

	VkImage image = VK_NULL_HANDLE;
	VmaAllocation imageAllocation = VK_NULL_HANDLE;

	VkImageCreateInfo imageInfo {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = m_depthFormat;
	imageInfo.extent = { m_extent.width, m_extent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VmaAllocationCreateInfo imageAllocInfo = {};
	imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imageAllocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vmaCreateImage(m_device->GetVmaAllocator(), &imageInfo, &imageAllocInfo, &image, &imageAllocation, nullptr)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkImageView imageView = VK_NULL_HANDLE;

	VkImageViewCreateInfo imageViewInfo {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.image = image;
	imageViewInfo.format = m_depthFormat;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// Stencil aspect should only be set on depth + stencil formats
	// (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		imageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	if (vkCreateImageView(m_device->GetVkDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}

	m_depthStancilImage
		= std::make_shared<Image>(m_device, image, imageView, VK_IMAGE_LAYOUT_UNDEFINED, imageAllocation);
}

void Grafkit::Core::FrameBuffer::CreateRenderPass()
{
#if 0 // TODO: Add attachment descriptions and references
// Ez egy szar, ha nem jo semmire akkro ki kell baszni innen

	std::vector<VkAttachmentDescription> attachmentDescriptions;
	for (auto& attachment : m_attachments) {
		attachmentDescriptions.push_back(attachment.description);
	};

	// Collect attachment references
	std::vector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference = {};
	bool hasDepth = false;

	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); // Swap chain image
	uint32_t attachmentIndex = 1; // Skip the first attachment (swap chain image)

	// Collect attachment references
	for (auto& attachment : m_attachments) {
		if (attachment.IsDepthStencil()) {
			// Only one depth attachment allowed
			assert(!hasDepth);
			depthReference.attachment = attachmentIndex;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			hasDepth = true;
		} else {
			colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		}
		attachmentIndex++;
	};

	// Default render pass setup uses only one subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	if (!colorReferences.empty()) {
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	}
	if (hasDepth) {
		subpass.pDepthStencilAttachment = &depthReference;
	}

	// ...

#else // Re-initiate old render pass

	const auto mSwapChainImageFormat = m_device->ChooseSwapSurfaceFormat().format;

	std::array<VkAttachmentDescription, 2> attachments = {};
	// Color attachment
	attachments[0].format = m_colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask
		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].dstStageMask
		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask
		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = 0;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(m_device->GetVkDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

#endif
}

void Grafkit::Core::FrameBuffer::CreateFramebuffer(const SwapChainRef& swapChain)
{
	const size_t frameBufferCount = swapChain->GetImageCount();

	// Create framebuffers for each swap chain image
	for (size_t index = 0; index < frameBufferCount; index++) {
		m_frameBuffers.emplace_back();

		std::vector<VkImageView> attachmentViews;
		attachmentViews.resize(m_attachments.size() + 2);

		// Add color + depth attachment views to the list first
		attachmentViews[0] = swapChain->GetImage(index)->GetImageView();
		attachmentViews[1] = m_depthStancilImage->GetImageView();

		// Then add the rest of the attachments
		size_t attachmentIndex = 2;
		for (const auto& attachment : m_attachments) {
			attachmentViews[attachmentIndex] = attachment.image->GetImageView();
			attachmentIndex++;
		}

		// Find. max number of layers across attachments
		uint32_t maxLayers = 1;
		for (const auto& attachment : m_attachments) {
			if (attachment.subresourceRange.layerCount > maxLayers) {
				maxLayers = attachment.subresourceRange.layerCount;
			}
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.pAttachments = attachmentViews.data();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
		framebufferInfo.width = m_extent.width;
		framebufferInfo.height = m_extent.height;
		framebufferInfo.layers = maxLayers;
		if (vkCreateFramebuffer(m_device->GetVkDevice(), &framebufferInfo, nullptr, &m_frameBuffers.back())
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Grafkit::Core::FrameBuffer::CreateSampler()
{
	const VkSamplerCreateInfo samplerInfo = Initializers::SamplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
	if (vkCreateSampler(m_device->GetVkDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create sampler!");
	}
}
