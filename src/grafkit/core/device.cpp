#include "stdafx.h"
#include <iomanip>
#include <sstream>
//
#include <vk_mem_alloc.h>

#include "grafkit/core/descriptor_pool.h"
#include "grafkit/core/device.h"
#include "grafkit/core/log.h"

using namespace Grafkit::Core;

constexpr uint32_t INITIAL_DESCRIPTOR_SET_SIZE = 128;
constexpr uint32_t INITIAL_DESCRIPTOR_MAX_SETS = 8;

Device::Device(const Core::InstanceRef &instance) : m_instance(instance)

{
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateGraphicsQueue();
	CreatePresentQueue();
	CreateCommandPool();

	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);

#ifdef _DEBUG
	PrintVulkanDeviceLimits();
#endif

	InitializeAllocator();

	m_descriptorPool = std::make_unique<Core::DescriptorPool>(MakeReference(*this),
		INITIAL_DESCRIPTOR_MAX_SETS,
		std::vector<DescriptorPool::PoolSet>({
			{VK_DESCRIPTOR_TYPE_SAMPLER, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, INITIAL_DESCRIPTOR_SET_SIZE},
			{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, INITIAL_DESCRIPTOR_SET_SIZE},
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

// MARK: Public methods
void Device::WaitIdle() const
{
	vkDeviceWaitIdle(m_device);
}

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

void Device::EndSingleTimeCommands(const VkCommandBuffer &commandBuffer) const
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

[[nodiscard]] DescriptorPoolRef Device::GetDescriptorPool() const
{
	return MakeReference(*m_descriptorPool);
}

// -----------------------------------------------------------------------------------------------------------------------------------
// MARK: Auxiliary methods

void Device::PickPhysicalDevice()
{

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance->GetVkInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance->GetVkInstance(), &deviceCount, devices.data());

	for (const auto &device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void Device::CreateLogicalDevice()
{
	const auto indices = GetQueueFamilies();

	const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(Instance::DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = Instance::DEVICE_EXTENSIONS.data();

	std::stringstream logExtList{};
	logExtList << "Device extensions: ";
	for (const auto &extension : Instance::DEVICE_EXTENSIONS)
	{
		logExtList << extension << " ";
	}
	Log::Instance().Info(logExtList.str());

	if (Instance::ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(Instance::VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = Instance::VALIDATION_LAYERS.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
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

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

bool Device::IsDeviceSuitable(const VkPhysicalDevice &device) const
{
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SurfaceProperties swapChainSupport = QueryPhisicalDeviceSurfaceProperties(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices Device::FindQueueFamilies(const VkPhysicalDevice &physicalDevice) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	uint32_t index = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = index;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, m_instance->GetVkSurface(), &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = index;
		}

		if (indices.IsComplete())
		{
			break;
		}

		++index;
	}

	return indices;
}

// TODO -> Swap chain
SurfaceProperties Device::QueryPhisicalDeviceSurfaceProperties(const VkPhysicalDevice &physicalDevice) const
{
	SurfaceProperties details{};

	VK_CHECK_RESULT(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_instance->GetVkSurface(), &details.capabilities));

	uint32_t formatCount = 0;
	VK_CHECK_RESULT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_instance->GetVkSurface(), &formatCount, nullptr));

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice, m_instance->GetVkSurface(), &formatCount, details.formats.data()));
	}

	uint32_t presentModeCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice, m_instance->GetVkSurface(), &presentModeCount, nullptr));

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, m_instance->GetVkSurface(), &presentModeCount, details.presentModes.data()));
	}

	return details;
}

bool Device::CheckDeviceExtensionSupport(const VkPhysicalDevice &device) const
{
	uint32_t extensionCount;
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

	std::set<std::string> requiredExtensions(Instance::DEVICE_EXTENSIONS.begin(), Instance::DEVICE_EXTENSIONS.end());

	for (const auto &extension : availableExtensions)
	{
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
// MARK: Device Properties

// TOOD -> Swap chain
[[nodiscard]] QueueFamilyIndices Device::GetQueueFamilies() const
{
	if (!m_queueFamilyIndices.has_value())
	{
		m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice);
	}
	return m_queueFamilyIndices.value();
}

SurfaceProperties Device::GetSurfaceProperties() const
{
	if (!m_surfaceProperties.has_value())
	{
		m_surfaceProperties = QueryPhisicalDeviceSurfaceProperties(m_physicalDevice);
	}
	return m_surfaceProperties.value();
}

uint32_t Device::GetMaxConcurrentFrames() const
{
	// TODO: This should ba updated when the swap chain is recreated with the actual image count
	if (!m_framesInFligtCount.has_value())
	{
		const SurfaceProperties &swapChainSupport = GetSurfaceProperties();

		// Use one more than the minimum if possible (assume double buffering at least)
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		m_framesInFligtCount = imageCount;
	}

	return m_framesInFligtCount.value();
}

// ----------------------------------------------------------------------------
// MARK: Debug methods
// TODO: Should be moved to a separate file
#ifdef _DEBUG
namespace
{
	// NOLINTBEGIN modernize-avoid-c-arrays - Keep C array for fixed size for Vulkan API
	template <size_t UUID_SIZE>
	inline std::string PrintUUID(const uint8_t (&uuid)[UUID_SIZE])
	{
		std::stringstream ss;
		for (int i = 0; i < UUID_SIZE; ++i)
		{
			ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(uuid[i]);
			if (i == 3 || i == 5 || i == 7 || i == 9)
			{
				ss << "-";
			}
		}
		return ss.str();
	}
	// NOLINTEND modernize-avoid-c-arrays

} // namespace

void Device::PrintVulkanDeviceLimits() const
{
	std::stringstream ss;

	const VkPhysicalDeviceLimits &limits = m_deviceProperties.limits;
	ss << std::endl;
	ss << "Vulkan Device Properties:" << std::endl;
	ss << "--------------------------" << std::endl;
	ss << "Device Name: " << m_deviceProperties.deviceName << std::endl;
	ss << "Device Type: " << m_deviceProperties.deviceType << std::endl;
	ss << "API Version: " << m_deviceProperties.apiVersion << std::endl;
	ss << "Driver Version: " << m_deviceProperties.driverVersion << std::endl;
	ss << "Vendor ID: " << m_deviceProperties.vendorID << std::endl;
	ss << "Device ID: " << m_deviceProperties.deviceID << std::endl;
	ss << "Pipeline Cache UUID: " << PrintUUID(m_deviceProperties.pipelineCacheUUID) << std::endl;
	ss << std::endl;
	ss << "Vulkan Device Limitations:" << std::endl;
	ss << "--------------------------" << std::endl;
	ss << "Max Image Dimension 1D: " << limits.maxImageDimension1D << std::endl;
	ss << "Max Image Dimension 2D: " << limits.maxImageDimension2D << std::endl;
	ss << "Max Image Dimension 3D: " << limits.maxImageDimension3D << std::endl;
	ss << "Max Image Dimension Cube: " << limits.maxImageDimensionCube << std::endl;
	ss << "Max Image Array Layers: " << limits.maxImageArrayLayers << std::endl;
	ss << "Max Texel Buffer Elements: " << limits.maxTexelBufferElements << std::endl;
	ss << "Max Uniform Buffer Range: " << limits.maxUniformBufferRange << std::endl;
	ss << "Max Storage Buffer Range: " << limits.maxStorageBufferRange << std::endl;
	ss << "Max Push Constants Size: " << limits.maxPushConstantsSize << std::endl;
	ss << "Max Memory Allocation Count: " << limits.maxMemoryAllocationCount << std::endl;
	ss << "Max Sampler Allocation Count: " << limits.maxSamplerAllocationCount << std::endl;
	ss << "Buffer Image Granularity: " << limits.bufferImageGranularity << std::endl;
	ss << "Max Bound Descriptor Sets: " << limits.maxBoundDescriptorSets << std::endl;
	ss << "Max Per Stage Descriptor Samplers: " << limits.maxPerStageDescriptorSamplers << std::endl;
	ss << "Max Per Stage Descriptor Uniform Buffers: " << limits.maxPerStageDescriptorUniformBuffers << std::endl;
	ss << "Max Per Stage Descriptor Storage Buffers: " << limits.maxPerStageDescriptorStorageBuffers << std::endl;
	ss << "Max Per Stage Descriptor Sampled Images: " << limits.maxPerStageDescriptorSampledImages << std::endl;
	ss << "Max Per Stage Descriptor Storage Images: " << limits.maxPerStageDescriptorStorageImages << std::endl;
	ss << "Max Per Stage Descriptor Input Attachments: " << limits.maxPerStageDescriptorInputAttachments << std::endl;
	ss << "Max Per Stage Resources: " << limits.maxPerStageResources << std::endl;
	ss << "Max Descriptor Set Samplers: " << limits.maxDescriptorSetSamplers << std::endl;
	ss << "Max Descriptor Set Uniform Buffers: " << limits.maxDescriptorSetUniformBuffers << std::endl;
	ss << "Max Descriptor Set Storage Buffers: " << limits.maxDescriptorSetStorageBuffers << std::endl;
	ss << "Max Descriptor Set Uniform Buffers Dynamic: " << limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
	ss << "Max Descriptor Set Storage Buffers Dynamic: " << limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
	ss << "Max Descriptor Set Sampled Images: " << limits.maxDescriptorSetSampledImages << std::endl;
	ss << "Max Descriptor Set Storage Images: " << limits.maxDescriptorSetStorageImages << std::endl;
	ss << "Max Descriptor Set Input Attachments: " << limits.maxDescriptorSetInputAttachments << std::endl;
	ss << "Max Vertex Input Attributes: " << limits.maxVertexInputAttributes << std::endl;
	ss << "Max Vertex Input Bindings: " << limits.maxVertexInputBindings << std::endl;
	ss << "Max Vertex Input Attribute Offset: " << limits.maxVertexInputAttributeOffset << std::endl;
	ss << "Max Vertex Input Binding Stride: " << limits.maxVertexInputBindingStride << std::endl;
	ss << "Max Vertex Output Components: " << limits.maxVertexOutputComponents << std::endl;
	ss << "Max Tessellation Generation Level: " << limits.maxTessellationGenerationLevel << std::endl;
	ss << "Max Tessellation Patch Size: " << limits.maxTessellationPatchSize << std::endl;
	ss << "Max Tessellation Control Per Vertex Input Components: "
	   << limits.maxTessellationControlPerVertexInputComponents << std::endl;
	ss << "Max Tessellation Control Per Vertex Output Components: "
	   << limits.maxTessellationControlPerVertexOutputComponents << std::endl;
	ss << "Max Tessellation Control Per Patch Output Components: "
	   << limits.maxTessellationControlPerPatchOutputComponents << std::endl;
	ss << "Max Tessellation Control Total Output Components: " << limits.maxTessellationControlTotalOutputComponents
	   << std::endl;
	ss << "Max Tessellation Evaluation Input Components: " << limits.maxTessellationEvaluationInputComponents
	   << std::endl;
	ss << "Max Tessellation Evaluation Output Components: " << limits.maxTessellationEvaluationOutputComponents
	   << std::endl;
	ss << "Max Geometry Shader Invocations: " << limits.maxGeometryShaderInvocations << std::endl;
	ss << "Max Geometry Input Components: " << limits.maxGeometryInputComponents << std::endl;
	ss << "Max Geometry Output Components: " << limits.maxGeometryOutputComponents << std::endl;
	ss << "Max Geometry Output Vertices: " << limits.maxGeometryOutputVertices << std::endl;
	ss << "Max Geometry Total Output Components: " << limits.maxGeometryTotalOutputComponents << std::endl;
	ss << "Max Fragment Input Components: " << limits.maxFragmentInputComponents << std::endl;
	ss << "Max Fragment Output Attachments: " << limits.maxFragmentOutputAttachments << std::endl;
	ss << "Max Fragment Dual Src Attachments: " << limits.maxFragmentDualSrcAttachments << std::endl;
	ss << "Max Fragment Combined Output Resources: " << limits.maxFragmentCombinedOutputResources << std::endl;
	ss << "Max Compute Shared Memory Size: " << limits.maxComputeSharedMemorySize << std::endl;
	ss << "Max Compute Work Group Count: (" << limits.maxComputeWorkGroupCount[0] << ", "
	   << limits.maxComputeWorkGroupCount[1] << ", " << limits.maxComputeWorkGroupCount[2] << ")" << std::endl;
	ss << "Max Compute Work Group Invocations: " << limits.maxComputeWorkGroupInvocations << std::endl;
	ss << "Max Compute Work Group Size: (" << limits.maxComputeWorkGroupSize[0] << ", "
	   << limits.maxComputeWorkGroupSize[1] << ", " << limits.maxComputeWorkGroupSize[2] << ")" << std::endl;
	ss << "Sub-pixel Precision Bits: " << limits.subPixelPrecisionBits << std::endl;
	ss << "Sub-texel Precision Bits: " << limits.subTexelPrecisionBits << std::endl;
	ss << "Mip-map Precision Bits: " << limits.mipmapPrecisionBits << std::endl;
	ss << "Max Draw Indexed Index Value: " << limits.maxDrawIndexedIndexValue << std::endl;
	ss << "Max Draw Indirect Count: " << limits.maxDrawIndirectCount << std::endl;
	ss << "Max Sampler LOD Bias: " << limits.maxSamplerLodBias << std::endl;
	ss << "Max Sampler Anisotropy: " << limits.maxSamplerAnisotropy << std::endl;
	ss << "Max Viewports: " << limits.maxViewports << std::endl;
	ss << "Max Viewport Dimensions: (" << limits.maxViewportDimensions[0] << ", " << limits.maxViewportDimensions[1]
	   << ")" << std::endl;
	ss << "Viewport Bounds Range: (" << limits.viewportBoundsRange[0] << ", " << limits.viewportBoundsRange[1] << ")"
	   << std::endl;
	ss << "Viewport Sub-pixel Bits: " << limits.viewportSubPixelBits << std::endl;

	Log::Instance().Info(ss.str());
}
#endif
