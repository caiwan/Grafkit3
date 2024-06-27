#ifndef GRAFKIT_CORE_BUFFER_H
#define GRAFKIT_CORE_BUFFER_H

#include <grafkit/common.h>
#include <vector>
#include <vk_mem_alloc.h>

namespace Grafkit {
	namespace Core {

		struct GKAPI Buffer {
			VkBuffer buffer = VK_NULL_HANDLE;
			VmaAllocation allocation = VK_NULL_HANDLE;
			VmaAllocationInfo allocationInfo {};

			void Destroy(const DeviceRef& device);

			static Buffer CreateBuffer(const DeviceRef& device,
				const size_t size,
				const VkBufferUsageFlags usage,
				const VmaMemoryUsage memoryUsage);

			void Update(const DeviceRef& device, const void* data, const size_t size);
		};

		struct GKAPI RingBuffer {
			std::vector<Buffer> buffers;
			std::vector<void*> mappedData;

			[[nodiscard]] const Buffer& GetBuffer(const uint32_t frameIndex) const { return buffers[frameIndex]; }

			void Destroy(const DeviceRef& device);

			static RingBuffer CreateBuffer(const DeviceRef& device,
				const size_t size,
				const VkBufferUsageFlags usage,
				const VmaMemoryUsage memoryUsage);

			void Update(const DeviceRef& device, const void* data, const size_t size, const uint32_t frameIndex);
		};

		template <class Type> struct GKAPI UniformBuffer {
			RingBuffer buffer {};
			Type data {};

			void Destroy(const DeviceRef& device) { buffer.Destroy(device); }

			static UniformBuffer<Type> CreateBuffer(const DeviceRef& device)
			{
				UniformBuffer<Type> uniformBuffer = {};
				uniformBuffer.buffer = RingBuffer::CreateBuffer(
					device, sizeof(Type), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
				return uniformBuffer;
			}

			void Update(const DeviceRef& device, const uint32_t frameIndex)
			{
				buffer.Update(device, reinterpret_cast<void*>(&data), sizeof(Type), frameIndex);
			}

			VkBuffer GetBuffer(const uint32_t frameIndex) const { return buffer.GetBuffer(frameIndex).buffer; }
		};

	} // namespace Core
} // namespace Grafkit

#endif // GRAFKIT_CORE_BUFFER_H
