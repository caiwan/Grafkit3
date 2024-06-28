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
		virtual std::shared_ptr<IResource> Raw() const = 0;
		virtual std::string Kind() const = 0;
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

		[[nodiscard]] std::shared_ptr<IResource> Raw() const override
		{
			const auto p = std::dynamic_pointer_cast<IResource>(m_resource);
			assert(p);
			return p;
		}

		[[nodiscard]] std::string Kind() const override { return ResourceType::KIND.data(); }

	protected:
		DescriptorType m_descriptor {};
		ResourcePtr m_resource {};
	};

	// ---------------------------------------------------------------------------------

	class ResoureManger {
	public:
		ResoureManger(const Core::DeviceRef& device)
			: m_device(device)
		{
		}

		ResoureManger(const ResoureManger&) = delete; // Delete copy constructor
		ResoureManger& operator=(const ResoureManger&) = delete; // Delete copy assignment operator

		virtual ~ResoureManger() = default;

		template <typename ResourceType, typename BuilderType>
		std::shared_ptr<ResourceType> LoadResource(const std::string& name, BuilderType&& builder)
		{
			builder.Build(m_device, Grafkit::MakeReference(*this));
			std::string kind = builder.Kind();
			m_resources[kind][name] = builder.Raw();
			return builder.Resource();
		}

		template <typename ResourceType> std::shared_ptr<ResourceType> Get(const std::string& name)
		{
			const std::string kind = ResourceType::KIND.data();
			const auto it = m_resources.find(kind);
			if (it != m_resources.end()) {
				const auto it2 = it->second.find(name);
				if (it2 != it->second.end()) {
					return std::dynamic_pointer_cast<ResourceType>(it2->second);
				}
			}
			return nullptr;
		}

		void CollectGarbage();

	private:
		Core::DeviceRef m_device;
		std::map<std::string, std::map<std::string, std::shared_ptr<IResource>>> m_resources;
	};

} // namespace Grafkit::Resource

#endif // BUILDER_H
