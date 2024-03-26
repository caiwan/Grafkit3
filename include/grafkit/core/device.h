#ifndef __GRAFKIT_CORE_DEVICE_H__
#define __GRAFKIT_CORE_DEVICE_H__

#include <memory>
#include <optional>
#include <vector>
//
#include <vk_mem_alloc.h>
//
#include <grafkit/common.h>
#include <grafkit/core/instance.h>
#include <grafkit/core/window.h>

namespace Grafkit::Core {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class GKAPI Device {
	public:
		explicit Device(const Core::Instance& instance);
		~Device();

		void WaitIdle() const;
		[[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(const VkCommandBuffer& commandBuffer) const;

		// [[nodiscard]] const Instance& GetInstance() const { return instance; }

		[[nodiscard]] const VkDevice& GetVkDevice() const { return m_device; }
		[[nodiscard]] const VkPhysicalDevice& GetVkPhysicalDevice() const { return m_physicalDevice; }
		[[nodiscard]] const VkQueue& GetVkGraphicsQueue() const { return m_graphicsQueue; }
		[[nodiscard]] const VkQueue& GetVkPresentQueue() const { return m_presentQueue; }
		[[nodiscard]] const VkCommandPool& GetVkCommandPool() const { return m_commandPool; }
		[[nodiscard]] const VmaAllocator& GetVmaAllocator() const { return m_allocator; }

		// TODO: Cache the results
		// This is not quite neccessary
		// TODO: Cache
		[[nodiscard]] QueueFamilyIndices FindQueueFamilies() const { return FindQueueFamilies(m_physicalDevice); }
		// TODO: Cache
		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport() const
		{
			return QuerySwapChainSupport(m_physicalDevice);
		}

		[[nodiscard]] bool CheckDeviceExtensionSupport() const
		{
			// TODO: Specify which extensions are required
			return CheckDeviceExtensionSupport(m_physicalDevice);
		}

	private:
		[[nodiscard]] VkPhysicalDevice PickPhysicalDevice(const VkInstance& instance) const;
		[[nodiscard]] VkDevice CreateLogicalDevice(const VkInstance& instance) const;
		[[nodiscard]] VkQueue CreateGraphicsQueue() const;
		[[nodiscard]] VkQueue CreatePresentQueue() const;
		[[nodiscard]] VkCommandPool CreateCommandPool() const;
		void InitializeAllocator();

		[[nodiscard]] QueueFamilyIndices FindQueueFamilies(
			const VkPhysicalDevice& m_device) const; // ?? Clarify whose resposibility is this?
		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(
			const VkPhysicalDevice& m_device) const; // TODO -> Instance
		[[nodiscard]] bool IsDeviceSuitable(const VkPhysicalDevice& m_device) const; // TODO -> Instance
		[[nodiscard]] bool CheckDeviceExtensionSupport(const VkPhysicalDevice& m_device) const; // TODO -> Instance

		const Core::Instance& m_instance;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		VkCommandPool m_commandPool;

		VmaAllocator m_allocator;
	};

} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_DEVICE_H__
