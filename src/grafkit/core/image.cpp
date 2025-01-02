#include "stdafx.h"

#include "grafkit/core/buffer.h"
#include "grafkit/core/device.h"
#include "grafkit/core/image.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/vulkan_utils.h"

using namespace Grafkit::Core;

constexpr bool USE_IMAGE_MEMORY_BARRIER_2 = true;

Grafkit::Core::Image::Image(const DeviceRef &device,
	const VkImage &image,
	const VkImageView &imageView,
	const VkImageLayout layout,
	const std::optional<VmaAllocation> allocation)
	: m_device(device)
	, m_image(image)
	, m_imageView(imageView)
	, m_layout(layout)
	, m_allocation(allocation)
{
}

Grafkit::Core::Image::~Image()
{
	vkDestroyImageView(**m_device, m_imageView, nullptr);
	if (m_allocation.has_value())
	{
		vmaDestroyImage(m_device->GetVmaAllocator(), m_image, m_allocation.value());
	}
}

/// ---------------------------------------------------------------------------------------------

ImagePtr Image::CreateImage(const DeviceRef &device,
	const VkExtent3D size,
	const VkFormat format,
	const VkImageType type,
	const bool mipmapped,
	const VkImageUsageFlags usage,
	const VkImageLayout layout)
{
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VkImageCreateInfo imageInfo = Initializers::ImageCreateInfo(type, size, format, usage);

	if (mipmapped)
	{
		imageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	VK_CHECK_RESULT(vmaCreateImage(device->GetVmaAllocator(), &imageInfo, &allocInfo, &image, &allocation, nullptr));

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT)
	{
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	VkImageView imageView = VK_NULL_HANDLE;

	// build a image-view for the image
	VkImageViewCreateInfo imageViewInfo = Initializers::ImageViewCreateInfo();
	imageViewInfo.image = image;
	imageViewInfo.format = format;

	switch (type)
	{
	case VK_IMAGE_TYPE_1D:
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
		break;
	case VK_IMAGE_TYPE_2D:
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case VK_IMAGE_TYPE_3D:
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		break;
		// TODO: Add cube map support -> layerCount
	default:
		throw std::runtime_error("unsupported image type");
	}

	imageViewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
	imageViewInfo.subresourceRange.aspectMask = aspectFlag;

	VK_CHECK_RESULT(vkCreateImageView(**device, &imageViewInfo, nullptr, &imageView));

	return std::make_shared<Image>(device, image, imageView, layout, allocation);
}

ImagePtr Image::CreateImage(const DeviceRef &device,
	const void *data,
	const VkExtent3D size,
	const uint32_t channels,
	const VkFormat format,
	const VkImageType type,
	const bool mipmapped,
	const VkImageUsageFlags usage,
	const VkImageLayout layout)
{
	const size_t dataSize = size.depth * size.width * size.height * channels;
	Buffer uploadbuffer =
		Buffer::CreateBuffer(device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	if (data != nullptr)
	{
		uploadbuffer.Update(device, data, dataSize);
	}

	ImagePtr newImage = Image::CreateImage(device,
		size,
		format,
		type,
		mipmapped,
		usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		layout);

	auto *commandBuffer = device->BeginSingleTimeCommands();

	TransitionImageLayout(commandBuffer,
		newImage->GetImage(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkBufferImageCopy copyRegion = {};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;

	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent = size;

	vkCmdCopyBufferToImage(commandBuffer,
		uploadbuffer.buffer,
		newImage->GetImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion);

	TransitionImageLayout(commandBuffer, newImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layout);

	device->EndSingleTimeCommands(commandBuffer);

	uploadbuffer.Destroy(device);

	return newImage;
}

void Image::TransitionImageLayout(VkCommandBuffer &command,
	const VkImage &image,
	const VkImageLayout currentLayout,
	const VkImageLayout newLayout)
{

	if constexpr (USE_IMAGE_MEMORY_BARRIER_2)
	{

		VkImageMemoryBarrier2 imageBarrier = Initializers::ImageMemoryBarrier2();
		imageBarrier.pNext = nullptr;

		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

		imageBarrier.oldLayout = currentLayout;
		imageBarrier.newLayout = newLayout;

		VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
										  ? VK_IMAGE_ASPECT_DEPTH_BIT
										  : VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange = Initializers::ImageSubresourceRange(aspectMask);
		imageBarrier.image = image;

		VkDependencyInfo depInfo{};
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;

		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		vkCmdPipelineBarrier2(command, &depInfo);
	}
	else
	{

		// This has to support mip levels separately !!!!

		// TODO: Add support for depth images
		// TODO: Add support for other image aspects

		VkImageMemoryBarrier barrier = Initializers::ImageMemoryBarrier();
		barrier.oldLayout = currentLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(command, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
}
