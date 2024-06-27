#ifndef GRAFKIT_CORE_INSTANCE_H
#define GRAFKIT_CORE_INSTANCE_H

#include <grafkit/common.h>
#include <vector>

namespace Grafkit::Core {
	class GKAPI Instance {
	public:
#ifdef NDEBUG
		static constexpr bool enableValidationLayers = false;
#else
		static constexpr bool enableValidationLayers = true;
#endif

		static const std::vector<const char*> validationLayers;
		static const std::vector<const char*> deviceExtensions;

		explicit Instance(const Core::WindowRef& window);
		explicit Instance(const Core::WindowRef& window, std::vector<std::string> instanceExtensions);
		~Instance();

		[[nodiscard]] const VkInstance& GetVkInstance() const { return m_instance; }
		[[nodiscard]] const VkSurfaceKHR& GetVkSurface() const { return m_surface; }

	private:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;

		VkInstance CreateInstance(std::vector<std::string> instanceExtensions);

		bool CheckValidationLayerSupport();
		[[nodiscard]] VkSurfaceKHR CreateSurface(const Core::WindowRef& window);

		// ---
		VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();
		void SetupDebugMessenger();
		std::vector<const char*> GetRequiredExtensions();

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};
} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_INSTANCE_H__
