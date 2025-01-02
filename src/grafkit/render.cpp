#include "stdafx.h"

#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor_pool.h"
#include "grafkit/core/device.h"
#include "grafkit/core/image.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/instance.h"
#include "grafkit/core/render_target.h"
#include "grafkit/core/swap_chain.h"
#include "grafkit/core/vulkan_utils.h"
#include "grafkit/core/window.h"
#include "grafkit/render.h"

using namespace Grafkit;
using namespace Grafkit::Core;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// MARK: BaseRenderContext
BaseRenderContext::BaseRenderContext(const Core::WindowRef &window)
	: m_window(window)
{
	m_instance = std::make_unique<Core::Instance>(window);

	m_device = std::make_unique<Core::Device>(MakeReference(*m_instance));
	m_swapChain = std::make_unique<Core::SwapChain>(window, MakeReference(*m_instance), MakeReference((*m_device)));

	m_renderTarget = //
		RenderTargetBuilder(MakeReference(*m_device))
			.CreateAttachments(MakeReference(*m_swapChain))
			.AddAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.Build();

	InitializeCommandBuffers();
}

BaseRenderContext::~BaseRenderContext()
{
	Flush();

	m_commandBuffers.clear();

	m_renderTarget.reset();
	m_swapChain.reset();
	m_device.reset();
	m_instance.reset();
}

Core::CommandBufferRef BaseRenderContext::BeginCommandBuffer()
{
	if (!m_swapChain->AcquireNextFrame())
	{
		// TOOD: Recreate swap chain
		throw std::runtime_error("swap chain out of date!");
	}

	m_frameIndex = m_swapChain->GetCurrentFrameIndex();

	Core::CommandBufferPtr &commandBuffer = m_commandBuffers[m_frameIndex];
	commandBuffer->Reset();

	VkCommandBufferBeginInfo beginInfo = Core::Initializers::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(**commandBuffer, &beginInfo));

	return MakeReference(*commandBuffer);
}

void BaseRenderContext::EndFrame(const Core::CommandBufferRef &commandBuffer)
{
	commandBuffer->End();

	m_swapChain->SubmitCommandBuffer(**commandBuffer);
	m_swapChain->Present();
}

void BaseRenderContext::Flush()
{
	m_swapChain->WaitForFences();
	m_device->WaitIdle();
}

// ----------------------------------------------------------------------------

void BaseRenderContext::InitializeCommandBuffers()
{
	for (uint32_t frameIndex = 0; frameIndex < m_swapChain->GetImageCount(); frameIndex++)
	{
		m_commandBuffers.push_back(std::make_unique<Core::CommandBuffer>(MakeReference(*m_device), frameIndex));
	}
}

[[nodiscard]] float BaseRenderContext::GetAspectRatio() const
{
	const auto extent = m_swapChain->GetExtent();
	return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}

VkExtent2D Grafkit::BaseRenderContext::GetExtent() const
{
	return m_swapChain->GetExtent();
}

// ----------------------------------------------------------------------------
// MARK: RenderContext

Grafkit::RenderContext::RenderContext(const Core::WindowRef &window)

	: BaseRenderContext(window)
{
}
