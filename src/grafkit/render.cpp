#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
//
#include <grafkit/core/device.h>
#include <grafkit/core/instance.h>
#include <grafkit/core/swap_chain.h>
#include <grafkit/render.h>

using namespace Grafkit;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

RenderContext::RenderContext(const Core::Window& window)
	: window(window)
	, instance(std::make_unique<Core::Instance>(window))
	, device(std::make_unique<Core::Device>(*instance))
	, swapChain(std::make_unique<Core::SwapChain>(window, *instance, *device))
{
	InitializeRenderPass();
	InitializeFrameBuffers();
	InitializeCommandBuffers();
}

RenderContext::~RenderContext()
{
	device->WaitIdle();
	vkFreeCommandBuffers(device->GetVkDevice(), device->GetVkCommandPool(), 1, &commandBuffer);
	vkDestroyRenderPass(device->GetVkDevice(), renderPass, nullptr);
	for (auto framebuffer : frameBuffers)
	{
		vkDestroyFramebuffer(device->GetVkDevice(), framebuffer, nullptr);
	}
	swapChain.reset();
	device.reset();
	instance.reset();
}

VkCommandBuffer RenderContext::BeginCommandBuffer()
{
	swapChain->AcquireNextFrame();
	vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

	return commandBuffer;
}

void RenderContext::BeginFrame(VkCommandBuffer& commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	const auto imageIndex = swapChain->GetCurrentImageIndex();
	const auto swapChainExtent = swapChain->GetExtent();

	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = frameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void RenderContext::DrawFrame(VkCommandBuffer& commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	swapChain->SubmitCommandBuffer(commandBuffer);
	swapChain->Present();
}

Core::GraphicsPipelineBuilder RenderContext::CreateGraphicsPipelineBuilder() const
{
	return Core::GraphicsPipelineBuilder(*device, renderPass);
}

// ----------------------------------------------------------------------------

void RenderContext::InitializeCommandBuffers()
{

	VkCommandBufferAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = device->GetVkCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device->GetVkDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void RenderContext::InitializeFrameBuffers()
{
	// Create framebuffers for the swap chain images
	const auto surfaceFormat = swapChain->ChooseSwapSurfaceFormat();
	const auto swapChainExtent = swapChain->GetExtent();

	// frameBuffers.resize(frameBuffers.size());
	const size_t frameBufferCount = swapChain->GetImageCount();

	for (size_t i = 0; i < frameBufferCount; i++)
	{
		frameBuffers.emplace_back(VkFramebuffer());
		VkImageView attachments[] = { swapChain->GetImageViews()[i] };

		VkFramebufferCreateInfo framebufferInfo {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device->GetVkDevice(), &framebufferInfo, nullptr, &frameBuffers.back()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void RenderContext::InitializeRenderPass()
{
	const auto swapChainImageFormat = swapChain->ChooseSwapSurfaceFormat().format;
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device->GetVkDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}