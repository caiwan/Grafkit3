#include "stdafx.h"
//
#include "grafkit/core/device.h"
#include "grafkit/core/image.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/instance.h"
#include "grafkit/core/swap_chain.h"
#include "grafkit/core/vulkan_utils.h"
#include "grafkit/core/window.h"

using namespace Grafkit::Core;

constexpr std::array<VkFormat, 4> DEPTH_FORMATS = {
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
};

// ----------------------------------------------------------------------------

SwapChain::SwapChain(const WindowRef &window, const InstanceRef &instance, const DeviceRef &device)
	: m_device(device), m_extent(ChooseSwapExtent(window, device))
{
	m_format = ChooseSwapSurfaceFormat().format;
	m_framesInFlight = m_device->GetMaxConcurrentFrames();
	InitializeSwapChain(window, instance, device);
	InitializeSwapChainImages();
	InitializeSyncObjects();
}

SwapChain::~SwapChain()
{
	WaitForFences();
	for (size_t i = 0; i < m_images.size(); i++)
	{
		m_images[i].reset();
		vkDestroySemaphore(m_device->GetVkDevice(), m_presentCompleteSemaphores[i], nullptr);
		vkDestroySemaphore(m_device->GetVkDevice(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device->GetVkDevice(), m_inFlightFences[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device->GetVkDevice(), m_swapChain, nullptr);
}

// ----------------------------------------------------------------------------
// MARK: Public methods
bool Grafkit::Core::SwapChain::AcquireNextFrame()
{
	vkWaitForFences(m_device->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(m_device->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame]);

	const VkResult result = vkAcquireNextImageKHR(m_device->GetVkDevice(),
		m_swapChain,
		UINT64_MAX,
		m_presentCompleteSemaphores[m_currentFrame],
		VK_NULL_HANDLE,
		&m_imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return false;
	}
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	return true;
}

void SwapChain::SubmitCommandBuffer(const VkCommandBuffer &commandBuffer)
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

	VK_CHECK_RESULT(vkQueueSubmit(m_device->GetVkGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]));
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

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
	{
		// TODO: recreate swap chain -> Handle window resize
		throw std::runtime_error("swap chain out of date!");
	}
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	// TODO: Atomic increment
	m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}

void Grafkit::Core::SwapChain::WaitForFences() noexcept
{
	vkWaitForFences(m_device->GetVkDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
}

// ----------------------------------------------------------------------------
// MARK: Creator methods

void SwapChain::InitializeSwapChain(const WindowRef &window, const InstanceRef &instance, const DeviceRef &device)
{
	const SurfaceProperties surfaceProperties = device->GetSurfaceProperties();
	const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat();
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode();
	const VkExtent2D extent = ChooseSwapExtent(window, device);

	const QueueFamilyIndices indices = device->GetQueueFamilies();
	const std::array<uint32_t, 2> queueFamilyIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = instance->GetVkSurface();

	createInfo.minImageCount = m_framesInFlight;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = surfaceProperties.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(device->GetVkDevice(), &createInfo, nullptr, &m_swapChain))
}

void SwapChain::InitializeSwapChainImages()
{
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_swapChain, &imageCount, nullptr);

	// Determine actual images needed for the swap chain
	std::vector<VkImage> swapChainImagesVk;
	swapChainImagesVk.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_swapChain, &imageCount, swapChainImagesVk.data());

	if (swapChainImagesVk.size() != imageCount)
	{
		throw std::runtime_error("failed to get swap chain images!");
	}

	// Create image views for the swap chain images
	for (const auto &image : swapChainImagesVk)
	{
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

		if (vkCreateImageView(m_device->GetVkDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}

		m_images.emplace_back(std::make_shared<Image>(m_device, image, imageView));
	}
}

void SwapChain::InitializeSyncObjects()
{
	for (size_t i = 0; i < m_framesInFlight; i++)
	{
		VkSemaphoreCreateInfo semaphoreInfo = Initializers::SemaphoreCreateInfo();
		VkFenceCreateInfo fenceInfo = Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

		m_presentCompleteSemaphores.emplace_back();
		m_renderFinishedSemaphores.emplace_back();
		m_inFlightFences.emplace_back();

		VK_CHECK_RESULT(
			vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreInfo, nullptr, &m_presentCompleteSemaphores.back()));
		VK_CHECK_RESULT(
			vkCreateSemaphore(m_device->GetVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores.back()));
		VK_CHECK_RESULT(vkCreateFence(m_device->GetVkDevice(), &fenceInfo, nullptr, &m_inFlightFences.back()));
	}
}

// MARK: Helper methods
VkExtent2D SwapChain::ChooseSwapExtent(const WindowRef &window, const DeviceRef &device) const
{
	const auto swapChainSupport = device->GetSurfaceProperties();
	const auto &capabilities = swapChainSupport.capabilities;
	const auto windowSize = window->GetBufferSize();

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	return {
		std::clamp(static_cast<uint32_t>(windowSize.width),
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width),
		std::clamp(static_cast<uint32_t>(windowSize.height),
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height),
	};
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode() const
{
	// TODO: Vsync goes here (for now, just mailbox)
	const SurfaceProperties swapChainSupport = m_device->GetSurfaceProperties();
	const std::vector<VkPresentModeKHR> &availablePresentModes = swapChainSupport.presentModes;

	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
		if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkFormat SwapChain::ChooseDepthFormat() const
{
	VkFormatProperties props{};
	for (VkFormat format : DEPTH_FORMATS)
	{
		vkGetPhysicalDeviceFormatProperties(m_device->GetVkPhysicalDevice(), format, &props);

		if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat() const
{
	const SurfaceProperties swapChainSupport = m_device->GetSurfaceProperties();
	const std::vector<VkSurfaceFormatKHR> &surfaceFormats = swapChainSupport.formats;

	for (const auto &surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return surfaceFormat;
		}
	}

	return surfaceFormats.front();
}
