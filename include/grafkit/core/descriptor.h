#ifndef GRAFKIT_CORE_DESCRIPTOR_H
#define GRAFKIT_CORE_DESCRIPTOR_H

#include <grafkit/common.h>
#include <grafkit/core/buffer.h>
#include <optional>
#include <vector>

namespace Grafkit::Core
{

	class GKAPI DescriptorSet
	{
	public:
		explicit DescriptorSet(const DeviceRef &device,
			const VkDescriptorSetLayout layout,
			std::vector<VkDescriptorSet> descriptors,
			const uint32_t bindOffset = 0);

		virtual ~DescriptorSet();

		void Bind(const Core::CommandBufferRef &commandBuffer,
			const VkPipelineLayout &pipelineLayout,
			const uint32_t frame) const noexcept;

		void Update(const Buffer &buffer,
			const uint32_t binding,
			const std::optional<uint32_t> frame = std::nullopt) noexcept;

		void Update(const RingBuffer &buffer, const uint32_t binding) noexcept;

		void Update(const ImagePtr &image,
			const VkSampler &sampler,
			const uint32_t binding,
			const std::optional<uint32_t> frame = std::nullopt) noexcept;

		[[nodiscard]] inline const VkDescriptorSetLayout &GetVkDescriptorSetLayout() const noexcept
		{
			return m_layout;
		}

		[[nodiscard]] inline const VkDescriptorSet &GetVkDescriptorSet(const uint32_t currentFrame) const noexcept
		{
			return m_descriptorSets[currentFrame];
		}

		// MARLK: Factory methods
		static DescriptorSetPtr
		Create(const DeviceRef &device, const uint32_t set, const std::vector<VkDescriptorSetLayoutBinding> &bindings);

	private:
		void Update(const VkDescriptorBufferInfo &bufferInfo,
			const uint32_t binding,
			const std::optional<uint32_t> frame = std::nullopt) noexcept;

		void Update(const VkDescriptorImageInfo &imageInfo,
			const uint32_t binding,
			const std::optional<uint32_t> frame = std::nullopt) noexcept;

		const DeviceRef m_device;
		std::vector<VkDescriptorSet> m_descriptorSets;
		VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
		uint32_t m_descriptorOffset = 0;
	};

	// --

	// class GKAPI DescriptorBuilder {

	// public:
	// 	explicit DescriptorBuilder(const DeviceRef& device);

	// 	DescriptorBuilder& AddLayoutBindings(const DescriptorSetLayoutBinding& bindings);

	// 	[[nodiscard]] DescriptorSetPtr Build();

	// private:
	// 	const DeviceRef m_device;
	// 	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	// 	uint32_t m_descriptorSet = 0;
	// };

	// class GKAPI DescriptorFactory {
	// public:
	// 	[[nodiscard]] DescriptorBuilder CreateDescriptorBuilder(const DeviceRef& device) const
	// 	{
	// 		return DescriptorBuilder(device);
	// 	}
	// };

} // namespace Grafkit::Core

#endif // GRAFKIT_CORE_DESCRIPTOR_H
