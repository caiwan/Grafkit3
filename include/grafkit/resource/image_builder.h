#ifndef GRAFKIT_IMAGE_BUILDER_H
#define GRAFKIT_IMAGE_BUILDER_H

#include <optional>
#include <span>

#include <grafkit/common.h>
#include <grafkit/core/image.h>
#include <grafkit/descriptors/image_desc.h>
#include <grafkit/interface/resource.h>

namespace Grafkit::Resource {

	class ImageBuilder : public ResourceBuilder<ImageDesc, Core::Image> {
	public:
		explicit ImageBuilder(const ImageDesc& desc)
			: ResourceBuilder<ImageDesc, Core::Image>(desc)
		{
		}

		[[nodiscard]] bool ResolveDependencies([[maybe_unused]] const RefWrapper<ResourceManager>& resources) final
		{
			return true;
		}

		void Build(const Core::DeviceRef& device) override;
	};

	class SolidImageBuilder : public ResourceBuilder<SolidImageDesc, Core::Image> {
	public:
		explicit SolidImageBuilder(const SolidImageDesc& desc)
			: ResourceBuilder<SolidImageDesc, Core::Image>(desc)
		{
		}

		[[nodiscard]] bool ResolveDependencies([[maybe_unused]] const RefWrapper<ResourceManager>& resources) final
		{
			return true;
		}

		void Build(const Core::DeviceRef& device) override;
	};

	class CheckerImageBuilder : public ResourceBuilder<CheckerImageDesc, Core::Image> {
	public:
		explicit CheckerImageBuilder(const DescriptorType& desc)
			: ResourceBuilder<CheckerImageDesc, Core::Image>(desc)
		{
		}

		[[nodiscard]] bool ResolveDependencies([[maybe_unused]] const RefWrapper<ResourceManager>& resources) final
		{
			return true;
		}

		void Build(const Core::DeviceRef& device) override;
	};

} // namespace Grafkit::Resource

#endif // IMAGE_BUILDER_H
