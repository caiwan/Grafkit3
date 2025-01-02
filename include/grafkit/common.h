#ifndef __GRAFKIT__API_COMMON_H__
#define __GRAFKIT__API_COMMON_H__

#define VMA_VULKAN_VERSION 1003000

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

// Clang workaround for vma_alloc
#ifdef __clang__
#define VMA_NOT_NULL
#define VMA_NULLABLE
#endif // __clang__

#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Grafkit
{
	// MARK: Reference Wrapper
	template <typename T>
	class RefWrapper
	{
	public:
		explicit RefWrapper(T &ref)
			: m_ptr(&ref)
		{
		}

		explicit RefWrapper(T *ref)
			: m_ptr(ref)
		{
		}

		RefWrapper() = default;

		[[nodiscard]] T &operator*() const noexcept
		{
			assert(m_ptr);
			return *m_ptr;
		}

		[[nodiscard]] T *operator->() const noexcept
		{
			assert(m_ptr);
			return m_ptr;
		}

		[[nodiscard]] T *Get() const noexcept
		{
			assert(m_ptr);
			return m_ptr;
		}

		template <typename U>
		[[nodiscard]] RefWrapper<U> Cast()
		{
			U *ptr = dynamic_cast<U *>(m_ptr);
			assert(ptr);
			return ptr;
		}

	private:
		T *m_ptr = nullptr;
	};

	// Factory function to simplify the creation of a RefWrapper
	template <typename T>
	RefWrapper<T> MakeReference(T &ref)
	{
		return RefWrapper<T>(ref);
	}
	template <typename S, typename T>
	RefWrapper<S> MakeReferenceAs(T &ref)
	{
		S *ptr = dynamic_cast<S *>(&ref);
		assert(ptr);
		return RefWrapper<S>(ptr);
	}

	// MARK: Common Structures
	// TOOD: This has to be memory aligned
	struct TimeInfo
	{
		float time = 0.0f;
		float deltaTime = 0.0f;
	};
} // namespace Grafkit

// MARK: Forward declarations
namespace Grafkit::Core
{
	class IWindow; // GLFW Window
	using WindowRef = RefWrapper<IWindow>;

	class Instance; // Vulkan Instance
	using InstanceRef = RefWrapper<Instance>;

	class Device; // PhysicalDevice + LogicalDevice
	using DeviceRef = RefWrapper<Device>;

	class CommandBuffer; // CommandBuffer
	using CommandBufferRef = RefWrapper<CommandBuffer>;

	class SwapChain;
	using SwapChainRef = RefWrapper<SwapChain>;

	class RenderTarget; // Framrbuffer + renderpass
	using RenderTargetPtr = std::shared_ptr<RenderTarget>;

	class DescriptorPool;
	using DescriptorPoolRef = RefWrapper<DescriptorPool>;

	class Image;
	using ImagePtr = std::shared_ptr<Image>;

	class Pipeline; // Pipeline + PipelineLayout (Shader)
	using PipelinePtr = std::shared_ptr<Pipeline>;

	struct VertexDescription
	{
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
	};

	struct DescriptorBinding
	{
		uint32_t binding;
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
	};

	struct DescriptorSetLayoutBinding
	{
		uint32_t set;
		std::vector<DescriptorBinding> bindings;
	};

	using DescriptorSetLayoutBindings = std::vector<DescriptorSetLayoutBinding>;
	using DescriptorSetLayoutBindingMap = std::unordered_map<uint32_t, std::vector<DescriptorBinding>>;

	class DescriptorSet; // DescriptorSetLayout + DescriptorSet
	using DescriptorSetPtr = std::shared_ptr<DescriptorSet>;

	constexpr size_t MAX_PUSH_CONSTANT_SIZE = 128;

} // namespace Grafkit::Core

namespace Grafkit
{
	class RenderStage; // Pipeline + RenderTarget + DescriptorSets
	using RenderStagePtr = std::shared_ptr<RenderStage>;

	class RenderGraph; // List of RenderStages
	using RenderGraphPtr = std::shared_ptr<RenderGraph>;

	struct Material; // Material "Shader + Texture + UBOs"
	using MaterialPtr = std::shared_ptr<Material>;

	class Texture; // Texture
	using TexturePtr = std::shared_ptr<Texture>;

	class Mesh; // Mesh "Model"
	using MeshPtr = std::shared_ptr<Mesh>;

	class Scenegraph;
	using ScenegraphPtr = std::shared_ptr<Scenegraph>;
} // namespace Grafkit

namespace Grafkit::Resource
{
	class ResoureManger;
	using ResoureMangerRef = RefWrapper<ResoureManger>;

	class IResourceLoader;
	template <typename ParamT, typename ResourceT>
	class ResourceBuilder;

} // namespace Grafkit::Resource

namespace Grafkit::Animation
{
	struct Channel;
	using ChannelPtr = std::shared_ptr<Channel>;

	class Target;
	using TargetPtr = std::shared_ptr<Target>;

	struct Animation;
	using AnimationPtr = std::shared_ptr<Animation>;

} // namespace Grafkit::Animation

#endif // __GRAFKIT__API_COMMON_H__
