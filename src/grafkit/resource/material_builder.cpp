#include "stdafx.h"
#include <grafkit/core/descriptor.h>
#include <grafkit/core/device.h>
#include <grafkit/core/image.h>
#include <grafkit/core/initializers.h>
#include <grafkit/core/pipeline.h>
#include <grafkit/render/material.h>
#include <grafkit/render/texture.h>
#include <grafkit/resource/material_builder.h>

using namespace Grafkit::Resource;
using Grafkit::Texture;
using Grafkit::TexturePtr;

bool Grafkit::Resource::MaterialBuilder::ResolveDependencies(const RefWrapper<ResourceManager>& resources)
{
	bool result = true;
	if (m_pipeline == nullptr && !m_descriptor.pipeline.empty()) {
		const auto pipeline = resources->Get<Core::Pipeline>(m_descriptor.pipeline);
		if (m_pipeline != nullptr) {
			m_pipeline = pipeline;
		} else {
			result = false;
		}
	}

	if (m_images.empty() && !m_descriptor.textures.empty()) {
		for (const auto& [bindId, textureName] : m_descriptor.textures) {
			const auto image = resources->Get<Core::Image>(textureName);
			if (image != nullptr) {
				m_images[static_cast<uint32_t>(bindId)] = image;
			} else {
				result = false;
			}
		}
	}

	return result;
}

void MaterialBuilder::Build(const Core::DeviceRef& device)
{
	if (m_descriptorSets.empty()) {
		throw std::runtime_error("Error: Descriptor set is null");
	}

	if (m_pipeline == nullptr) {
		throw std::runtime_error("Error: Pipeline is null");
	}

	std::map<uint32_t, TexturePtr> textures;

	assert(m_descriptorSets[Grafkit::TEXTURE_SET] != nullptr);
	for (const auto& [bindId, image] : m_images) {
		assert(image != nullptr);
		const auto texture = CreateTexture(device, image);
		m_descriptorSets[Grafkit::TEXTURE_SET]->Update(texture->GetImage(), texture->GetSampler(), bindId);
		textures[bindId] = texture;
	}
	m_resource = std::make_shared<Material>();

	m_resource->descriptorSets.resize(m_descriptorSets.size());
	for (const auto& [set, descriptorSet] : m_descriptorSets) {
		assert(set < m_descriptorSets.size());
		assert(descriptorSet != nullptr);
		m_resource->descriptorSets[set] = descriptorSet;
	}

	m_resource->textures.resize(m_images.size());
	for (const auto& [bindId, texture] : textures) {
		assert(texture != nullptr);
		m_resource->textures[bindId] = texture;
	}

	m_resource->pipeline = m_pipeline;
}

const TexturePtr MaterialBuilder::CreateTexture(const Core::DeviceRef& device, const Core::ImagePtr& image) const
{
	if (image == nullptr) {
		throw std::runtime_error("Error: Image is null");
	}

	// TOOD: Separate sampler from texture
	VkSamplerCreateInfo samplerInfo = Core::Initializers::SamplerCreateInfo();

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;

	// ---

	VkSampler sampler {};
	if (vkCreateSampler(**device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return std::make_shared<Texture>(device, image, sampler);
}
