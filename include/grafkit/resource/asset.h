#ifndef ASSET_H
#define ASSET_H

#include <grafkit/common.h>

#include <any>
#include <functional>
#include <string>
#include <typeindex>

/**
 * @brief Asset loader interface
 * Manage the loading of assets from the file system
 */
namespace Grafkit::Asset {
	class ISerializedAsset;
	using SerializedAssetPtr = std::shared_ptr<ISerializedAsset>;

	class IAssetLoader;
	using AssetLoaderPtr = std::unique_ptr<IAssetLoader>;
	using AssetLoaderRef = Grafkit::RefWrapper<IAssetLoader>;

	class GKAPI IAssetSource {
	public:
		virtual ~IAssetSource() = default;
		virtual void ReadData(std::type_index assetType, const std::string& assetName, std::vector<uint8_t>& data) const
			= 0;

		template <class T> void ReadData(const std::string& assetName, std::vector<uint8_t>& data) const
		{
			ReadData(typeid(T), assetName, data);
		}
	};

	class GKAPI ISerializedAsset {
	public:
		virtual ~ISerializedAsset() = default;

		virtual void Deserialize(std::any& object) = 0;

		template <class T> void DeserializeAs()
		{
			std::any object = T();
			Deserialize(object);
			return std::any_cast<T>(object);
		}
	};

	class GKAPI IAssetLoader {
	public:
		IAssetLoader() = default;
		IAssetLoader(const IAssetLoader&) = delete; // Delete copy constructor
		IAssetLoader& operator=(const IAssetLoader&) = delete; // Delete copy assignment operator

		virtual ~IAssetLoader() = default;

		[[nodiscard]] virtual SerializedAssetPtr Load(std::type_index typeIndex, const std::string& assetName) const
			= 0;

		template <class T> SerializedAssetPtr LoadAsset(const std::string& assetName)
		{
			return Load(typeid(T), assetName);
		}
	};

	// MARK: Asset loader implementation
	template <class AssetSourceT, class SerializedAssetT>
	class GKAPI AssetLoader : virtual public IAssetLoader, virtual public AssetSourceT {
	public:
		using SerializedAssetType = SerializedAssetT;

		AssetLoader() = default;
		AssetLoader(const AssetLoader&) = delete; // Delete copy constructor
		AssetLoader& operator=(const AssetLoader&) = delete; // Delete copy assignment operator

		~AssetLoader() override = default;

		[[nodiscard]] SerializedAssetPtr Load(std::type_index typeIndex, const std::string& assetName) const override
		{
			std::vector<uint8_t> data;
			AssetSourceT::ReadData(typeIndex, assetName, data);
			return std::make_shared<SerializedAssetT>(typeIndex, data);
		}

		template <class T> [[nodiscard]] SerializedAssetPtr LoadAsset(const std::string& assetName) const
		{
			return Load(typeid(T), assetName);
		}

		template <class T> [[nodiscard]] T LoadAs(const std::string& assetName) const
		{
			return Load<T>(assetName)->template DeserializeAs<T>();
		}
	};

} // namespace Grafkit::Asset

#endif // ASSET_H
