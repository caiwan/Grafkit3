#include "stdafx.h"

#include <grafkit/core/command_buffer.h>
#include <grafkit/core/descriptor_pool.h>
#include <grafkit/core/device.h>
#include <grafkit/core/framebuffer.h>
#include <grafkit/core/image.h>
#include <grafkit/core/initializers.h>
#include <grafkit/core/instance.h>
#include <grafkit/core/swap_chain.h>
#include <grafkit/core/window.h>
#include <grafkit/render.h>

using namespace Grafkit;
using namespace Grafkit::Core;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

RenderContext::RenderContext(const Core::WindowRef& window)
	: window(window)
{
	m_instance = std::make_unique<Core::Instance>(window);

	m_device = std::make_unique<Core::Device>(MakeReference(*m_instance));
	m_swapChain = std::make_unique<Core::SwapChain>(window, MakeReference(*m_instance), MakeReference((*m_device)));

	m_frameBuffer = std::make_unique<Core::FrameBuffer>(
		MakeReference(*m_device), MakeReference(*m_swapChain), std::vector<FrameBufferAttachmentInfo>({}));

	InitializeCommandBuffers();
	SetupViewport();
}

RenderContext::~RenderContext()
{
	Flush();

	m_commandBuffers.clear();

	m_frameBuffer.reset();
	m_swapChain.reset();
	m_device.reset();
	m_instance.reset();
}

const Core::DeviceRef RenderContext::GetDevice() const { return MakeReference(*m_device); }

Core::CommandBufferRef RenderContext::BeginCommandBuffer()
{
	m_currentImageIndex = m_swapChain->GetCurrentImageIndex();
	m_nextFrameIndex = m_swapChain->AcquireNextFrame();

	m_commandBuffers[m_currentImageIndex]->Reset();
	return MakeReference(*m_commandBuffers[m_currentImageIndex]);
}

void RenderContext::BeginFrame(const Core::CommandBufferRef& commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo = Core::Initializers::CommandBufferBeginInfo();

	if (vkBeginCommandBuffer(**commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	const auto swapChainExtent = m_swapChain->GetExtent();

	// TOOD: Move to FrameBuffer + Use std::span
	VkClearValue clearValues[2] = {
		{ 0.0f, 0.0f, 0.2f, 1.0f },
		{ 1.0f, 0 },
	};
	// clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
	// clearValues[1].depthStencil = { 1.0f, 0 };

	m_frameBuffer->BeginRenderPass(commandBuffer, m_currentImageIndex, clearValues);

	vkCmdSetViewport(**commandBuffer, 0, 1, &m_viewport);
	vkCmdSetScissor(**commandBuffer, 0, 1, &m_scissor);
}

void RenderContext::EndFrame(const Core::CommandBufferRef& commandBuffer)
{
	commandBuffer->End();

	m_swapChain->SubmitCommandBuffer(**commandBuffer);
	m_swapChain->Present();
}

void RenderContext::Flush()
{
	m_swapChain->WaitForFences();
	m_device->WaitIdle();
}

// ----------------------------------------------------------------------------

DescriptorBuilder RenderContext::DescriptorBuilder() const { return Core::DescriptorBuilder(MakeReference(*m_device)); }

GraphicsPipelineBuilder RenderContext::PipelineBuilder() const
{
	return GraphicsPipelineBuilder(MakeReference(*m_device), MakeReference(*m_frameBuffer));
}

// ----------------------------------------------------------------------------

void RenderContext::InitializeCommandBuffers()
{
	for (uint32_t frameIndex = 0; frameIndex < m_swapChain->GetImageCount(); frameIndex++) {
		m_commandBuffers.push_back(std::make_unique<Core::CommandBuffer>(MakeReference(*m_device), frameIndex));
	}
}

void RenderContext::SetupViewport()
{
	const auto& swapChainExtent = m_swapChain->GetExtent();
	m_viewport.x = 0.0f;
	m_viewport.y = 0.0f;
	m_viewport.width = static_cast<float>(m_swapChain->GetExtent().width);
	m_viewport.height = static_cast<float>(m_swapChain->GetExtent().height);
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	m_scissor.offset = { 0, 0 };
	m_scissor.extent = swapChainExtent;
}

[[nodiscard]] float RenderContext::GetAspectRatio() const
{
	const auto extent = m_swapChain->GetExtent();
	return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}
