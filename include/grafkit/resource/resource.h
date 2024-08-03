#ifndef GRAFKIT_BUILDER_H
#define GRAFKIT_BUILDER_H

#include <grafkit/common.h>

#include <fstream>
#include <functional>
#include <string>
#include <typeindex>

namespace Grafkit::Asset {
	class AssetLoader;
	using AssetLoaderRef = Grafkit::RefWrapper<AssetLoader>;
} // namespace Grafkit::Asset

namespace Grafkit::Resource {
	class ResourceManager;

	// MARK: Resource Loader Interface
	class GKAPI IResourceLoader {
	public:
		IResourceLoader() = default;
		IResourceLoader(const IResourceLoader&) = delete; // Delete copy constructor
		IResourceLoader& operator=(const IResourceLoader&) = delete; // Delete copy assignment operator

		virtual ~IResourceLoader() = default;

		[[nodiscard]] virtual bool ResolveDependencies(const RefWrapper<ResourceManager>& assetManager) = 0;
		virtual void Build(const Core::DeviceRef& device) = 0;

		[[nodiscard]] virtual std::shared_ptr<void> GetResource() const = 0;
	};

	template <class DescriptorT, class ResourceT> class GKAPI ResourceBuilder : public IResourceBuilder {
	public:
		using ResourceType = ResourceT;
		using DescriptorType = DescriptorT;
		using ResourcePtr = std::shared_ptr<ResourceType>;
		using Builder = ResourceBuilder<DescriptorType, ResourceType>;

		explicit ResourceBuilder(const DescriptorType& desc = {})
			: m_descriptor(desc)
		{
		}

		[[nodiscard]] ResourcePtr BuildResource(
			const Core::DeviceRef& device, const RefWrapper<ResourceManager>& assetManager)
		{
			if (!ResolveDependencies(assetManager)) {
				return nullptr;
			}
			Build(device);
			return m_resource;
		}

		// 	[[nodiscard]] const DescriptorType& Descriptor() const { return m_descriptor; }
		// 	[[nodiscard]] ResourcePtr Resource() const { return m_resource; }

		// protected:
		// 	DescriptorType m_descriptor {};
		// 	ResourcePtr m_resource {};
	};

	// -----------------------------------------------------------------------------
	// MARK: Loader system
	// TODO: Create a mixin for it for Application
	class ResourceLoaderRegistry {
	public:
		using LoaderFunc = std::function<bool(std::shared_ptr<void>&, const std::string&, ResourceManager&)>;

		static ResourceLoaderRegistry& Instance()
		{
			static ResourceLoaderRegistry instance;
			return instance;
		}

		template <typename T> void RegisterLoader(LoaderFunc func) { loaders[typeid(T)] = func; }
		LoaderFunc GetLoader(std::type_index type) const;

	private:
		ResourceLoaderRegistry() = default;
		std::unordered_map<std::type_index, LoaderFunc> loaders;
	};

	class ResourceManager {
	public:
		ResourceManager(const Asset::AssetLoaderRef& loader)
			: m_loader(loader)
		{
		}

		// Template method to load different types of assets
		template <typename T> std::shared_ptr<T> Load(const std::string& name)
		{
			std::shared_ptr<void> asset = std::make_shared<T>();
			ResourceLoaderRegistry::LoaderFunc loader = ResourceLoaderRegistry::Instance().GetLoader(typeid(T));
			if (loader && loader(asset, filename, *this)) {
				assets[typeid(T)][name] = asset;
				return std::static_pointer_cast<T>(asset);
			} else {
				// std::cerr << "Failed to load asset: " << filename << std::endl;
				return nullptr;
			}
		}

		// Template method to get different types of assets
		template <typename T> std::shared_ptr<T> Get(const std::string& name) const
		{
			auto typeAssets = assets.find(typeid(T));
			if (typeAssets != assets.end()) {
				auto it = typeAssets->second.find(name);
				if (it != typeAssets->second.end()) {
					return std::static_pointer_cast<T>(it->second);
				}
			}
			// std::cerr << "Asset not found: " << name << std::endl;
			return nullptr;
		}

	private:
		const Asset::AssetLoaderRef m_loader;
		std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<void>>> m_assets;
	};

	// Mixins

} // namespace Grafkit::Resource

#endif // BUILDER_H
