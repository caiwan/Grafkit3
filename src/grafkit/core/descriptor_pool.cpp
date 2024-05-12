#include "stdafx.h"

#include "grafkit/core/descriptor_pool.h"
#include "grafkit/core/device.h"
#include "grafkit/core/log.h"

using namespace Grafkit::Core;

DescriptorPool::DescriptorPool(const DeviceRef &device, const uint32_t maxSets, std::vector<PoolSet> poolSets)
	: m_device(device)
	, m_poolSets(std::move(poolSets))
	, m_maxSets(maxSets)
{
	assert(m_maxSets > 0);
	assert(!m_poolSets.empty());
	m_readyPools.push_back(CreatePool());
}

DescriptorPool::~DescriptorPool()
{
	if (m_readyPools.empty() && m_fullPools.empty())
	{
		return;
	}

	if (!m_descriptorPoolMap.empty())
	{
		Log::Instance().Error("Descriptor pool destroyed with %d descriptor sets still allocated",
			m_descriptorPoolMap.size());
		for (const auto &pair : m_descriptorPoolMap)
		{
			Log::Instance().Trace("Freeing descriptor set %p from pool %p", pair.first, pair.second);
			vkFreeDescriptorSets(**m_device, pair.second, 1, &pair.first);
		}
	}
	else
	{
		Log::Instance().Trace("Descriptor pool destroyed with no descriptor sets left allocated");
	}

	// clean up all pools
	for (const auto &pool : m_readyPools)
	{
		vkDestroyDescriptorPool(**m_device, pool, nullptr);
	}

	for (const auto &pool : m_fullPools)
	{
		vkDestroyDescriptorPool(**m_device, pool, nullptr);
	}
}

VkDescriptorSet DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout &layout)
{
	// get or create a pool to allocate from
	VkDescriptorPool poolToUse = GetPool();

	VkDescriptorSetAllocateInfo allocInfo = Initializers::DescriptorSetAllocateInfo(poolToUse, &layout, 1);

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkResult result = vkAllocateDescriptorSets(**m_device, &allocInfo, &descriptorSet);

	// allocation failed. Try again with another pool
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
	{
		m_fullPools.push_back(poolToUse);

		poolToUse = GetPool();
		allocInfo.descriptorPool = poolToUse;

		result = vkAllocateDescriptorSets(**m_device, &allocInfo, &descriptorSet);
	}

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor set");
	}

	Log::Instance().Trace("Allocating descriptor. Object=%p", descriptorSet);

	m_readyPools.push_back(poolToUse);

	m_descriptorPoolMap.emplace(descriptorSet, poolToUse);

	return descriptorSet;
}

[[nodiscard]] std::vector<VkDescriptorSet> DescriptorPool::AllocateDescriptorSets(const VkDescriptorSetLayout &layout,
	const uint32_t count)
{
	Log::Instance().Trace("Allocating %d descriptor sets", count);
	std::vector<VkDescriptorSet> descriptorSets;
	for (uint32_t i = 0; i < count; i++)
	{
		descriptorSets.emplace_back(AllocateDescriptorSet(layout));
	}
	return descriptorSets;
}

void Grafkit::Core::DescriptorPool::DeallocateDescriptorSet(VkDescriptorSet descriptorSet)
{
	auto poolIt = m_descriptorPoolMap.find(descriptorSet);
	assert(poolIt != m_descriptorPoolMap.end());

	Log::Instance().Trace("Freeing descriptor set %p from pool %p", poolIt->first, poolIt->second);

	vkFreeDescriptorSets(**m_device, poolIt->second, 1, &descriptorSet);
	m_descriptorPoolMap.erase(descriptorSet);
}

VkDescriptorPool DescriptorPool::CreatePool()
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.reserve(m_poolSets.size());
	for (const auto &poolSize : m_poolSets)
	{
		poolSizes.push_back(Initializers::DescriptorPoolSize(poolSize.type, static_cast<uint32_t>(poolSize.size)));
	}

	Log::Instance().Trace("Creating descriptor pool with %d sets", m_maxSets);

	VkDescriptorPool pool = VK_NULL_HANDLE;
	VkDescriptorPoolCreateInfo info = Initializers::DescriptorPoolCreateInfo(poolSizes, m_maxSets);
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	vkCreateDescriptorPool(**m_device, &info, nullptr, &pool);

	// If the pool was a full pool, move it back to ready pools to reuse it
	auto it = std::find(m_fullPools.begin(), m_fullPools.end(), pool);
	if (it != m_fullPools.end())
	{
		m_fullPools.erase(it);
		m_readyPools.push_back(pool);
	}

	return pool;
}

VkDescriptorPool DescriptorPool::GetPool()
{
	// check if we have a pool ready
	if (!m_readyPools.empty())
	{
		VkDescriptorPool pool = m_readyPools.back();
		m_readyPools.pop_back();
		return pool;
	}

	// need to create a new pool
	VkDescriptorPool pool = CreatePool();
	if (pool == nullptr)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
	return pool;
}

// ---
