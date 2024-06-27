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
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Grafkit {

	class IResource {
	public:
		virtual ~IResource() = default;
		virtual std::string Kind() const = 0;
	};

#define GRAFKIT_RESOURCE_KIND(kind)                  \
	static constexpr std::string_view KIND = (kind); \
	[[nodiscard]] std::string Kind() const override { return (kind); }

	template <typename T> class RefWrapper {
	public:
		explicit RefWrapper(T& ref)
			: ptr(&ref)
		{
		}

		RefWrapper() = default;

		[[nodiscard]] T& operator*() const noexcept
		{
			assert(ptr);
			return *ptr;
		}

		[[nodiscard]] T* operator->() const noexcept
		{
			assert(ptr);
			return ptr;
		}

		[[nodiscard]] T* Get() const noexcept
		{
			assert(ptr);
			return ptr;
		}

		template <typename U> [[nodiscard]] RefWrapper<U> Cast()
		{
			U* p = dynamic_cast<U*>(ptr);
			assert(p);
			return p;
		}

	private:
		T* ptr = nullptr;
	};

	// Factory function to simplify the creation of a RefWrapper
	template <typename T> RefWrapper<T> MakeReference(T& ref) { return RefWrapper<T>(ref); }

	// TOOD: This has to be memory aligned
	struct TimeInfo {
		float time = 0.0f;
		float deltaTime = 0.0f;
	};

	// Forward declarations
	namespace Core {
		class Window; // GLFW Window
		using WindowRef = RefWrapper<Window>;

		class Instance; // Vulkan Instance
		using InstanceRef = RefWrapper<Instance>;

		class Device; // PhysicalDevice + LogicalDevice
		using DeviceRef = RefWrapper<Device>;

		class CommandBuffer; // CommandBuffer
		using CommandBufferRef = RefWrapper<CommandBuffer>;

		class SwapChain;
		using SwapChainRef = RefWrapper<SwapChain>;

		class FrameBuffer;
		using FrameBufferRef = RefWrapper<FrameBuffer>;

		class DescriptorPool;
		using DescriptorPoolRef = RefWrapper<DescriptorPool>;

		class Image;
		typedef std::shared_ptr<Image> ImagePtr;

		class Pipeline; // Pipeline + PipelineLayout (Shader)
		typedef std::shared_ptr<Pipeline> PipelinePtr;

		struct VertexDescription {
			std::vector<VkVertexInputBindingDescription> bindings;
			std::vector<VkVertexInputAttributeDescription> attributes;
		};

		struct DescriptorBinding {
			uint32_t binding;
			VkDescriptorType descriptorType;
			VkShaderStageFlags stageFlags;
		};

		struct SetDescriptor {
			uint32_t set;
			std::vector<DescriptorBinding> bindings;
		};

		class DescriptorSet; // DescriptorSetLayout + DescriptorSet
		typedef std::shared_ptr<DescriptorSet> DescriptorSetPtr;

		constexpr size_t MAX_PUSH_CONSTANT_SIZE = 128;

	} // namespace Core

	namespace Resource {
		class ResoureManger;
		using ResoureMangerRef = RefWrapper<ResoureManger>;

		class IResourceBuilder;
		template <typename ParamT, typename ResourceT> class ResourceBuilder;

	} // namespace Resource

	// namespace Render {

	struct Material; // Material "Shader + Texture + UBOs"
	typedef std::shared_ptr<Material> MaterialPtr;

	class Texture; // Texture
	typedef std::shared_ptr<Texture> TexturePtr;

	// struct Primitive; // Primitive "Meshlet"
	// typedef std::shared_ptr<Primitive> PrimitivePtr;

	class Mesh; // Mesh "Model"
	typedef std::shared_ptr<Mesh> MeshPtr;

	class Scenegraph;
	typedef std::shared_ptr<Scenegraph> ScenegraphPtr;

	// } // namespace Render
	namespace Animation {
		struct Channel;
		typedef std::shared_ptr<Channel> ChannelPtr;

		class Target;
		typedef std::shared_ptr<Target> TargetPtr;

		struct Animation;
		typedef std::shared_ptr<Animation> AnimationPtr;

	} // namespace Animation

} // namespace Grafkit

#endif // __GRAFKIT__API_COMMON_H__
