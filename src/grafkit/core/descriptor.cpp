#include "stdafx.h"

#include <grafkit/common.h>
#include <grafkit/core/command_buffer.h>
#include <grafkit/core/descriptor.h>
#include <grafkit/core/descriptor_pool.h>
#include <grafkit/core/device.h>
#include <grafkit/core/image.h>

using namespace Grafkit::Core;

DescriptorSet::DescriptorSet(const DeviceRef &device,
	const VkDescriptorSetLayout layout,
	std::vector<VkDescriptorSet> descriptors,
	const uint32_t bindOffset)
	: m_device(device), m_layout(layout), m_descriptorSets(std::move(descriptors)), m_descriptorOffset(bindOffset)
{
}

DescriptorSet::~DescriptorSet()
{
	vkDestroyDescriptorSetLayout(**m_device, m_layout, nullptr);
}

void DescriptorSet::Bind(const Core::CommandBufferRef &commandBuffer,
	const VkPipelineLayout &pipelineLayout,
	const uint32_t frame) const noexcept
{
	assert(frame < m_descriptorSets.size());
	vkCmdBindDescriptorSets(**commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		m_descriptorOffset,
		1,
		&m_descriptorSets[frame],
		0,
		nullptr);
}

void DescriptorSet::Update(const Buffer &buffer, const uint32_t binding, const std::optional<uint32_t> frame) noexcept
{
	Update(VkDescriptorBufferInfo{buffer.buffer, 0, buffer.allocationInfo.size}, binding, frame);
}
void DescriptorSet::Update(const RingBuffer &ringBuffer, const uint32_t binding) noexcept
{

	for (uint32_t i = 0; i < ringBuffer.buffers.size(); ++i)
	{
		Update(VkDescriptorBufferInfo{ringBuffer.buffers[i].buffer, 0, ringBuffer.buffers[i].allocationInfo.size},
			binding,
			i);
	}
}
void DescriptorSet::Update(const ImagePtr &image,
	const VkSampler &sampler,
	const uint32_t binding,
	const std::optional<uint32_t> frame) noexcept
{
	const VkDescriptorImageInfo imageInfo =
		Initializers::DescriptorImageInfo(sampler, image->GetImageView(), image->GetLayout());
	Update(imageInfo, binding, frame);
}

void DescriptorSet::Update(
	const VkDescriptorBufferInfo &bufferInfo, const uint32_t binding, const std::optional<uint32_t> frame) noexcept
{
	if (frame.has_value())
	{

		VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(
			m_descriptorSets[frame.value()], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfo);
		vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
	}
	else
	{
		for (auto &mDescriptorSet : m_descriptorSets)
		{
			VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(
				mDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfo);
			vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}

void DescriptorSet::Update(
	const VkDescriptorImageInfo &imageInfo, const uint32_t binding, const std::optional<uint32_t> frame) noexcept
{
	if (frame.has_value())
	{
		VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(
			m_descriptorSets[frame.value()], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding, &imageInfo);
		vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
	}
	else
	{
		for (auto &mDescriptorSet : m_descriptorSets)
		{
			VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(
				mDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding, &imageInfo);
			vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}

const VkDescriptorSet &DescriptorSet::GetVkDescriptorSet(const uint32_t currentFrame) const noexcept
{
	return m_descriptorSets[currentFrame];
}

DescriptorBuilder::DescriptorBuilder(const DeviceRef &device) : m_device(device)
{
}

DescriptorBuilder &DescriptorBuilder::AddLayoutBindings(const Core::DescriptorSetLayoutBinding &descriptor)
{
	m_descriptorSet = descriptor.set;
	for (const auto &binding : descriptor.bindings)
	{
		m_bindings.emplace_back(
			Initializers::DescriptorSetLayoutBinding(binding.descriptorType, binding.stageFlags, binding.binding));
	}

	return *this;
}

DescriptorSetPtr DescriptorBuilder::Build()
{
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayoutCreateInfo layoutInfo = Initializers::DescriptorSetLayoutCreateInfo(m_bindings);
	if (vkCreateDescriptorSetLayout(**m_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!");
	}

	std::vector<VkDescriptorSet> descriptorSets =
		m_device->GetDescriptorPool()->AllocateDescriptorSets(descriptorSetLayout, m_device->GetMaxConcurrentFrames());

	return std::make_shared<DescriptorSet>(m_device, descriptorSetLayout, std::move(descriptorSets), m_descriptorSet);
}
