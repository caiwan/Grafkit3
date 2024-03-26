#ifndef __GRAFKIT__API_COMMON_H__
#define __GRAFKIT__API_COMMON_H__

#if GRAFKIT_LIBRARY_TYPE == STATIC
#define GKAPI
#elif GRAFKIT_LIBRARY_TYPE == SHARED
// Define the import/export macro for Windows
#if defined(_WIN32) || defined(__WIN32__)
#ifdef MYLIBRARY_EXPORTS
#define GKAPI __declspec(dllexport)
#else
#define GKAPI __declspec(dllimport)
#endif
#else
// Define a no-op for non-Windows platforms
#define GKAPI
#endif

#endif
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

namespace Grafkit
{
	namespace Core
	{
		class Window; // GLFW Window
		using WindowPtr = std::unique_ptr<Window>;

		class Instance; // Vulkan Instance
		using InstancePtr = std::unique_ptr<Instance>;

		class Device; // PhysicalDevice + LogicalDevice
		using DevicePtr = std::unique_ptr<Device>;

		class Image; // Image
		using ImagePtr = std::shared_ptr<Image>;

		class SwapChain; // SwapChain + ImageViews
		using SwapChainPtr = std::unique_ptr<SwapChain>;

		// class RenderPass; // RenderPass
		// using RenderPassPtr = std::unique_ptr<RenderPass>;

		class Pipeline; // Pipeline + PipelineLayout (Shader)
		typedef std::shared_ptr<Pipeline> PipelinePtr;

		// class GraphicsPipeline; // RenderPass + GraphicsPipeline
		// typedef std::shared_ptr<GraphicsPipeline> GraphicsPipelinePtr;

		// class Command;		   // CommandPool + CommandBuffers
		// class Synchronization; // SyncObjects( Semaphores + Fences)
	}

}

//
#include <glm/glm.hpp>
// #include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

namespace Grafkit
{
	template <typename ReturnType, typename ClassType, typename... Args> class MemberFunctionCache
	{
	public:
		MemberFunctionCache(ClassType& inInstance, ReturnType (ClassType::*inFunc)(Args...))
			: instance(inInstance)
			, func(inFunc)
		{
		}

		ReturnType operator()(Args... inArgs)
		{
			if (!result.has_value())
			{
				result = (instance->*func)(std::forward<Args>(inArgs)...);
			}
			return result.value();
		}

		void invalidateCache() { result.reset(); }

	private:
		ClassType& instance;
		ReturnType (ClassType::*func)(Args...);
		std::optional<ReturnType> result;
	};
}

#endif // __GRAFKIT__API_COMMON_H__
