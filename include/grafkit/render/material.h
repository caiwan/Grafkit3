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

	constexpr uint32_t TextureSet = 0;
	constexpr uint32_t CameraViewSet = 1;
	constexpr uint32_t ModelViewSet = 2;

	constexpr uint32_t DiffuseTextureBinding = 0;
	constexpr uint32_t NormalTextureBinding = 1;
	constexpr uint32_t RoughnessTextureBinding = 2;
	constexpr uint32_t MetallicTextureBinding = 3;
	constexpr uint32_t AmbientOcclusionTextureBinding = 4;
	constexpr uint32_t EmissiveTextureBinding = 5;

	constexpr uint32_t ModelViewBinding = 0;

	struct GKAPI Material : public IResource {
		// enum class AlphaMode : uint32_t { Opaque, Mask, Blend };

		Core::PipelinePtr pipeline;
		std::vector<Core::DescriptorSetPtr> descriptorSets;
		std::vector<TexturePtr> textures;

		GRAFKIT_RESOURCE_KIND("Material")

		static std::vector<Core::SetDescriptor> GetLayoutBindings()
		{
			return {
				{
					TextureSet,
					{
						{
							DiffuseTextureBinding,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							NormalTextureBinding,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							RoughnessTextureBinding,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							MetallicTextureBinding,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							AmbientOcclusionTextureBinding,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							EmissiveTextureBinding,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
					},
				},
				{
					CameraViewSet,
					{
						{
							ModelViewBinding,
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
