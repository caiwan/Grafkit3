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
		virtual void ReadData(const std::string& assetName, std::vector<uint8_t>& data) const = 0;
	};

	class GKAPI ISerializedAsset {
	public:
		virtual ~ISerializedAsset() = default;

		virtual void Deserialize(const std::type_index& assetType, std::any& object) = 0;

		virtual void ReadData(std::vector<uint8_t>& data) const = 0;

		template <class T> T DeserializeAs()
		{
			std::any object = T();
			Deserialize(typeid(T), object);
			return std::any_cast<T>(object);
		}
	};

	class GKAPI IAssetLoader {
	public:
		IAssetLoader() = default;
		IAssetLoader(const IAssetLoader&) = delete; // Delete copy constructor
		IAssetLoader& operator=(const IAssetLoader&) = delete; // Delete copy assignment operator

		virtual ~IAssetLoader() = default;

		[[nodiscard]] virtual SerializedAssetPtr Load(const std::string& assetName) const = 0;
	};

	// MARK: Asset loader implementation
	template <class AssetSourceT, class SerializedAssetT>
	class GKAPI AssetLoader : virtual public IAssetLoader, virtual protected AssetSourceT {
	public:
		using SerializedAssetType = SerializedAssetT;

		AssetLoader() = default;
		AssetLoader(const AssetLoader&) = delete; // Delete copy constructor
		AssetLoader& operator=(const AssetLoader&) = delete; // Delete copy assignment operator

		~AssetLoader() override = default;

		[[nodiscard]] SerializedAssetPtr Load(const std::string& assetName) const override
		{
			std::vector<uint8_t> data;
			AssetSourceT::ReadData(assetName, data);
			return std::make_shared<SerializedAssetT>(std::move(data));
		}
	};

} // namespace Grafkit::Asset

#endif // ASSET_H
