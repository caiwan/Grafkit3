#include <fstream>
#include <grafkit_loader/asset_loader_system.h>
#include <gtest/gtest.h>

#include <grafkit/resource/material_builder.h>

using Grafkit::Asset::JsonAssetLoader;
using Grafkit::Asset::SerializedAssetPtr;

class TestLoaderSystem : public ::testing::Test {
protected:
	void SetUp() override { m_assetLoader = std::make_unique<JsonAssetLoader>(); }

	void TearDown() override { }

	std::unique_ptr<Grafkit::Asset::JsonAssetLoader> m_assetLoader;
};

TEST_F(TestLoaderSystem, ReadData)
{
	const SerializedAssetPtr serializedData = m_assetLoader->Load("test.json");
	ASSERT_TRUE(serializedData != nullptr);

	std::vector<uint8_t> data;
	serializedData->ReadData(data);
	ASSERT_FALSE(data.empty());
}

TEST_F(TestLoaderSystem, ReadDataAndDeserialize)
{
	const SerializedAssetPtr serializedData = m_assetLoader->Load("test.json");
	ASSERT_TRUE(serializedData != nullptr);

	const auto materialDesc = serializedData->DeserializeAs<Grafkit::Resource::MaterialDesc>();

	ASSERT_EQ(materialDesc.name, "test");
}
