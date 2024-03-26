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
#include <grafkit/core/device.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using namespace Grafkit::Core;

Device::Device(const Core::Instance& instance)
	: m_instance(instance)
	, m_physicalDevice(PickPhysicalDevice(instance.GetVkInstance()))
	, device(CreateLogicalDevice(instance.GetVkInstance()))
	, m_graphicsQueue(CreateGraphicsQueue())
	, m_presentQueue(CreatePresentQueue())
	, m_commandPool(CreateCommandPool())
{
	InitializeAllocator();
}

Device::~Device()
{
	WaitIdle();

	vmaDestroyAllocator(m_allocator);
	vkDestroyCommandPool(device, m_commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

void Device::WaitIdle() const { vkDeviceWaitIdle(device); }

VkCommandBuffer Device::BeginSingleTimeCommands() const
{
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

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

	vkFreeCommandBuffers(device, m_commandPool, 1, &commandBuffer);
}

// -----------------------------------------------------------------------------------------------------------------------------------

VkPhysicalDevice Device::PickPhysicalDevice(const VkInstance& instance) const
{

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return physicalDevice;
}

VkDevice Device::CreateLogicalDevice(const VkInstance& instance) const
{
	QueueFamilyIndices indices = FindQueueFamilies();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(Instance::deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = Instance::deviceExtensions.data();

	if (Instance::enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(Instance::validationLayers.size());
		createInfo.ppEnabledLayerNames = Instance::validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	VkDevice createdPhysicalDevice;
	vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &createdPhysicalDevice);
	return createdPhysicalDevice;
}

VkQueue Device::CreateGraphicsQueue() const
{
	QueueFamilyIndices indices = FindQueueFamilies();
	VkQueue createdQueue;
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &createdQueue);
	return createdQueue;
}

VkQueue Device::CreatePresentQueue() const
{
	QueueFamilyIndices indices = FindQueueFamilies();
	VkQueue createdQueue;
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &createdQueue);
	return createdQueue;
}

VkCommandPool Device::CreateCommandPool() const
{
	QueueFamilyIndices indices = FindQueueFamilies();

	VkCommandPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

	VkCommandPool createdCommandPool;
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &createdCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
	return createdCommandPool;
}

bool Device::IsDeviceSuitable(const VkPhysicalDevice& device) const
{
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices Device::FindQueueFamilies(const VkPhysicalDevice& device) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_instance.GetVkSurface(), &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		++i;
	}

	return indices;
}

SwapChainSupportDetails Device::QuerySwapChainSupport(const VkPhysicalDevice& device) const
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_instance.GetVkSurface(), &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance.GetVkSurface(), &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_instance.GetVkSurface(), &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_instance.GetVkSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, m_instance.GetVkSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool Device::CheckDeviceExtensionSupport(const VkPhysicalDevice& device) const
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(Instance::deviceExtensions.begin(), Instance::deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void Device::InitializeAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.m_physicalDevice = m_physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.m_instance = m_instance.GetVkInstance();
	vmaCreateAllocator(&allocatorInfo, &m_allocator);
}
