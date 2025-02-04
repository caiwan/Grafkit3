#ifndef GRAFKIT_RENDER_POSTEFFECT_H
#define GRAFKIT_RENDER_POSTEFFECT_H

#include <grafkit/common.h>

namespace Grafkit
{
	constexpr uint32_t MAP_SET = 0;

	constexpr uint32_t ALBEDO_MAP_BINDING = 0;
	constexpr uint32_t NORMAL_MAP_BINDING = 1;
	constexpr uint32_t MATERIAL_MAP_BINDING = 2;
	constexpr uint32_t EMISSIVE_MAP_BINDING = 3;
	constexpr uint32_t POSITION_MAP_BINDING = 4;
	constexpr uint32_t DEPTH_MAP_BINDING = 5;

	constexpr uint32_t POSTEFFECT_SET = 1;

	constexpr uint32_t POSTFX_PARAM_BINDING = 0;

	struct GKAPI PostEffect
	{
		static std::vector<Core::DescriptorSetLayoutBinding> GetLayoutBindings()
		{
			return {
				{
					MAP_SET,
					{
						{
							ALBEDO_MAP_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							NORMAL_MAP_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							MATERIAL_MAP_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							EMISSIVE_MAP_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							POSITION_MAP_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
						{
							DEPTH_MAP_BINDING,
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							VK_SHADER_STAGE_FRAGMENT_BIT,
						},
					},
				},
				{
					POSTEFFECT_SET,
					{
						POSTFX_PARAM_BINDING,
						VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						VK_SHADER_STAGE_VERTEX_BIT,
					},
				},
			};
		} // namespace Grafkit
	};

	struct GKAPI Blur : public PostEffect
	{
		struct BlurData
		{
			float blurSize = 1.0f;
		};

		BlurData data;
	};

} // namespace Grafkit

#endif // GRAFKIT_RENDER_POSTEFFECT_H
