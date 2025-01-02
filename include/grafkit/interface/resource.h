#ifndef GRAFKIT_BUILDER_H
#define GRAFKIT_BUILDER_H

#include <grafkit/common.h>

#include <fstream>
#include <functional>
#include <string>
#include <typeindex>

namespace Grafkit::Asset
{
	class IAssetLoader;
	using IAssetLoaderRef = Grafkit::RefWrapper<IAssetLoader>;
} // namespace Grafkit::Asset

namespace Grafkit::Resource
{
	class ResourceManager;
	using ResourceManagerPtr = std::unique_ptr<ResourceManager>;
	using ResourceManagerRef = Grafkit::RefWrapper<ResourceManager>;

	// MARK: Resource Loader Interface
	class GKAPI IResourceLoader
	{
	public:
		IResourceLoader() = default;
		IResourceLoader(const IResourceLoader &) = delete;			  // Delete copy constructor
		IResourceLoader &operator=(const IResourceLoader &) = delete; // Delete copy assignment operator

		virtual ~IResourceLoader() = default;

		[[nodiscard]] virtual bool ResolveDependencies(const RefWrapper<ResourceManager> &resources) = 0;
		virtual void Build(const Core::DeviceRef &device) = 0;

		[[nodiscard]] virtual std::shared_ptr<void> GetResource() const = 0;
	};

	template <class DescriptorT, class ResourceT>
	class GKAPI ResourceBuilder : public IResourceLoader
	{
	public:
		using ResourceType = ResourceT;
		using DescriptorType = DescriptorT;
		using ResourcePtr = std::shared_ptr<ResourceType>;
		using Builder = ResourceBuilder<DescriptorType, ResourceType>;

		explicit ResourceBuilder(const DescriptorType &desc = {})
			: m_descriptor(desc)
		{
		}

		[[nodiscard]] ResourcePtr BuildResource(const Core::DeviceRef &device,
			const RefWrapper<ResourceManager> &resources)
		{
			if (!ResolveDependencies(resources))
			{
				return nullptr;
			}
			Build(device);
			return m_resource;
		}

		[[nodiscard]] const DescriptorType &Descriptor() const
		{
			return m_descriptor;
		}
		[[nodiscard]] ResourcePtr Resource() const
		{
			return m_resource;
		}

		[[nodiscard]] std::shared_ptr<void> GetResource() const final
		{
			return m_resource;
		}

		ResourceBuilder(const ResourceBuilder &) = delete;			  // Delete copy constructor
		ResourceBuilder &operator=(const ResourceBuilder &) = delete; // Delete copy assignment operator
		ResourceBuilder(ResourceBuilder &&) = delete;				  // Delete move constructor
		ResourceBuilder &operator=(ResourceBuilder &&) = delete;	  // Delete move assignment operator

	protected:
		DescriptorType m_descriptor{};
		ResourcePtr m_resource{};
	};

	// -----------------------------------------------------------------------------
	// MARK: Loader system
	// TODO: Create a mixin for it for Application
	class ResourceLoaderRegistry
	{
	public:
		using LoaderFunc = std::function<bool(std::shared_ptr<void> &, const std::string &, ResourceManager &)>;

		static ResourceLoaderRegistry &Instance()
		{
			static ResourceLoaderRegistry instance;
			return instance;
		}

		template <typename T>
		void RegisterLoader(LoaderFunc func)
		{
			m_loaders[typeid(T)] = std::move(func);
		}
		LoaderFunc GetLoader(std::type_index type) const;

	private:
		ResourceLoaderRegistry() = default;
		std::unordered_map<std::type_index, LoaderFunc> m_loaders;
	};

	class ResourceManager
	{
	public:
		explicit ResourceManager(const Asset::IAssetLoaderRef loader)
			: m_loader(loader)
		{
		}

		// Template method to load different types of assets
		template <typename T>
		std::shared_ptr<T> Load(const std::string &filename)
		{
			std::shared_ptr<void> asset = std::make_shared<T>();
			ResourceLoaderRegistry::LoaderFunc loader = ResourceLoaderRegistry::Instance().GetLoader(typeid(T));
			if (loader && loader(asset, filename, *this))
			{
				m_resources[typeid(T)][filename] = asset;
				return std::static_pointer_cast<T>(asset);
			}
			return nullptr;
		}

		// Template method to get different types of assets
		template <typename T>
		std::shared_ptr<T> Get(const std::string &name) const
		{
			auto typeAssets = m_resources.find(typeid(T));
			if (typeAssets != m_resources.end())
			{
				auto it = typeAssets->second.find(name);
				if (it != typeAssets->second.end())
				{
					return std::static_pointer_cast<T>(it->second);
				}
			}
			return nullptr;
		}

	private:
		const Asset::IAssetLoaderRef m_loader;
		std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<void>>> m_resources;
	};

	// Mixins

} // namespace Grafkit::Resource

#endif // BUILDER_H
