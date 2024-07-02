#include "stdafx.h"

#include <grafkit/core/image.h>
#include <grafkit/resource/image_builder.h>

using namespace Grafkit::Resource;
using Grafkit::Core::Image;

void ImageBuilder::Build(const Core::DeviceRef& device)
{
	m_resource = Image::CreateImage(device,
		reinterpret_cast<void*>(m_descriptor.image.data()), // NOLINT vulkan API requires void* here
		{
			m_descriptor.size.x,
			m_descriptor.size.y,
			1,
		},
		m_descriptor.channels,
		VkFormat(m_descriptor.format),
		VK_IMAGE_TYPE_2D,
		m_descriptor.useMipmap,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void SolidImageBuilder::Build(const Core::DeviceRef& device)
{
	m_resource = Image::CreateImage(device,
		reinterpret_cast<void*>(&m_descriptor.color), // NOLINT vulkan API requires void* here
		{ 1, 1, 1 },
		4,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TYPE_2D,
		false,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void CheckerImageBuilder::Build(const Core::DeviceRef& device)
{
	constexpr uint32_t channels = 4;
	const auto size = m_descriptor.size.x * m_descriptor.size.y * channels;
	std::vector<uint8_t> bitmap(size);

	for (int y = 0; y < m_descriptor.size.y; ++y) {
		for (int x = 0; x < m_descriptor.size.x; ++x) {
			int index = (y * m_descriptor.size.x + x) * channels;
			if ((x / m_descriptor.divisions.x + y / m_descriptor.divisions.y) % 2 == 0) {
				// Set color1
				bitmap[index] = m_descriptor.color1.r;
				bitmap[index + 1] = m_descriptor.color1.g;
				bitmap[index + 2] = m_descriptor.color1.b;
				bitmap[index + 3] = m_descriptor.color1.a;
			} else {
				// Set color2
				bitmap[index] = m_descriptor.color2.r;
				bitmap[index + 1] = m_descriptor.color2.g;
				bitmap[index + 2] = m_descriptor.color2.b;
				bitmap[index + 3] = m_descriptor.color2.a;
			}
		}
	}

	m_resource = Image::CreateImage(device,
		bitmap.data(),
		{
			m_descriptor.size.x,
			m_descriptor.size.y,
			1,
		},
		channels,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TYPE_2D,
		m_descriptor.useMipmap,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
