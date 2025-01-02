#ifndef GRAFKIT_TEXTURE_BUILDER_H
#define GRAFKIT_TEXTURE_BUILDER_H

#include <unordered_map>

#include <grafkit/common.h>
#include <grafkit/descriptors/material_desc.h>
#include <grafkit/interface/resource.h>

namespace Grafkit::Resource
{
	class MaterialBuilder : public ResourceBuilder<MaterialDesc, Grafkit::Material>
	{
	public:
		explicit MaterialBuilder(const MaterialDesc &desc = {})
			: ResourceBuilder(desc)
		{
		}

		MaterialBuilder &SetRenderStage(const RenderStagePtr &renderStage)
		{
			m_renderStage = renderStage;
			return *this;
		}

		[[nodiscard]] MaterialBuilder &AddTextureImage(const uint32_t binding,
			const Core::ImagePtr &image) // TOOD: Add sampler
		{
			m_images[binding] = image;
			return *this;
		}

		[[nodiscard]] bool ResolveDependencies(const RefWrapper<ResourceManager> &resources) final;
		void Build(const Core::DeviceRef &device) final;

	private:
		RenderStagePtr m_renderStage;

		std::unordered_map<uint32_t, Core::ImagePtr> m_images;

		[[nodiscard]] TexturePtr CreateTexture(const Core::DeviceRef &device, const Core::ImagePtr &image) const;
	};

} // namespace Grafkit::Resource

#endif // TEXTURE_BUILDER_H
