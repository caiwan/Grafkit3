
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
//
#include <grafkit/core/swap_chain.h>

using namespace Grafkit::Core;

// ----------------------------------------------------------------------------

SwapChain::SwapChain(const Window& window, const Instance& instance, const Device& device)
	: device(device)
	, extent(ChooseSwapExtent(window, device))
	, swapChain(CreateSwapChain(window, instance, device))
{
	InitializeSwapChainImages();
	InitializeImageViews();
	InitializeSyncObjects();
}

SwapChain::~SwapChain()
{

	vkWaitForFences(device.GetVkDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	device.WaitIdle();

	for (auto& imageView : imageViews)
	{
		vkDestroyImageView(device.GetVkDevice(), imageView, nullptr);
	}

	vkDestroyFence(device.GetVkDevice(), inFlightFence, nullptr);
	vkDestroySemaphore(device.GetVkDevice(), renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device.GetVkDevice(), imageAvailableSemaphore, nullptr);

	vkDestroySwapchainKHR(device.GetVkDevice(), swapChain, nullptr);
}

// ----------------------------------------------------------------------------

void Grafkit::Core::SwapChain::AcquireNextFrame()
{
	vkWaitForFences(device.GetVkDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device.GetVkDevice(), 1, &inFlightFence);

	vkAcquireNextImageKHR(device.GetVkDevice(), swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex);
}

void SwapChain::SubmitCommandBuffer(const VkCommandBuffer& commandBuffer)
{
	VkSubmitInfo submitInfo {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(device.GetVkGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void SwapChain::Present()
{
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };

	VkPresentInfoKHR presentInfo {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &currentImageIndex;

	vkQueuePresentKHR(device.GetVkPresentQueue(), &presentInfo);
}

// ----------------------------------------------------------------------------

VkExtent2D SwapChain::ChooseSwapExtent(const Window& window, const Device& device) const
{
	const auto swapChainSupport = device.QuerySwapChainSupport();
	const auto& capabilities = swapChainSupport.capabilities;
	const auto windowSize = window.GetBufferSize();

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { static_cast<uint32_t>(windowSize.width), static_cast<uint32_t>(windowSize.height) };

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkSwapchainKHR SwapChain::CreateSwapChain(const Window& window, const Instance& instance, const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(device);
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode(device);
	const VkExtent2D extent = ChooseSwapExtent(window, device);

	const QueueFamilyIndices indices = device.FindQueueFamilies();
	const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = instance.GetVkSurface();

	createInfo.minImageCount = FindImageCount(device);
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR createdSwapChian;

	if (vkCreateSwapchainKHR(device.GetVkDevice(), &createInfo, nullptr, &createdSwapChian) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	return createdSwapChian;
}

void SwapChain::InitializeSwapChainImages()
{
	uint32_t imageCount = FindImageCount(device);

	std::vector<VkImage> swapChainImagesVk;
	vkGetSwapchainImagesKHR(device.GetVkDevice(), swapChain, &imageCount, nullptr);
	swapChainImagesVk.resize(imageCount);
	vkGetSwapchainImagesKHR(device.GetVkDevice(), swapChain, &imageCount, swapChainImagesVk.data());

	if (swapChainImagesVk.size() != imageCount)
	{
		throw std::runtime_error("failed to get swap chain images!");
	}

	for (const auto& image : swapChainImagesVk)
	{
		images.emplace_back(image);
	}
}

void SwapChain::InitializeImageViews()
{
	// Create image views for the swap chain images
	const auto surfaceFormat = ChooseSwapSurfaceFormat(device);
	imageViews.resize(images.size());
	
	for (size_t i = 0; i < images.size(); i++)
	{
		VkImageViewCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = surfaceFormat.format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.GetVkDevice(), &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void SwapChain::InitializeSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device.GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
		|| vkCreateSemaphore(device.GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS
		|| vkCreateFence(device.GetVkDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
}

// ----------------------------------------------------------------------------

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	const std::vector<VkSurfaceFormatKHR>& availableFormats = swapChainSupport.formats;

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	const std::vector<VkPresentModeKHR>& availablePresentModes = swapChainSupport.presentModes;

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t SwapChain::FindImageCount(const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	return imageCount;
}
