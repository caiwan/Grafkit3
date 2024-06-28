#ifndef GRAFKIT_TEXTURE_BUILDER_H
#define GRAFKIT_TEXTURE_BUILDER_H

#include <grafkit/common.h>
#include <grafkit/render/material.h>
#include <grafkit/render/texture.h>
#include <grafkit/resource/resource.h>
#include <unordered_map>

namespace Grafkit::Resource {

	// TOOD: Merge matertial and texture

	struct MaterialDesc {
		// std::map<uint32_t, std::string> textures;
	};

	class MaterialBuilder : public ResourceBuilder<MaterialDesc, Grafkit::Material> {
	public:
		explicit MaterialBuilder(const MaterialDesc& desc = {})
			: ResourceBuilder(desc)
		{
		}

		[[nodiscard]] MaterialBuilder& SetPipeline(const Core::PipelinePtr& pipeline)
		{
			m_pipeline = pipeline;
			return *this;
		}

		[[nodiscard]] MaterialBuilder& AddDescriptorSet(const Core::DescriptorSetPtr& descriptorSet, const uint32_t set)
		{
			m_descriptorSets[set] = descriptorSet;
			return *this;
		}

		[[nodiscard]] MaterialBuilder& AddTexture(const uint32_t binding, const TexturePtr& texture)
		{
			m_textures[binding] = texture;
			return *this;
		}

		void Build(const Core::DeviceRef& device) override;

	private:
		Core::PipelinePtr m_pipeline;
		std::unordered_map<uint32_t, Core::DescriptorSetPtr> m_descriptorSets;
		std::unordered_map<uint32_t, TexturePtr> m_textures;
	};

	struct TextureDesc {
		std::string imageName;
	};

	// Texture = Image + sampler
	class TextureBuilder : public ResourceBuilder<TextureDesc, Grafkit::Texture> {
	public:
		explicit TextureBuilder(const TextureDesc& desc = {})
			: ResourceBuilder(desc)
		{
		}

		void Build(const Core::DeviceRef& device) override;

		[[nodiscard]] TextureBuilder& SetImage(const Core::ImagePtr& image)
		{
			m_image = image;
			return *this;
		}

	private:
		Core::ImagePtr m_image;
	};

} // namespace Grafkit::Resource

#endif // TEXTURE_BUILDER_H
