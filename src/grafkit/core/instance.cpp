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
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// 
#include <grafkit/core/instance.h>
#include <grafkit/core/window.h>

using namespace Grafkit::Core;

// ----------------------------------------------------------------------------
VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

// ----------------------------------------------------------------------------

const std::vector<const char*> Instance::validationLayers = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> Instance::deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// ----------------------------------------------------------------------------

Grafkit::Core::Instance::Instance(const Core::Window& window, std::vector<const char*> instanceExtensions)
	: instance(CreateInstance(instanceExtensions))
	, surface(CreateSurface(window))
{
	SetupDebugMessenger();
}

Instance::Instance(const Core::Window& window)
	: instance(CreateInstance({}))
	, surface(CreateSurface(window))
{
	SetupDebugMessenger();
}

Instance::~Instance()
{
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

// ----------------------------------------------------------------------------

VkInstance Instance::CreateInstance(std::vector<const char*> instanceExtensions)
{
	if (Instance::enableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	const VkApplicationInfo appInfo(
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"GrafkitApp",
		VK_MAKE_VERSION(1, 0, 0),
		"GrafkitVK",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0);

	auto extensions = GetRequiredExtensions();
	for (auto& extension : instanceExtensions)
	{
		extensions.emplace_back(extension);
	}

	std::cout << "Required extensions:";
	for (const auto& extension : extensions)
	{
		std::cout << " " << extension;
	}
	std::cout << std::endl;

	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		const VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = CreateDebugMessengerCreateInfo();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VkInstance createdInstance;

	if (vkCreateInstance(&createInfo, nullptr, &createdInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}

	return createdInstance;
}

VkDebugUtilsMessengerCreateInfoEXT Instance::CreateDebugMessengerCreateInfo()
{
	if (Instance::enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;

		return createInfo;
	}
	else
	{
		return VkDebugUtilsMessengerCreateInfoEXT();
	}
}

VkSurfaceKHR Instance::CreateSurface(const Core::Window& window)
{
	VkSurfaceKHR vkSurface;
	glfwCreateWindowSurface(instance, window.GetWindow(), nullptr, &vkSurface);
	return vkSurface;
}

void Grafkit::Core::Instance::SetupDebugMessenger()
{
	if (Instance::enableValidationLayers)
	{

		const VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}

std::vector<const char*> Instance::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (Instance::enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool Instance::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : Instance::validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
	[[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	// TOOD: Put logger here instead of std::cerr

	return VK_FALSE;
}
