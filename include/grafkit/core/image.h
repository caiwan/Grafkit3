#ifndef GRAFKIT_RENDER_IMAGE_H
#define GRAFKIT_RENDER_IMAGE_H

#include <grafkit/common.h>
#include <vk_mem_alloc.h>

#include <optional>

namespace Grafkit::Core {

	class Image {
	public:
		explicit Image(const DeviceRef& device,
			const VkImage& image,
			const VkImageView& imageView,
			const VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			const std::optional<VmaAllocation> allocation = std::nullopt);

		~Image();

		[[nodiscard]] const VkImage& GetImage() const { return m_image; }
		[[nodiscard]] const VkImageView& GetImageView() const { return m_imageView; }
		[[nodiscard]] const VkImageLayout& GetLayout() const { return m_layout; }

		static ImagePtr CreateImage(const DeviceRef& device,
			const VkExtent3D size,
			const VkFormat format,
			const VkImageType type,
			const bool mipmapped = false,
			const VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
			const VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		static ImagePtr CreateImage(const DeviceRef& device,
			const void* data,
			const VkExtent3D size,
			const uint32_t channels,
			const VkFormat format,
			const VkImageType type,
			const bool mipmapped = false,
			const VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
			const VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// TODO: Update Buffer

	private:
		const DeviceRef m_device;
		std::optional<VmaAllocation> m_allocation;

		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		static void TransitionImageLayout(
			VkCommandBuffer& command, const VkImage& image, VkImageLayout currentLayout, VkImageLayout newLayout);
	};

} // namespace Grafkit::Core

#endif // GRAFKIT_RENDER_IMAGE_H
