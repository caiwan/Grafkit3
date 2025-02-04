#include "stdafx.h"

#include "grafkit/core/descriptor.h"
#include "grafkit/core/device.h"
#include "grafkit/core/image.h"
#include "grafkit/core/initializers.h"
#include "grafkit/core/pipeline.h"
#include "grafkit/core/vulkan_utils.h"
#include "grafkit/interface/resource.h"
#include "grafkit/render/material.h"
#include "grafkit/render/render_graph.h"
#include "grafkit/render/texture.h"
#include "grafkit/resource/material_builder.h"

using namespace Grafkit::Resource;
using Grafkit::Texture;
using Grafkit::TexturePtr;

bool Grafkit::Resource::MaterialBuilder::ResolveDependencies(const RefWrapper<ResourceManager> &resources)
{
	bool result = true;

	if (m_renderStage == nullptr && !m_descriptor.stage.empty())
	{
		m_renderStage = resources->Get<Grafkit::RenderStage>(m_descriptor.stage);
		result = m_renderStage != nullptr;
	}

	if (m_images.empty() && !m_descriptor.textures.empty())
	{
		for (const auto &[bindId, textureName] : m_descriptor.textures)
		{
			const auto image = resources->Get<Core::Image>(textureName);
			if (image != nullptr)
			{
				m_images.emplace(static_cast<uint32_t>(bindId), image);
			}
			else
			{
				result = false;
			}
		}
	}

	return result;
}

void MaterialBuilder::Build(const Core::DeviceRef &device)
{
	m_resource = std::make_shared<Grafkit::Material>();
	m_resource->stage = m_renderStage;

	const Core::DescriptorSetLayoutBindingMap materialBindings =
		m_renderStage->GetDescriptorSetLayoutBindings(Grafkit::StageDescriptorType::Material);
	m_resource->descriptorSets.reserve(materialBindings.size());

	std::vector<uint32_t> layoutSlots = {
		Grafkit::TEXTURE_SET,
	};

	for (const auto &layoutSlot : layoutSlots)
	{
		m_resource->descriptorSets.emplace(layoutSlot, std::move(m_renderStage->CreateDescriptorSet(layoutSlot)));
	}

	m_resource->textures.reserve(m_images.size());

	// TOOD: This will be handled automatiaclly by generated code
	const auto textureSetDescriptorIt = m_resource->descriptorSets.find(Grafkit::TEXTURE_SET);
	if (textureSetDescriptorIt == m_resource->descriptorSets.end())
	{
		throw std::runtime_error("Error: Texture descriptor set not found");
	}

	// TOOD: Add null object for missing textures

	// Mandate all texture slots to be filled
	const auto &textureSetDescriptor = textureSetDescriptorIt->second;
	const auto &textureBindingsIt = materialBindings.find(Grafkit::TEXTURE_SET);
	if (textureBindingsIt == materialBindings.end())
	{
		throw std::runtime_error(
			"Error: Texture descriptor set not found in the pipeline descriptor set layout bindings.");
	}

	for (const auto &binding : textureBindingsIt->second)
	{
		const auto image = m_images.find(binding.binding);
		if (image != m_images.end())
		{
			TexturePtr texture = CreateTexture(device, image->second);
			textureSetDescriptor->Update(texture->GetImage(), texture->GetSampler(), binding.binding);
			m_resource->textures.emplace(binding.binding, std::move(texture));
		}
		else
		{
			// TODO: Generate textures if not provided or decide to check for all textures dynamically
			std::string msg = "Error: Texture slot not filled: " + std::to_string(binding.binding);
			throw std::runtime_error(msg);
		}
	}
}

TexturePtr MaterialBuilder::CreateTexture(const Core::DeviceRef &device, const Core::ImagePtr &image) const
{
	if (image == nullptr)
	{
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

	VkSampler sampler = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateSampler(**device, &samplerInfo, nullptr, &sampler));

	return std::make_shared<Texture>(device, image, sampler);
}
