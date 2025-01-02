#ifndef GRAFKIT_CORE_DESCRIPTOR_POOL_H
#define GRAFKIT_CORE_DESCRIPTOR_POOL_H

#include <grafkit/common.h>
#include <vector>

namespace Grafkit::Core
{
	class GKAPI DescriptorPool
	{
	public:
		struct PoolSet
		{
			VkDescriptorType type;
			uint32_t size;
		};

		explicit DescriptorPool(const DeviceRef &device, const uint32_t maxSets, std::vector<PoolSet> poolSets = {});

		virtual ~DescriptorPool();

		[[nodiscard]] VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetLayout &layout);
		[[nodiscard]] std::vector<VkDescriptorSet> AllocateDescriptorSets(const VkDescriptorSetLayout &layout,
			const uint32_t count);

		void DeallocateDescriptorSet(VkDescriptorSet descriptorSet);

	private:
		const DeviceRef m_device;
		std::vector<PoolSet> m_poolSets;
		std::vector<VkDescriptorPool> m_fullPools;
		std::vector<VkDescriptorPool> m_readyPools;

		std::unordered_map<VkDescriptorSet, VkDescriptorPool> m_descriptorPoolMap;

		uint32_t m_maxSets;

		[[nodiscard]] VkDescriptorPool CreatePool();
		[[nodiscard]] VkDescriptorPool GetPool();
	};

} // namespace Grafkit::Core

#endif // GRAFKIT_CORE_DESCRIPTOR_POOL_H
