
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
#include <grafkit/core/initializers.h>
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
	WaitForFences();
	for (size_t i = 0; i < images.size(); i++) {
		vkDestroyImageView(device.GetVkDevice(), imageViews[i], nullptr);
		vkDestroySemaphore(device.GetVkDevice(), presentCompleteSemaphores[i], nullptr);
		vkDestroySemaphore(device.GetVkDevice(), renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device.GetVkDevice(), inFlightFences[i], nullptr);
	}

	vkDestroySwapchainKHR(device.GetVkDevice(), swapChain, nullptr);
}

// ----------------------------------------------------------------------------

uint32_t Grafkit::Core::SwapChain::AcquireNextFrame()
{
	WaitForFences();
	vkResetFences(device.GetVkDevice(), 1, &inFlightFences[currentFrame]);

	imageIndex = 0;

	const auto result = vkAcquireNextImageKHR(device.GetVkDevice(),
		swapChain,
		UINT64_MAX,
		presentCompleteSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// TODO: recreate swap chain -> Handle window resize
		throw std::runtime_error("swap chain out of date!");
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	return imageIndex;
}

void SwapChain::SubmitCommandBuffer(const VkCommandBuffer& commandBuffer)
{

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = Initializers::SubmitInfo();
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask = &waitStageMask;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	if (vkQueueSubmit(device.GetVkGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void SwapChain::Present()
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imageIndex;

	const auto result = vkQueuePresentKHR(device.GetVkPresentQueue(), &presentInfo);

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		// TODO: recreate swap chain -> Handle window resize
		throw std::runtime_error("swap chain out of date!");
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % static_cast<uint32_t>(images.size());
}

void Grafkit::Core::SwapChain::WaitForFences()
{
	vkWaitForFences(device.GetVkDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

// ----------------------------------------------------------------------------

VkExtent2D SwapChain::ChooseSwapExtent(const Window& window, const Device& device) const
{
	const auto swapChainSupport = device.QuerySwapChainSupport();
	const auto& capabilities = swapChainSupport.capabilities;
	const auto windowSize = window.GetBufferSize();

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = { static_cast<uint32_t>(windowSize.width), static_cast<uint32_t>(windowSize.height) };

		actualExtent.width
			= std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height
			= std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

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

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR createdSwapChian;

	if (vkCreateSwapchainKHR(device.GetVkDevice(), &createInfo, nullptr, &createdSwapChian) != VK_SUCCESS) {
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

	if (swapChainImagesVk.size() != imageCount) {
		throw std::runtime_error("failed to get swap chain images!");
	}

	for (const auto& image : swapChainImagesVk) {
		images.emplace_back(image);
	}
}

void SwapChain::InitializeImageViews()
{
	// Create image views for the swap chain images
	const auto surfaceFormat = ChooseSwapSurfaceFormat(device);
	imageViews.resize(images.size());

	for (size_t i = 0; i < images.size(); i++) {
		VkImageViewCreateInfo createInfo = Initializers::ImageViewCreateInfo();
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

		if (vkCreateImageView(device.GetVkDevice(), &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void SwapChain::InitializeSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo = Initializers::SemaphoreCreateInfo();
	VkFenceCreateInfo fenceInfo = Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	for (size_t i = 0; i < images.size(); i++) {
		presentCompleteSemaphores.emplace_back();
		renderFinishedSemaphores.emplace_back();
		inFlightFences.emplace_back();

		if (vkCreateSemaphore(device.GetVkDevice(), &semaphoreInfo, nullptr, &presentCompleteSemaphores.back())
				!= VK_SUCCESS
			|| vkCreateSemaphore(device.GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores.back())
				!= VK_SUCCESS
			|| vkCreateFence(device.GetVkDevice(), &fenceInfo, nullptr, &inFlightFences.back()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

// ----------------------------------------------------------------------------

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	const std::vector<VkSurfaceFormatKHR>& availableFormats = swapChainSupport.formats;

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	const std::vector<VkPresentModeKHR>& availablePresentModes = swapChainSupport.presentModes;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t SwapChain::FindImageCount(const Device& device) const
{
	const SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport();
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	return imageCount;
}
