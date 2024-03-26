#ifndef GRAFKIT_CORE_BUFFER_H
#define GRAFKIT_CORE_BUFFER_H

#include <grafkit/common.h>
#include <vk_mem_alloc.h>

namespace Grafkit {
	namespace Core {

		struct GKAPI AllocatedBuffer {
			VkBuffer buffer;
			VmaAllocation allocation;
		};

		struct GKAPI AllocatedImage {
			VkFormat format;
			VkExtent3D extent;

			VkImage image;
			VkImageView imageView;
			VmaAllocation allocation;
		};

		// TODO: Add uniform buffer support

	} // namespace Core
} // namespace Grafkit

#endif // GRAFKIT_CORE_BUFFER_H
