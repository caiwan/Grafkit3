#include "stdafx.h"

#include <grafkit/core/descriptor_pool.h>
#include <grafkit/core/device.h>

using namespace Grafkit::Core;

constexpr uint32_t MAX_SET_COUNT = 4096;

DescriptorPool::DescriptorPool(const DeviceRef& device, const uint32_t maxSets, const std::vector<PoolSet> poolRatios)
	: m_device(device)
	, m_poolSets(poolRatios.begin(), poolRatios.end())
	, m_maxSets(std::min(maxSets, MAX_SET_COUNT))
{
	m_readyPools.push_back(CreatePool());
}

DescriptorPool::~DescriptorPool()
{
	for (auto& pool : m_readyPools) {
		vkDestroyDescriptorPool(**m_device, pool, nullptr);
	}

	for (auto& pool : m_fullPools) {
		vkDestroyDescriptorPool(**m_device, pool, nullptr);
	}
}

VkDescriptorSet DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout& layout)
{
	// get or create a pool to allocate from
	VkDescriptorPool poolToUse = GetPool();

	VkDescriptorSetAllocateInfo allocInfo = Initializers::DescriptorSetAllocateInfo(poolToUse, &layout, 1);

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(**m_device, &allocInfo, &descriptorSet);

	// allocation failed. Try again with another pool
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

		m_fullPools.push_back(poolToUse);

		poolToUse = GetPool();
		allocInfo.descriptorPool = poolToUse;

		if (vkAllocateDescriptorSets(**m_device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor set");
		}
	}

	m_readyPools.push_back(poolToUse);
	return descriptorSet;
}

[[nodiscard]] std::vector<VkDescriptorSet> DescriptorPool::AllocateDescriptorSets(
	const VkDescriptorSetLayout& layout, const uint32_t count)
{
	std::vector<VkDescriptorSet> descriptorSets;
	for (uint32_t i = 0; i < count; i++) {
		descriptorSets.emplace_back(AllocateDescriptorSet(layout));
	}
	return descriptorSets;
}

VkDescriptorPool DescriptorPool::CreatePool()
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSet ratio : m_poolSets) {
		poolSizes.push_back(Initializers::DescriptorPoolSize(ratio.type, static_cast<uint32_t>(ratio.size)));

		ratio.size = std::min(ratio.size + ratio.size / 2, MAX_SET_COUNT);
	}

	VkDescriptorPoolCreateInfo info = Initializers::DescriptorPoolCreateInfo(poolSizes, m_maxSets);
	VkDescriptorPool pool;
	vkCreateDescriptorPool(**m_device, &info, nullptr, &pool);

	return pool;
}

VkDescriptorPool DescriptorPool::GetPool()
{
	// Get or create a pool [to allocate from]
	VkDescriptorPool pool;
	// check if we have a pool ready
	if (m_readyPools.size() != 0) {
		pool = m_readyPools.back();
		m_readyPools.pop_back();
	} else {
		// need to create a new pool
		pool = CreatePool();
	}

	m_maxSets = std::min(m_maxSets + m_maxSets / 2, MAX_SET_COUNT);

	return pool;
}

// ---
