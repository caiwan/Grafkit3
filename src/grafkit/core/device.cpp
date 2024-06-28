#include "stdafx.h"
#include <sstream>
//
#include <grafkit/core/descriptor_pool.h>
#include <grafkit/core/device.h>
#include <grafkit/core/log.h>

#include <vk_mem_alloc.h>

using namespace Grafkit::Core;

constexpr uint32_t INITIAL_DESCRIPTOR_SET_SIZE = 2;

constexpr VkFormat DEPTH_FORMATS[] = {
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
};

Device::Device(const Core::InstanceRef& instance)
	: m_instance(instance)

{
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateGraphicsQueue();
	CreatePresentQueue();
	CreateCommandPool();

	InitializeAllocator();

	m_descriptorPool = std::make_unique<Core::DescriptorPool>(MakeReference(*this),
		INITIAL_DESCRIPTOR_SET_SIZE,
		std::vector<DescriptorPool::PoolSet>({
			{ VK_DESCRIPTOR_TYPE_SAMPLER, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, INITIAL_DESCRIPTOR_SET_SIZE },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, INITIAL_DESCRIPTOR_SET_SIZE },
		}));
}

Device::~Device()
{
	WaitIdle();

	m_descriptorPool.reset();

	vmaDestroyAllocator(m_allocator);
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyDevice(m_device, nullptr);
}

void Device::WaitIdle() const { vkDeviceWaitIdle(m_device); }

VkCommandBuffer Device::BeginSingleTimeCommands() const
{
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Device::EndSingleTimeCommands(const VkCommandBuffer& commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

[[nodiscard]] DescriptorPoolRef Device::GetDescriptorPool() const { return MakeReference(*m_descriptorPool); }

// -----------------------------------------------------------------------------------------------------------------------------------

void Device::PickPhysicalDevice()
{

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance->GetVkInstance(), &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance->GetVkInstance(), &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Device::CreateLogicalDevice()
{
	const auto indices = GetQueueFamilies();

	const std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures {};

	VkDeviceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(Instance::DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = Instance::DEVICE_EXTENSIONS.data();

	std::stringstream logExtList {};
	logExtList << "Device extensions: ";
	for (const auto& extension : Instance::DEVICE_EXTENSIONS) {
		logExtList << extension << " ";
	}
	Log::Instance().Info(logExtList.str());

	if (Instance::ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(Instance::VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = Instance::VALIDATION_LAYERS.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
}

void Device::CreateGraphicsQueue()
{
	QueueFamilyIndices indices = GetQueueFamilies();
	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
}

void Device::CreatePresentQueue()
{
	QueueFamilyIndices indices = GetQueueFamilies();
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void Device::CreateCommandPool()
{
	QueueFamilyIndices indices = GetQueueFamilies();

	VkCommandPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

bool Device::IsDeviceSuitable(const VkPhysicalDevice& device) const
{
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QueryPhisicalDeviceSurfaceProperties(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices Device::FindQueueFamilies(const VkPhysicalDevice& physicalDevice) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	uint32_t index = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = index;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, m_instance->GetVkSurface(), &presentSupport);

		if (presentSupport) {
			indices.presentFamily = index;
		}

		if (indices.IsComplete()) {
			break;
		}

		++index;
	}

	return indices;
}

SwapChainSupportDetails Device::QueryPhisicalDeviceSurfaceProperties(const VkPhysicalDevice& physicalDevice) const
{
	SwapChainSupportDetails details {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_instance->GetVkSurface(), &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_instance->GetVkSurface(), &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice, m_instance->GetVkSurface(), &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_instance->GetVkSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, m_instance->GetVkSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool Device::CheckDeviceExtensionSupport(const VkPhysicalDevice& device) const
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(Instance::DEVICE_EXTENSIONS.begin(), Instance::DEVICE_EXTENSIONS.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void Device::InitializeAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_physicalDevice;
	allocatorInfo.device = m_device;
	allocatorInfo.instance = m_instance->GetVkInstance();
	vmaCreateAllocator(&allocatorInfo, &m_allocator);
}

// ----------------------------------------------------------------------------

QueueFamilyIndices Device::GetQueueFamilies() const
{
	if (!m_queueFamilyIndices.has_value()) {
		m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice);
	}
	return m_queueFamilyIndices.value();
}

[[nodiscard]] SwapChainSupportDetails Device::GetSwapChainSupportDetails() const
{
	if (!m_swapChainSupportDetails.has_value()) {
		m_swapChainSupportDetails = QueryPhisicalDeviceSurfaceProperties(m_physicalDevice);
	}
	return m_swapChainSupportDetails.value();
}

VkSurfaceFormatKHR Device::ChooseSwapSurfaceFormat() const
{
	const SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails();
	const std::vector<VkSurfaceFormatKHR>& availableFormats = swapChainSupport.formats;

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Device::ChooseSwapPresentMode() const
{
	const SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails();
	const std::vector<VkPresentModeKHR>& availablePresentModes = swapChainSupport.presentModes;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkFormat Device::ChooseDepthFormat() const
{
	VkFormatProperties props {};
	for (VkFormat format : DEPTH_FORMATS) {
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

uint32_t Device::GetMaxFramesInFlight() const
{
	if (!m_maxImageCount.has_value()) {
		const SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails();
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0
			&& imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		m_maxImageCount = imageCount;
	}

	return m_maxImageCount.value();
}
