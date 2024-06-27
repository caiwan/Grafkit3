#include "stdafx.h"
#include <grafkit/core/command_buffer.h>
#include <grafkit/core/device.h>

using namespace Grafkit::Core;

Grafkit::Core::CommandBuffer::CommandBuffer(const DeviceRef& device, const uint32_t frameIndex)
	: m_device(device)
	, m_frameIndex(frameIndex)
{
	VkCommandBufferAllocateInfo allocInfo = Core::Initializers::CommandBufferAllocateInfo(
		m_device->GetVkCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	if (vkAllocateCommandBuffers(m_device->GetVkDevice(), &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

Grafkit::Core::CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(m_device->GetVkDevice(), m_device->GetVkCommandPool(), 1, &m_commandBuffer);
}

void Grafkit::Core::CommandBuffer::Reset() { vkResetCommandBuffer(m_commandBuffer, 0); }

void Grafkit::Core::CommandBuffer::End()
{
	vkCmdEndRenderPass(m_commandBuffer);
	if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
