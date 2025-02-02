#include "stdafx.h"
#include <sstream>
#include <utility>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "grafkit/core/instance.h"
#include "grafkit/core/log.h"
#include "grafkit/core/window.h"

using namespace Grafkit::Core;

// ----------------------------------------------------------------------------
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks *pAllocator)
{
	auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

// ----------------------------------------------------------------------------

const std::vector<const char *> Instance::VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> Instance::DEVICE_EXTENSIONS = { //
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
	VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
	VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME};

// ----------------------------------------------------------------------------

Grafkit::Core::Instance::Instance(const Core::WindowRef &window, const std::vector<std::string> &instanceExtensions)
	: m_instance(CreateInstance(instanceExtensions))
{
	m_surface = window->CreateSurface(m_instance);
	SetupDebugMessenger();
}

Instance::Instance(const Core::WindowRef &window)
	: m_instance(CreateInstance({}))
{
	m_surface = window->CreateSurface(m_instance);
	SetupDebugMessenger();
}

Instance::~Instance()
{
	if (!VALIDATION_LAYERS.empty())
	{
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

// ----------------------------------------------------------------------------

VkInstance Instance::CreateInstance(const std::vector<std::string> &instanceExtensions)
{
	if (Instance::ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	const VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"GrafkitApp",
		VK_MAKE_VERSION(1, 0, 0),
		"GrafkitVK",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_3};

	auto extensions = GetRequiredExtensions();
	for (const auto &extension : instanceExtensions)
	{
		extensions.emplace_back(extension.c_str());
	}

	std::stringstream logExtList;
	logExtList << "Required extensions:";
	for (const auto &extension : extensions)
	{
		logExtList << " " << extension;
	}
	Log::Instance().Info(logExtList.str());

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	void **ppNext = const_cast<void **>(&createInfo.pNext);

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = CreateDebugMessengerCreateInfo();
	if (!VALIDATION_LAYERS.empty())
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

		*ppNext = (void *)&debugCreateInfo; // NOLINT (cppcoreguidelines-pro-type-cstyle-cast) - Vulkan API
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
		*ppNext = nullptr;
	}

	VkInstance createdInstance = VK_NULL_HANDLE;

	if (vkCreateInstance(&createInfo, nullptr, &createdInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}

	return createdInstance;
}

VkDebugUtilsMessengerCreateInfoEXT Instance::CreateDebugMessengerCreateInfo()
{
	if (Instance::ENABLE_VALIDATION_LAYERS)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;

		return createInfo;
	}
	return {};
}

void Grafkit::Core::Instance::SetupDebugMessenger()
{
	if (Instance::ENABLE_VALIDATION_LAYERS)
	{

		const VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();

		if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}

std::vector<const char *> Instance::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (Instance::ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool Instance::CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : Instance::VALIDATION_LAYERS)
	{
		bool layerFound = false;

		for (const auto &layerProperties : availableLayers)
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
	[[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	[[maybe_unused]] void *pUserData)
{

	// std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	std::stringstream message;
	message << "validation layer: " << pCallbackData->pMessage;

	switch (messageSeverity)
	{
	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		Log::Instance().Debug(message.str());
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		Log::Instance().Info(message.str());
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		Log::Instance().Warning(message.str());
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		Log::Instance().Error(message.str());
		break;
	}

	return VK_FALSE;
}
