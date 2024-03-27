#ifndef __GRAFKIT_CORE_IMAGE_VIEW_H__
#define __GRAFKIT_CORE_IMAGE_VIEW_H__

#include <grafkit/common.h>
#include <grafkit/core/device.h>
#include <grafkit/core/instance.h>

namespace Grafkit::Core {
	class SwapChain {

	public:
		explicit SwapChain(const Window& window, const Instance& instance, const Device& device);
		virtual ~SwapChain(); // TODO: Shut down swap chain properly

		[[nodiscard]] uint32_t AcquireNextFrame();

		void SubmitCommandBuffer(const VkCommandBuffer& commandBuffer);
		void Present();

		void WaitForFences();

		[[nodiscard]] const VkExtent2D& GetExtent() const { return extent; }

		[[nodiscard]] std::size_t GetWidth() const { return extent.width; }

		[[nodiscard]] std::size_t GetHeight() const { return extent.height; }

		[[nodiscard]] const size_t GetImageCount() const { return images.size(); }

		[[nodiscard]] const size_t GetCurrentFrameIndex() const { return currentFrame; }

		[[nodiscard]] const VkSwapchainKHR& GetVkSwapChain() const { return swapChain; }

		[[nodiscard]] const std::vector<VkImage>& GetImages() const { return images; }

		[[nodiscard]] const std::vector<VkImageView>& GetImageViews() const { return imageViews; }

		[[nodiscard]] const VkImageView& GetVkImageView() const { return imageViews[currentFrame]; }

		[[nodiscard]] const VkSurfaceFormatKHR ChooseSwapSurfaceFormat() const
		{
			return ChooseSwapSurfaceFormat(device);
		}

		[[nodiscard]] const VkPresentModeKHR ChooseSwapPresentMode() const { return ChooseSwapPresentMode(device); }

		[[nodiscard]] const uint32_t FindImageCount() const { return FindImageCount(device); }

	private:
		const Device& device;

		VkExtent2D extent;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		uint32_t currentFrame = 0;
		uint32_t imageIndex = 0;

		std::vector<VkSemaphore> presentCompleteSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		// ---

		[[nodiscard]] VkExtent2D ChooseSwapExtent(const Window& window, const Device& device) const;
		[[nodiscard]] VkSwapchainKHR CreateSwapChain(
			const Window& window, const Instance& instance, const Device& device) const;

		void InitializeSwapChainImages();
		void InitializeImageViews();
		void InitializeSyncObjects();

		[[nodiscard]] VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const Device& device) const;
		[[nodiscard]] VkPresentModeKHR ChooseSwapPresentMode(const Device& device) const;
		[[nodiscard]] uint32_t FindImageCount(const Device& device) const;
	};

} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_IMAGE_VIEW_H__
