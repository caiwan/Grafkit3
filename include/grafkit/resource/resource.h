#ifndef GRAFKIT_BUILDER_H
#define GRAFKIT_BUILDER_H

#include <fstream>
#include <grafkit/common.h>
#include <string>

namespace Grafkit::Resource {

	class IResourceBuilder {
	public:
		IResourceBuilder() = default;
		IResourceBuilder(const IResourceBuilder&) = delete; // Delete copy constructor
		IResourceBuilder& operator=(const IResourceBuilder&) = delete; // Delete copy assignment operator

		virtual ~IResourceBuilder() = default;
		virtual void Build(const Core::DeviceRef& device) = 0;
	};

	template <typename DescriptorT, typename ResourceT> class ResourceBuilder : public IResourceBuilder {
	public:
		using ResourceType = ResourceT;
		using DescriptorType = DescriptorT;
		using ResourcePtr = std::shared_ptr<ResourceType>;
		using Builder = ResourceBuilder<DescriptorType, ResourceType>;

		explicit ResourceBuilder(const DescriptorType& desc = {})
			: m_descriptor(desc)
		{
		}

		[[nodiscard]] ResourcePtr BuildResource(const Core::DeviceRef& device)
		{
			Build(device);
			return m_resource;
		}

		[[nodiscard]] const DescriptorType& Descriptor() const { return m_descriptor; }
		[[nodiscard]] ResourcePtr Resource() const { return m_resource; }

	protected:
		DescriptorType m_descriptor {};
		ResourcePtr m_resource {};
	};

} // namespace Grafkit::Resource

#endif // BUILDER_H
