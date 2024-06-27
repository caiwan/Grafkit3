#ifndef GRAFKIT_IMAGE_BUILDER_H
#define GRAFKIT_IMAGE_BUILDER_H

#include <optional>
#include <span>

#include <grafkit/common.h>
#include <grafkit/core/image.h>
#include <grafkit/resource/resource.h>

namespace Grafkit {
	namespace Resource {

		struct ImageDesc {
			std::span<uint8_t> image;
			glm::uvec3 size;
			uint32_t format;
			uint32_t channels;
			bool useMipmap;
		};

		class ImageBuilder : public ResourceBuilder<ImageDesc, Core::Image> {
		public:
			explicit ImageBuilder(const ImageDesc& desc)
				: ResourceBuilder<ImageDesc, Core::Image>(desc)
			{
			}
			virtual void Build(const Core::DeviceRef& device) override;

			std::string Kind() const override { return "Image"; }
		};

		struct SolidImageDesc {
			glm::u8vec4 color;
		};

		class SolidImageBuilder : public ResourceBuilder<SolidImageDesc, Core::Image> {
		public:
			explicit SolidImageBuilder(const SolidImageDesc& desc)
				: ResourceBuilder<SolidImageDesc, Core::Image>(desc)
			{
			}

			virtual void Build(const Core::DeviceRef& device) override;

			std::string Kind() const override { return "Image"; }
		};

		struct CheckerImageDesc {
			glm::uvec3 size;
			glm::uvec2 divisions;
			glm::u8vec4 color1;
			glm::u8vec4 color2;
			bool useMipmap = false;
		};

		class CheckerImageBuilder : public ResourceBuilder<CheckerImageDesc, Core::Image> {
		public:
			explicit CheckerImageBuilder(const DescriptorType& desc)
				: ResourceBuilder<CheckerImageDesc, Core::Image>(desc)
			{
			}

			virtual void Build(const Core::DeviceRef& device) override;

			std::string Kind() const override { return "Image"; }
		};

	} // namespace Resource
} // namespace Grafkit

#endif // IMAGE_BUILDER_H
