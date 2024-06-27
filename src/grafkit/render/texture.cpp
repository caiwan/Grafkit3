#include "stdafx.h"
#include <grafkit/core/command_buffer.h>
#include <grafkit/core/descriptor.h>
#include <grafkit/core/device.h>
#include <grafkit/core/image.h>
#include <grafkit/core/initializers.h>
#include <grafkit/core/pipeline.h>
#include <grafkit/render/texture.h>

using namespace Grafkit;

Texture::Texture(const Core::DeviceRef& device, Core::ImagePtr image, VkSampler sampler)
	: m_device(device)
	, m_image(image)
	, m_sampler(sampler)
{
	assert(m_image != nullptr);
}

Texture::~Texture() { vkDestroySampler(**m_device, m_sampler, nullptr); }

void Texture::Bind(const Core::CommandBufferRef& commandBuffer,
	const Core::PipelinePtr& pipeline,
	const Core::DescriptorSetPtr& descriptor,
	const uint32_t binding,
	const uint32_t set)
{
	vkCmdBindDescriptorSets(**commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->GetPipelineLayout(),
		set,
		1,
		&descriptor->GetVkDescriptorSet(commandBuffer->GetFrameIndex()),
		0,
		nullptr);
}
