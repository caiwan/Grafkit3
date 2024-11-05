#include "stdafx.h"

#include "grafkit/core/descriptor_pool.h"
#include "grafkit/core/device.h"
#include "grafkit/core/log.h"

using namespace Grafkit::Core;

DescriptorPool::DescriptorPool(const DeviceRef &device, const uint32_t maxSets, std::vector<PoolSet> poolSets)
	: m_device(device), m_poolSets(std::move(poolSets)), m_maxSets(maxSets)
{
	assert(m_maxSets > 0);
	assert(!m_poolSets.empty());
	m_readyPools.push_back(CreatePool());
}

DescriptorPool::~DescriptorPool()
{
	for (auto &pool : m_readyPools)
	{
		vkDestroyDescriptorPool(**m_device, pool, nullptr);
	}

	for (auto &pool : m_fullPools)
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
	return descriptorSet;
}

[[nodiscard]] std::vector<VkDescriptorSet> DescriptorPool::AllocateDescriptorSets(
	const VkDescriptorSetLayout &layout, const uint32_t count)
{
	Log::Instance().Debug("Allocating %d descriptor sets", count);
	std::vector<VkDescriptorSet> descriptorSets;
	for (uint32_t i = 0; i < count; i++)
	{
		descriptorSets.emplace_back(AllocateDescriptorSet(layout));
	}
	return descriptorSets;
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

	VkDescriptorPoolCreateInfo info = Initializers::DescriptorPoolCreateInfo(poolSizes, m_maxSets);
	VkDescriptorPool pool = VK_NULL_HANDLE;
	vkCreateDescriptorPool(**m_device, &info, nullptr, &pool);

	return pool;
}

VkDescriptorPool DescriptorPool::GetPool()
{
	// Get or create a pool [to allocate from]
	VkDescriptorPool pool;
	// check if we have a pool ready
	if (!m_readyPools.empty())
	{
		pool = m_readyPools.back();
		m_readyPools.pop_back();
	}
	else
	{
		// need to create a new pool
		pool = CreatePool();
		if (pool == nullptr)
		{
			throw std::runtime_error("Failed to create descriptor pool");
		}
	}
	return pool;
}

// ---
