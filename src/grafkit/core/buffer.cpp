#include "stdafx.h"
#include <grafkit/core/buffer.h>
#include <grafkit/core/device.h>
#include <grafkit/core/initializers.h>

using namespace Grafkit::Core;

void Buffer::Destroy(const DeviceRef& device) { vmaDestroyBuffer(device->GetVmaAllocator(), buffer, allocation); }

Buffer Buffer::CreateBuffer(
	const DeviceRef& device, const size_t size, const VkBufferUsageFlags usage, const VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo = Initializers::BufferCreateInfo(usage, size);

	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = memoryUsage;

	Buffer buffer = {};

	// allocate the buffer
	if (vmaCreateBuffer(device->GetVmaAllocator(),
			&bufferInfo,
			&vmaAllocInfo,
			&buffer.buffer,
			&buffer.allocation,
			&buffer.allocationInfo)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer");
	}

	return buffer;
}

void Buffer::Update(const DeviceRef& device, const void* data, const size_t size)
{
	void* mappedData = nullptr;

	if (vmaMapMemory(device->GetVmaAllocator(), allocation, &mappedData) != VK_SUCCESS) {
		throw std::runtime_error("Failed to map memory");
	}

	memcpy(mappedData, data, size);
	vmaUnmapMemory(device->GetVmaAllocator(), allocation);

	if (vmaFlushAllocation(device->GetVmaAllocator(), allocation, 0, size) != VK_SUCCESS) {
		throw std::runtime_error("Failed to flush memory");
	}
}

void Grafkit::Core::RingBuffer::Destroy(const DeviceRef& device)
{
	for (auto& buffer : buffers) {
		vmaDestroyBuffer(device->GetVmaAllocator(), buffer.buffer, buffer.allocation);
	}
}

RingBuffer RingBuffer::CreateBuffer(
	const DeviceRef& device, const size_t size, const VkBufferUsageFlags usage, const VmaMemoryUsage memoryUsage)
{
	RingBuffer ringBuffer = {};

	for (size_t i = 0; i < device->GetMaxFramesInFlight(); i++) {
		VkBufferCreateInfo bufferInfo = Initializers::BufferCreateInfo(usage, size);

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = memoryUsage;
		vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		Buffer buffer = {};

		// allocate the buffer
		if (vmaCreateBuffer(device->GetVmaAllocator(),
				&bufferInfo,
				&vmaAllocInfo,
				&buffer.buffer,
				&buffer.allocation,
				&buffer.allocationInfo)
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer");
		}

		ringBuffer.buffers.push_back(buffer);
		ringBuffer.mappedData.push_back(buffer.allocationInfo.pMappedData);
	}

	return ringBuffer;
}

void Grafkit::Core::RingBuffer::Update(
	[[maybe_unused]] const DeviceRef& device, const void* data, const size_t size, const uint32_t frameIndex)
{
	memcpy(mappedData[frameIndex], data, size);
}
