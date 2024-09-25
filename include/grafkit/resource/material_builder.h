#ifndef GRAFKIT_TEXTURE_BUILDER_H
#define GRAFKIT_TEXTURE_BUILDER_H

#include <grafkit/common.h>

#include <grafkit/core/image.h>
#include <grafkit/descriptors/material_desc.h>
#include <grafkit/interface/resource.h>
#include <grafkit/render/material.h>
#include <grafkit/render/texture.h>

namespace Grafkit::Resource {

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

		[[nodiscard]] MaterialBuilder& AddTextureImage(
			const uint32_t binding, const Core::ImagePtr& image) // TOOD: Add sampler
		{
			m_images[binding] = image;
			return *this;
		}

		[[nodiscard]] bool ResolveDependencies(const RefWrapper<ResourceManager>& resources) final;
		void Build(const Core::DeviceRef& device) final;

	private:
		Core::PipelinePtr m_pipeline;
		std::unordered_map<uint32_t, Core::DescriptorSetPtr> m_descriptorSets;
		std::unordered_map<uint32_t, Core::ImagePtr> m_images;

		[[nodiscard]] const TexturePtr CreateTexture(const Core::DeviceRef& device, const Core::ImagePtr& image) const;
	};

} // namespace Grafkit::Resource

#endif // TEXTURE_BUILDER_H
