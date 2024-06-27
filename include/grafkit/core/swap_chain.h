#ifndef GRAFKIT_CORE_IMAGE_VIEW_H
#define GRAFKIT_CORE_IMAGE_VIEW_H

#include <grafkit/common.h>

namespace Grafkit::Core {

	class Instance;
	using InstanceRef = RefWrapper<Instance>;

	class SwapChain {
	public:
		explicit SwapChain(const WindowRef& window, const InstanceRef& instance, const DeviceRef& device);
		virtual ~SwapChain(); // TODO: Shut down swap chain properly

		[[nodiscard]] uint32_t AcquireNextFrame();

		void SubmitCommandBuffer(const VkCommandBuffer& commandBuffer);
		void Present();

		void WaitForFences() noexcept;

		[[nodiscard]] VkExtent2D GetExtent() const { return m_extent; }
		[[nodiscard]] size_t GetImageCount() const { return m_images.size(); }
		[[nodiscard]] const ImagePtr& GetImage(size_t index) const { return m_images[index]; }

		[[nodiscard]] VkFormat GetFormat() const { return m_format; }

		[[nodiscard]] uint32_t GetCurrentImageIndex() const { return m_currentFrame; }
		[[nodiscard]] const VkSwapchainKHR& GetVkSwapChain() const { return m_swapChain; }

	private:
		const DeviceRef m_device;

		VkExtent2D m_extent;
		VkFormat m_format;

		VkSwapchainKHR m_swapChain;

		std::vector<ImagePtr> m_images;

		uint32_t m_currentFrame = 0;
		uint32_t m_imageIndex = 0;

		std::vector<VkSemaphore> m_presentCompleteSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;

		// ---

		[[nodiscard]] VkExtent2D ChooseSwapExtent(const WindowRef& window, const DeviceRef& device) const;

		void InitializeSwapChain(const WindowRef& window, const InstanceRef& instance, const DeviceRef& device);
		void InitializeSwapChainImages();
		void InitializeSyncObjects();
	};

} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_IMAGE_VIEW_H__
