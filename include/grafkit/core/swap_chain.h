#ifndef GRAFKIT_CORE_IMAGE_VIEW_H
#define GRAFKIT_CORE_IMAGE_VIEW_H

#include <grafkit/common.h>
#include <optional>

namespace Grafkit::Core
{

	class Instance;
	using InstanceRef = RefWrapper<Instance>;

	class SwapChain
	{
	public:
		explicit SwapChain(const WindowRef &window, const InstanceRef &instance, const DeviceRef &device);
		virtual ~SwapChain();

		[[nodiscard]] bool AcquireNextFrame();

		void SubmitCommandBuffer(const VkCommandBuffer &commandBuffer);
		void Present();

		void WaitForFences() noexcept;

		// Getters
		[[nodiscard]] inline uint32_t GetCurrentFrameIndex() const
		{
			return m_currentFrame;
		}

		[[nodiscard]] inline VkExtent2D GetExtent() const
		{
			return m_extent;
		}

		[[nodiscard]] inline size_t GetImageCount() const
		{
			return m_images.size();
		}

		[[nodiscard]] inline const ImagePtr &GetImage(size_t index) const
		{
			return m_images[index];
		}

		[[nodiscard]] inline VkFormat GetFormat() const
		{
			return m_format;
		}

		[[nodiscard]] const VkSwapchainKHR &GetVkSwapChain() const
		{
			return m_swapChain;
		}

	private:
		const DeviceRef m_device;

		VkExtent2D m_extent = {0, 0};
		VkFormat m_format = VK_FORMAT_UNDEFINED;

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

		std::vector<ImagePtr> m_images;

		uint32_t m_imageIndex = 0;
		uint32_t m_currentFrame = 0;
		uint32_t m_framesInFlight = 0;

		std::vector<VkSemaphore> m_presentCompleteSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;

		// ---
		void InitializeSwapChain(const WindowRef &window, const InstanceRef &instance, const DeviceRef &device);
		void InitializeSwapChainImages();
		void InitializeSyncObjects();

		[[nodiscard]] VkExtent2D ChooseSwapExtent(const WindowRef &window, const DeviceRef &device) const;
		[[nodiscard]] VkSurfaceFormatKHR ChooseSwapSurfaceFormat() const;
		[[nodiscard]] VkPresentModeKHR ChooseSwapPresentMode() const;
		[[nodiscard]] VkFormat ChooseDepthFormat() const;
	};

} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_IMAGE_VIEW_H__
