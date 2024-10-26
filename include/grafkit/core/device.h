#ifndef GRAFKIT_CORE_DEVICE_H
#define GRAFKIT_CORE_DEVICE_H

#include <grafkit/common.h>
#include <grafkit/core/descriptor_pool.h>
#include <grafkit/core/instance.h>
#include <grafkit/core/window.h>
#include <memory>
#include <optional>
#include <vector>
#include <vk_mem_alloc.h>

namespace Grafkit::Core {
	using DescriptorPoolPtr = std::unique_ptr<DescriptorPool>;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SurfaceProperties {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class GKAPI Device {
	public:
		explicit Device(const Core::InstanceRef& instance);
		~Device();

		void WaitIdle() const;
		[[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(const VkCommandBuffer& commandBuffer) const;

		[[nodiscard]] const VkDevice& operator*() const { return m_device; }

		[[nodiscard]] const VkDevice& GetVkDevice() const { return m_device; }

		// TODO: This is not quite neccessary - Only used during rendering
		[[nodiscard]] const VkPhysicalDevice& GetVkPhysicalDevice() const { return m_physicalDevice; }

		[[nodiscard]] const VkQueue& GetVkGraphicsQueue() const { return m_graphicsQueue; }

		[[nodiscard]] const VkQueue& GetVkPresentQueue() const { return m_presentQueue; }

		[[nodiscard]] const VkCommandPool& GetVkCommandPool() const { return m_commandPool; }

		[[nodiscard]] const VmaAllocator& GetVmaAllocator() const { return m_allocator; }

		[[nodiscard]] DescriptorPoolRef GetDescriptorPool() const;

		// TODO: This is not quite neccessary - Only used during initialization
		[[nodiscard]] QueueFamilyIndices GetQueueFamilies() const; // TODO -> SwapChain
		[[nodiscard]] SurfaceProperties GetSurfaceProperties() const; // TODO -> SwapChain

		[[nodiscard]] bool CheckDeviceExtensionSupport() const
		{
			// TODO: Specify which extensions are required
			return CheckDeviceExtensionSupport(m_physicalDevice);
		}

		[[nodiscard]] const VkPhysicalDeviceProperties& GetDeviceProperties() { return m_deviceProperties; }

		[[nodiscard]] const VkPhysicalDeviceLimits& GetDeviceLimits() { return m_deviceProperties.limits; }

		[[nodiscard]] uint32_t GetMaxConcurrentFrames() const;

	private:
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateGraphicsQueue();
		void CreatePresentQueue();
		void CreateCommandPool();
		void InitializeAllocator();

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& physicalDevice) const;

		[[nodiscard]] SurfaceProperties QueryPhisicalDeviceSurfaceProperties(
			const VkPhysicalDevice& physicalDevice) const; // TODO -> SwapChain

		[[nodiscard]] bool IsDeviceSuitable(const VkPhysicalDevice& m_device) const;
		[[nodiscard]] bool CheckDeviceExtensionSupport(const VkPhysicalDevice& m_device) const;

#ifdef _DEBUG
		void PrintVulkanDeviceLimits() const;
#endif

		const Core::InstanceRef m_instance;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;

		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;

		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		DescriptorPoolPtr m_descriptorPool = VK_NULL_HANDLE;

		VmaAllocator m_allocator = VK_NULL_HANDLE;

		// Cache for expensive queries
		// TOOD: Move to a separate class/struct if needed
		mutable std::optional<QueueFamilyIndices> m_queueFamilyIndices = std::nullopt;
		mutable std::optional<SurfaceProperties> m_surfaceProperties = std::nullopt;
		mutable std::optional<uint32_t> m_framesInFligtCount = std::nullopt;

		VkPhysicalDeviceProperties m_deviceProperties {};
	};

} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_DEVICE_H__
