#ifndef GRAFKIT_MATERIAL_H
#define GRAFKIT_MATERIAL_H

#include <grafkit/common.h>

#include <unordered_map>

namespace Grafkit
{
	constexpr uint32_t TEXTURE_SET = 0;
	constexpr uint32_t CAMERA_VIEW_SET = 1;
	constexpr uint32_t MODEL_VIEW_SET = 2;

	constexpr uint32_t DIFFUSE_TEXTURE_BINDING = 0;
	constexpr uint32_t NORMAL_TEXTURE_BINDING = 1;
	constexpr uint32_t ROUGHNESS_TEXTURE_BINDING = 2;
	constexpr uint32_t METALLIC_TEXTURE_BINDING = 3;
	constexpr uint32_t AMBIENT_OCCLUSION_TEXTURE_BINDING = 4;
	constexpr uint32_t EMISSIVE_TEXTURE_BINDING = 5;

	constexpr uint32_t MODEL_VIEW_BINDING = 0;

	// MARK: Material
	// This is not quite a material, but a collection of textures and descriptor sets for the entire rendering stage
	// Grtaphics stage?
	struct GKAPI Material
	{
		std::string name;

		RenderStagePtr stage;

		struct MaterialData
		{
			float alphaCutoff = 1.0f;
			float metallicFactor = 1.0f;
		};

		std::unordered_map<uint32_t, Core::DescriptorSetPtr> descriptorSets;
		std::unordered_map<uint32_t, TexturePtr> textures;

		MaterialData data;

		static std::vector<Core::DescriptorSetLayoutBinding> GetLayoutBindings()
		{
			return {
				{
					TEXTURE_SET,
					{
						{
							DIFFUSE_TEXTURE_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							NORMAL_TEXTURE_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							ROUGHNESS_TEXTURE_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							METALLIC_TEXTURE_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							AMBIENT_OCCLUSION_TEXTURE_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							EMISSIVE_TEXTURE_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
					},
				},
				{
					CAMERA_VIEW_SET,
					{
						{
							MODEL_VIEW_BINDING,
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
							VK_SHADER_STAGE_VERTEX_BIT,
						},
					},
				},
			};
		}

		// TODO: Add update fns
	};

} // namespace Grafkit

#endif // _GRAFKIT_MATERIAL_H
