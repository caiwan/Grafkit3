#include "stdafx.h"
//
#include <grafkit/core/device.h>
#include <grafkit/core/image.h>
#include <grafkit/core/initializers.h>
#include <grafkit/core/instance.h>
#include <grafkit/core/swap_chain.h>
#include <grafkit/core/window.h>

using namespace Grafkit::Core;

// ----------------------------------------------------------------------------

SwapChain::SwapChain(const WindowRef& window, const InstanceRef& instance, const DeviceRef& device)
	: m_device(device)
	, m_format(device->ChooseSwapSurfaceFormat().format)
	, m_extent(ChooseSwapExtent(window, device))
{
	InitializeSwapChain(window, instance, device);
	InitializeSwapChainImages();
	InitializeSyncObjects();
}

SwapChain::~SwapChain()
{
	WaitForFences();
	for (size_t i = 0; i < m_images.size(); i++) {
		m_images[i].reset();
		vkDestroySemaphore(m_device->GetVkDevice(), m_presentCompleteSemaphores[i], nullptr);
		vkDestroySemaphore(m_device->GetVkDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device->GetVkDevice(), m_inFlightFences[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device->GetVkDevice(), m_swapChain, nullptr);
}

// ----------------------------------------------------------------------------

uint32_t Grafkit::Core::SwapChain::AcquireNextFrame()
{
	WaitForFences();
	vkResetFences(m_device->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame]);

	m_imageIndex = 0;

	const auto result = vkAcquireNextImageKHR(m_device->GetVkDevice(),
		m_swapChain,
		UINT64_MAX,
		m_presentCompleteSemaphores[m_currentFrame],
		VK_NULL_HANDLE,
		&m_imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		// TODO: recreate swap chain -> Handle window resize
		throw std::runtime_error("swap chain out of date!");
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	return m_imageIndex;
}

void SwapChain::SubmitCommandBuffer(const VkCommandBuffer& commandBuffer)
{

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = Initializers::SubmitInfo();
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentCompleteSemaphores[m_currentFrame];
	submitInfo.pWaitDstStageMask = &waitStageMask;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

	if (vkQueueSubmit(m_device->GetVkGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void SwapChain::Present()
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = &m_imageIndex;

	const auto result = vkQueuePresentKHR(m_device->GetVkPresentQueue(), &presentInfo);

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		// TODO: recreate swap chain -> Handle window resize
		throw std::runtime_error("swap chain out of date!");
	}
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % static_cast<uint32_t>(m_images.size());
}

void Grafkit::Core::SwapChain::WaitForFences() noexcept
{
	vkWaitForFences(m_device->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
}

// ----------------------------------------------------------------------------

VkExtent2D SwapChain::ChooseSwapExtent(const WindowRef& window, const DeviceRef& device) const
{
	const auto swapChainSupport = device->GetSwapChainSupportDetails();
	const auto& capabilities = swapChainSupport.capabilities;
	const auto windowSize = window->GetBufferSize();

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	VkExtent2D actualExtent = { static_cast<uint32_t>(windowSize.width), static_cast<uint32_t>(windowSize.height) };

	actualExtent.width
		= std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height
		= std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void SwapChain::InitializeSwapChain(const WindowRef& window, const InstanceRef& instance, const DeviceRef& device)
{
	const SwapChainSupportDetails swapChainSupport = device->GetSwapChainSupportDetails();
	const VkSurfaceFormatKHR surfaceFormat = device->ChooseSwapSurfaceFormat();
	const VkPresentModeKHR presentMode = device->ChooseSwapPresentMode();
	const VkExtent2D extent = ChooseSwapExtent(window, device);

	const QueueFamilyIndices indices = device->GetQueueFamilies();
	const std::array<uint32_t, 2> queueFamilyIndices
		= { indices.graphicsFamily.value(), indices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = instance->GetVkSurface();

	createInfo.minImageCount = device->GetMaxFramesInFlight();
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR createdSwapChian;

	if (vkCreateSwapchainKHR(device->GetVkDevice(), &createInfo, nullptr, &createdSwapChian) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	m_swapChain = createdSwapChian;
}

void SwapChain::InitializeSwapChainImages()
{
	uint32_t imageCount = m_device->GetMaxFramesInFlight();

	std::vector<VkImage> swapChainImagesVk;
	vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_swapChain, &imageCount, nullptr);
	swapChainImagesVk.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_swapChain, &imageCount, swapChainImagesVk.data());

	if (swapChainImagesVk.size() != imageCount) {
		throw std::runtime_error("failed to get swap chain images!");
	}

	// Create image views for the swap chain images
	// const auto surfaceFormat = m_device->ChooseSwapSurfaceFormat();

	for (const auto& image : swapChainImagesVk) {

		VkImageViewCreateInfo createInfo = Initializers::ImageViewCreateInfo();
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;

		if (vkCreateImageView(m_device->GetVkDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}

		m_images.emplace_back(std::make_shared<Image>(m_device, image, imageView));
	}
}

void SwapChain::InitializeSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo = Initializers::SemaphoreCreateInfo();
	VkFenceCreateInfo fenceInfo = Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	for (size_t i = 0; i < m_images.size(); i++) {
		m_presentCompleteSemaphores.emplace_back();
		m_renderFinishedSemaphores.emplace_back();
		m_inFlightFences.emplace_back();

		if (vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreInfo, nullptr, &m_presentCompleteSemaphores.back())
				!= VK_SUCCESS
			|| vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores.back())
				!= VK_SUCCESS
			|| vkCreateFence(m_device->GetVkDevice(), &fenceInfo, nullptr, &m_inFlightFences.back()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
