#ifndef GRAFKIT_MATERIAL_H
#define GRAFKIT_MATERIAL_H

#include <grafkit/common.h>

namespace Grafkit {

	// TOOD: Add extra options for binds here for other paprms such as vertex params [camera], skin, etc.
	struct MaterialData {
		// AlphaMode m_alphaMode = AlphaMode::Opaque;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
	};

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

	struct GKAPI Material {
		// enum class AlphaMode : uint32_t { Opaque, Mask, Blend };

		Core::PipelinePtr pipeline;
		std::vector<Core::DescriptorSetPtr> descriptorSets;
		std::vector<TexturePtr> textures;

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
	};

} // namespace Grafkit

#endif // _GRAFKIT_MATERIAL_H
