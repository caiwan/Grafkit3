#ifndef __GRAFKIT_CORE_INSTANCE_H__
#define __GRAFKIT_CORE_INSTANCE_H__

#include <grafkit/common.h>
#include <vector>

namespace Grafkit::Core
{
	class GKAPI Instance
	{
	public:
#ifdef NDEBUG
		static constexpr bool enableValidationLayers = false;
#else
		static constexpr bool enableValidationLayers = true;
#endif

		static const std::vector<const char*> validationLayers;
		static const std::vector<const char*> deviceExtensions;

		// --- Constructors

		explicit Instance(const Core::Window& window);
		explicit Instance(const Core::Window& window, std::vector<const char*> instanceExtensions);
		~Instance();

		[[nodiscard]] const VkInstance& GetVkInstance() const { return instance; }
		[[nodiscard]] const VkSurfaceKHR& GetVkSurface() const { return surface; }


	private:
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;

		VkInstance CreateInstance(std::vector<const char*> instanceExtensions);
		bool CheckValidationLayerSupport();
		[[nodiscard]] VkSurfaceKHR CreateSurface(const Core::Window& window);

		// ---
		VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();
		void SetupDebugMessenger();
		std::vector<const char*> GetRequiredExtensions();

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};
}

#endif // __GRAFKIT_CORE_INSTANCE_H__
