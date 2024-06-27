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

void MaterialBuilder::Build(const Core::DeviceRef& device)
{
	if (m_descriptorSets.empty()) {
		throw std::runtime_error("Error: Descriptor set is null");
	}

	if (m_pipeline == nullptr) {
		throw std::runtime_error("Error: Pipeline is null");
	}

	// TODO: -> Find textures
	// std::unordered_map<uint32_t, TexturePtr> textures;
	// for (const auto& [bindId, textureName] : m_descriptor.textures) {
	// 	auto texture = resources->Get<Texture>(textureName);
	// 	if (texture == nullptr) {
	// 		throw std::runtime_error("Error: Texture is null");
	// 	}
	// 	m_textures[bindId] = texture;
	// }

	assert(m_descriptorSets[Grafkit::TextureSet] != nullptr);
	for (const auto& [bindId, texture] : m_textures) {
		assert(texture != nullptr);
		m_descriptorSets[Grafkit::TextureSet]->Update(texture->GetImage(), texture->GetSampler(), bindId);
	}
	m_resource = std::make_shared<Material>();

	m_resource->descriptorSets.resize(m_descriptorSets.size());
	for (const auto& [set, descriptorSet] : m_descriptorSets) {
		assert(set < m_descriptorSets.size());
		assert(descriptorSet != nullptr);
		m_resource->descriptorSets[set] = descriptorSet;
	}

	m_resource->textures.resize(m_textures.size());
	for (const auto& [bindId, texture] : m_textures) {
		assert(texture != nullptr);
		m_resource->textures[bindId] = texture;
	}

	m_resource->pipeline = m_pipeline;
}

void TextureBuilder::Build(const Core::DeviceRef& device)
{
	VkSamplerCreateInfo samplerInfo = Core::Initializers::SamplerCreateInfo();

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.magFilter = VK_FILTER_LINEAR;

	Core::ImagePtr image = m_image;

	if (image == nullptr) {
		throw std::runtime_error("Error: Image is null");
	}

	VkSampler sampler {};
	if (vkCreateSampler(**device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	m_resource = std::make_shared<Texture>(device, image, sampler);
}
