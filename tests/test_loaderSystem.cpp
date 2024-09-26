#include <fstream>
#include <grafkit_loader/asset_loader_system.h>
#include <gtest/gtest.h>

class TestLoaderSystem : public ::testing::Test {
protected:
	void SetUp() override
	{
		m_assetLoader = std::make_unique<Grafkit::Asset::JsonAssetLoader>();
		// m_resources = std::make_unique<Grafkit::Resource::ResourceManager>(
		// 	Grafkit::MakeReferenceAs<Grafkit::Asset::IAssetLoader>(*m_assetLoader));
	}

	void TearDown() override { }

	std::unique_ptr<Grafkit::Asset::JsonAssetLoader> m_assetLoader;
	// Grafkit::Resource::ResourceManagerPtr m_resources;
};

TEST_F(TestLoaderSystem, ReadData)
{

	std::vector<uint8_t> data;
	m_assetLoader->ReadData(typeid(void), "test.json", data);
	ASSERT_FALSE(data.empty());
}
