#include <optional>
#include <string>
#include <vector>
//
#include <grafkit/application.h>
#include <grafkit/common.h>
#include <grafkit/core/buffer.h>
#include <grafkit/core/command_buffer.h>
#include <grafkit/core/pipeline.h>
#include <grafkit/core/render_target.h>
#include <grafkit/core/window.h>
#include <grafkit/render.h>
#include <grafkit/render/material.h>
#include <grafkit/render/mesh.h>
#include <grafkit/render/scenegraph.h>
#include <grafkit/render/texture.h>

#include <grafkit/core/log.h>

#include <grafkit_loader/asset_loader_system.h>

#include <grafkit/resource/image_builder.h>
#include <grafkit/resource/material_builder.h>
#include <grafkit/resource/scenegraph_builder.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

#include "cube.h"
#include "shaders/triangle.frag.h"
#include "shaders/triangle.vert.h"

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 768;

class HelloApplication : public Grafkit::Application
{
private:
	Grafkit::Asset::AssetLoaderPtr m_assetLoader;
	Grafkit::Resource::ResourceManagerPtr m_resources;

public:
	HelloApplication()
		: Grafkit::Application(WIDTH, HEIGHT, "Test application")
	{
		m_assetLoader = std::make_unique<Grafkit::Asset::JsonAssetLoader>();
		m_resources = std::make_unique<Grafkit::Resource::ResourceManager>(
			Grafkit::MakeReferenceAs<Grafkit::Asset::IAssetLoader>(*m_assetLoader));
	}

	~HelloApplication() = default;

	void Init() override
	{
	}

	void Update([[maybe_unused]] const Grafkit::TimeInfo &timeInfo) override
	{
	}

	void Render() override
	{
	}

	void Shutdown() override
	{
	}
};

int main()
{
	HelloApplication app;
	try
	{
		app.Run();
	}
	catch (const std::exception &e)
	{
		Grafkit::Core::Log::Instance().Error("Exception: %s", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
