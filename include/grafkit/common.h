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
#include <tuple>
#include <vector>

#include <glm/glm.hpp>
#include <grafkit/core/initializers.h>
#include <vulkan/vulkan.h>

namespace Grafkit {
	namespace Core {

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

		class Pipeline; // Pipeline + PipelineLayout (Shader)
		typedef std::shared_ptr<Pipeline> PipelinePtr;

		typedef std::tuple<std::vector<VkVertexInputBindingDescription>, std::vector<VkVertexInputAttributeDescription>>
			VertexDescription;

	} // namespace Core

	class Actor; // Abstract 3D Object
	typedef std::shared_ptr<Actor> ActorPtr;

	class Material; // Material "Shader + Texture"
	typedef std::shared_ptr<Material> MaterialPtr;

	class Texture; // Texture
	typedef std::shared_ptr<Texture> TexturePtr;

	class Primitive; // Primitive "Meshlet"
	typedef std::shared_ptr<Primitive> PrimitivePtr;

	class Mesh; // Mesh "Model"
	typedef std::shared_ptr<Mesh> MeshPtr;

} // namespace Grafkit

//

namespace Grafkit {
	template <typename ReturnType, typename ClassType, typename... Args> class MemberFunctionCache {
	public:
		MemberFunctionCache(ClassType& inInstance, ReturnType (ClassType::*inFunc)(Args...))
			: m_instance(inInstance)
			, m_func(inFunc)
		{
		}

		ReturnType operator()(Args... inArgs)
		{
			if (!m_result.has_value()) {
				m_result = (m_instance->*m_func)(std::forward<Args>(inArgs)...);
			}
			return m_result.value();
		}

		void invalidateCache() { m_result.reset(); }

	private:
		ClassType& m_instance;
		ReturnType (ClassType::*m_func)(Args...);
		std::optional<ReturnType> m_result;
	};
} // namespace Grafkit

#endif // __GRAFKIT__API_COMMON_H__
