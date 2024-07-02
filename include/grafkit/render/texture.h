#ifndef GRAFKIT_TEXTURE_H
#define GRAFKIT_TEXTURE_H

#include <grafkit/common.h>

namespace Grafkit {

	class GKAPI Texture {
	public:
		explicit Texture(const Core::DeviceRef& device, Core::ImagePtr image, VkSampler sampler = VK_NULL_HANDLE);
		~Texture();

		void Bind(const Core::CommandBufferRef& commandBuffer,
			const Core::PipelinePtr& pipeline,
			const Core::DescriptorSetPtr& descriptor,
			const uint32_t binding,
			const uint32_t set); // TOOD: Texture should take care aboput binding itself to a pipeline

		[[nodiscard]] Core::ImagePtr GetImage() const { return m_image; }
		[[nodiscard]] VkSampler GetSampler() const { return m_sampler; }

	private:
		const Core::DeviceRef m_device;
		Core::ImagePtr m_image;
		VkSampler m_sampler;
	};

} // namespace Grafkit

#endif // _GRAFKIT_TEXTURE_H_
