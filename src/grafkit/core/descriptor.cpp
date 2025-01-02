#include "stdafx.h"

#include "grafkit/common.h"
#include "grafkit/core/command_buffer.h"
#include "grafkit/core/descriptor.h"
#include "grafkit/core/descriptor_pool.h"
#include "grafkit/core/device.h"
#include "grafkit/core/image.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/vulkan_utils.h"

using namespace Grafkit::Core;

DescriptorSet::DescriptorSet(const DeviceRef &device,
	const VkDescriptorSetLayout layout,
	std::vector<VkDescriptorSet> descriptors,
	const uint32_t bindOffset)
	: m_device(device)
	, m_layout(layout)
	, m_descriptorSets(std::move(descriptors))
	, m_descriptorOffset(bindOffset)
{
}

DescriptorSet::~DescriptorSet()
{
	for (auto &descriptorSet : m_descriptorSets)
	{
		Log::Instance().Trace("Deallocating descriptor. Object=%p", descriptorSet);
		m_device->GetDescriptorPool()->DeallocateDescriptorSet(descriptorSet);
	}
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
	const VkDescriptorBufferInfo bufferInfo{
		buffer.buffer,
		0,
		buffer.allocationInfo.size,
	};
	Update(bufferInfo, binding, frame);
}
void DescriptorSet::Update(const RingBuffer &ringBuffer, const uint32_t binding) noexcept
{

	for (uint32_t i = 0; i < ringBuffer.buffers.size(); ++i)
	{
		const VkDescriptorBufferInfo bufferInfo{
			ringBuffer.buffers[i].buffer,
			0,
			ringBuffer.buffers[i].allocationInfo.size,
		};

		Update(bufferInfo, binding, i);
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

void DescriptorSet::Update(const VkDescriptorBufferInfo &bufferInfo,
	const uint32_t binding,
	const std::optional<uint32_t> frame) noexcept
{
	if (frame.has_value())
	{
		auto &descriptorSet = m_descriptorSets[frame.value()];

		VkWriteDescriptorSet descriptorWrite =
			Initializers::WriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfo);

		Log::Instance().Trace(
			"Updating descriptor set for buffer; Binding=%d Object=%p Frame=%d Buffer=%p Offset=%d Range=%d",
			binding,
			descriptorSet,
			frame.value(),
			bufferInfo.buffer,
			bufferInfo.offset,
			bufferInfo.range);

		vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
	}
	else
	{
		for (auto &descriptorSet : m_descriptorSets)
		{
			VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(descriptorSet,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				binding,
				&bufferInfo);

			Log::Instance().Trace(
				"Updating descriptor set for buffer; Binding=%d Object=%p Buffer=%p Offset=%d Range=%d",
				binding,
				descriptorSet,
				bufferInfo.buffer,
				bufferInfo.offset,
				bufferInfo.range);

			vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}

void DescriptorSet::Update(const VkDescriptorImageInfo &imageInfo,
	const uint32_t binding,
	const std::optional<uint32_t> frame) noexcept
{
	if (frame.has_value())
	{
		auto &descriptorSet = m_descriptorSets[frame.value()];
		VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			binding,
			&imageInfo);

		Log::Instance().Trace(
			"Updating descriptor set for image; Binding=%d Object=%p Frame=%d Sampler=%p Image=%p Layout=%d",
			binding,
			descriptorSet,
			frame.value(),
			imageInfo.sampler,
			imageInfo.imageView,
			imageInfo.imageLayout);

		vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
	}
	else
	{
		for (auto &descriptorSet : m_descriptorSets)
		{
			VkWriteDescriptorSet descriptorWrite = Initializers::WriteDescriptorSet(descriptorSet,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				binding,
				&imageInfo);

			Log::Instance().Trace(
				"Updating descriptor set for image; Binding=%d Object=%p Sampler=%p Image=%p Layout=%d",
				binding,
				descriptorSet,
				imageInfo.sampler,
				imageInfo.imageView,
				imageInfo.imageLayout);

			vkUpdateDescriptorSets(**m_device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}

DescriptorSetPtr DescriptorSet::Create(const DeviceRef &device,
	const VkDescriptorSetLayout descriptorSetLayout,
	const uint32_t set,
	const std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
	std::vector<VkDescriptorSet> descriptorSets =
		device->GetDescriptorPool()->AllocateDescriptorSets(descriptorSetLayout, device->GetMaxConcurrentFrames());

	return std::make_shared<DescriptorSet>(device, descriptorSetLayout, std::move(descriptorSets), set);
}

DescriptorSetPtr DescriptorSet::Create(const DeviceRef &device,
	const VkDescriptorSetLayout descriptorSetLayout,
	const uint32_t set,
	const std::vector<Core::DescriptorBinding> &bindings)
{
	// TOOD: This should usde a different descriptorset layout binding than the one from the pipeline
	std::vector<VkDescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(bindings.size());
	std::transform(bindings.begin(),
		bindings.end(),
		std::back_inserter(vkBindings),
		[](const Core::DescriptorBinding &binding)
		{
			return Core::Initializers::DescriptorSetLayoutBinding(binding.descriptorType,
				binding.stageFlags,
				binding.binding);
		});
	return Create(device, descriptorSetLayout, set, vkBindings);
}
