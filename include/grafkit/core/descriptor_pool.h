#ifndef GRAFKIT_CORE_DESCRIPTOR_POOL_H
#define GRAFKIT_CORE_DESCRIPTOR_POOL_H

#include <grafkit/common.h>
#include <vector>

namespace Grafkit::Core {
	class GKAPI DescriptorPool {
	public:
		struct PoolSet {
			VkDescriptorType type;
			uint32_t size;
		};

		explicit DescriptorPool(
			const DeviceRef& device, const uint32_t maxSets, const std::vector<PoolSet>& poolRatios = {});

		virtual ~DescriptorPool();

		[[nodiscard]] VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetLayout& layout);
		[[nodiscard]] std::vector<VkDescriptorSet> AllocateDescriptorSets(
			const VkDescriptorSetLayout& layout, const uint32_t count);

	private:
		const DeviceRef m_device;
		std::vector<PoolSet> m_poolSets;
		std::vector<VkDescriptorPool> m_fullPools;
		std::vector<VkDescriptorPool> m_readyPools;
		uint32_t m_maxSets;

		[[nodiscard]] VkDescriptorPool CreatePool();
		[[nodiscard]] VkDescriptorPool GetPool();
	};

} // namespace Grafkit::Core

#endif // GRAFKIT_CORE_DESCRIPTOR_POOL_H
