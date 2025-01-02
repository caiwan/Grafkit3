#include <optional>
#include <string>
#include <vector>
//
#include <grafkit/application.h>
#include <grafkit/common.h>
#include <grafkit/core/buffer.h>
#include <grafkit/core/command_buffer.h>
#include <grafkit/core/pipeline.h>
#include <grafkit/core/window.h>
#include <grafkit/render.h>
#include <grafkit/render/material.h>
#include <grafkit/render/mesh.h>
#include <grafkit/render/render_graph.h>
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

#include "cube_mesh.h"
#include "shaders/triangle.frag.h"
#include "shaders/triangle.vert.h"

#include "shaders/quad.vert.h"
#include "shaders/red.frag.h"

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 768;

class HelloApplication : public Grafkit::Application
{
private:
	Grafkit::Asset::AssetLoaderPtr m_assetLoader;
	Grafkit::Resource::ResourceManagerPtr m_resources;

	Grafkit::Core::DescriptorSetPtr m_modelviewDescriptor;

	Grafkit::ScenegraphPtr m_sceneGraph;
	Grafkit::RenderGraphPtr m_renderGraph;

	struct
	{
		Grafkit::NodePtr rootNode;
		Grafkit::NodePtr centerNode;
		Grafkit::NodePtr leftNode;
		Grafkit::NodePtr rightNode;
		Grafkit::NodePtr frontNode;
		Grafkit::NodePtr rearNode;
		Grafkit::NodePtr topNode;
		Grafkit::NodePtr bottomNode;
	} m_nodes;

	Grafkit::Core::UniformBuffer<Grafkit::CameraView> m_ubo;

public:
	HelloApplication()
		: Grafkit::Application(WIDTH, HEIGHT, "Test application")
	{
		m_assetLoader = std::make_unique<Grafkit::Asset::JsonAssetLoader>();
		m_resources = std::make_unique<Grafkit::Resource::ResourceManager>(
			Grafkit::MakeReferenceAs<Grafkit::Asset::IAssetLoader>(*m_assetLoader));
	}

	~HelloApplication() override = default;

	void Init() override
	{
		const auto &device = m_renderContext->GetDevice();
		const auto resources = Grafkit::MakeReference(*m_resources);

		Grafkit::RenderStagePtr stage =
			Grafkit::RenderStageBuilder(m_renderContext->GetDevice())
				.SetRenderTarget(m_renderContext->GetRenderTarget())
				.SetVertexInputDescription(Grafkit::Vertex::GetVertexDescription())
				.AddDescriptorSetLayoutBindings(Grafkit::Material::GetLayoutBindings())
				.AddPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Grafkit::ModelView)})
				.SetVertexShader(triangle_vert, triangle_vert_len)
				.SetFragmentShader(triangle_frag, triangle_frag_len)
				.Build();

		m_renderGraph = std::make_shared<Grafkit::RenderGraph>();
		m_renderGraph->BuildFromStages({stage});

		Grafkit::Resource::CheckerImageDesc checkerImageDesc = {
			{256, 256, 1},
			{16, 16},
			{65, 105, 225, 255},
			{255, 165, 79, 255},
		};

		Grafkit::Core::ImagePtr image =
			Grafkit::Resource::CheckerImageBuilder(checkerImageDesc).BuildResource(device, resources);

		Grafkit::MaterialPtr material = //
			Grafkit::Resource::MaterialBuilder()
				.SetRenderStage(stage)
				.AddTextureImage(Grafkit::DIFFUSE_TEXTURE_BINDING, image)
				.BuildResource(device, resources);

		m_ubo = Grafkit::Core::UniformBuffer<Grafkit::CameraView>::CreateBuffer(device);
		m_modelviewDescriptor = stage->CreateDescriptorSet(Grafkit::CAMERA_VIEW_SET);
		m_modelviewDescriptor->Update(m_ubo.buffer, Grafkit::MODEL_VIEW_BINDING);

		Grafkit::MeshPtr mesh = //
			Grafkit::Resource::MeshBuilder()
				.AddPrimitive(TestApplication::vertices, TestApplication::indices, material)
				.BuildResource(device, resources);

		m_sceneGraph = std::make_shared<Grafkit::Scenegraph>();

		// Nodes
		m_nodes.rootNode = m_sceneGraph->CreateNode();
		m_nodes.centerNode = m_sceneGraph->CreateNode(mesh, m_nodes.rootNode);
		m_nodes.leftNode = m_sceneGraph->CreateNode(mesh, m_nodes.centerNode);
		m_nodes.rightNode = m_sceneGraph->CreateNode(mesh, m_nodes.centerNode);
		m_nodes.frontNode = m_sceneGraph->CreateNode(mesh, m_nodes.centerNode);
		m_nodes.rearNode = m_sceneGraph->CreateNode(mesh, m_nodes.centerNode);
		m_nodes.topNode = m_sceneGraph->CreateNode(mesh, m_nodes.centerNode);
		m_nodes.bottomNode = m_sceneGraph->CreateNode(mesh, m_nodes.centerNode);

		m_sceneGraph->AddDescriptorSet(Grafkit::CAMERA_VIEW_SET, m_modelviewDescriptor);
	}

	void Update([[maybe_unused]] const Grafkit::TimeInfo &timeInfo) override
	{
		m_ubo.data.projection = glm::perspective(glm::radians(45.0f), m_renderContext->GetAspectRatio(), 0.1f, 100.f);
		m_ubo.data.camera = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
		m_ubo.Update(m_renderContext->GetDevice(), m_renderContext->GetNextFrameIndex());

		m_nodes.rootNode->translation = glm::vec3(0.0f, 0.0f, 0.0f);
		m_nodes.centerNode->translation = glm::vec3(0.0f, 0.0f, 0.0f);
		m_nodes.leftNode->translation = glm::vec3(-3.0f, 0.0f, 0.0f);
		m_nodes.rightNode->translation = glm::vec3(3.0f, 0.0f, 0.0f);
		m_nodes.frontNode->translation = glm::vec3(0.0f, 0.0f, 3.0f);
		m_nodes.rearNode->translation = glm::vec3(0.0f, 0.0f, -3.0f);
		m_nodes.topNode->translation = glm::vec3(0.0f, 3.0f, 0.0f);
		m_nodes.bottomNode->translation = glm::vec3(0.0f, -3.0f, 0.0f);

		// Rotate the model
		m_nodes.centerNode->rotation = glm::quat(glm::vec3(glm::radians(30.0f) * timeInfo.time,
			glm::radians(45.0f) * timeInfo.time,
			glm::radians(60.0f) * timeInfo.time));

		m_nodes.leftNode->rotation = glm::quat(glm::vec3(glm::radians(.09f * 90.0f) * timeInfo.time,
			glm::radians(.75f * 90.0f) * timeInfo.time,
			glm::radians(.69f * 90.0f) * timeInfo.time));

		m_nodes.rightNode->rotation = glm::quat(glm::vec3(glm::radians(.66f * 90.0f) * timeInfo.time,
			glm::radians(.12f * 90.0f) * timeInfo.time,
			glm::radians(.15f * 90.0f) * timeInfo.time));

		m_nodes.frontNode->rotation = glm::quat(glm::vec3(glm::radians(.82f * 90.0f) * timeInfo.time,
			glm::radians(.71f * 90.0f) * timeInfo.time,
			glm::radians(.52f * 90.0f) * timeInfo.time));

		m_nodes.rearNode->rotation = glm::quat(glm::vec3(glm::radians(.28f * 90.0f) * timeInfo.time,
			glm::radians(.60f * 90.0f) * timeInfo.time,
			glm::radians(.43f * 90.0f) * timeInfo.time));

		m_nodes.topNode->rotation = glm::quat(glm::vec3(glm::radians(.81f * 90.0f) * timeInfo.time,
			glm::radians(.72f * 90.0f) * timeInfo.time,
			glm::radians(.26f * 90.0f) * timeInfo.time));

		m_nodes.bottomNode->rotation = glm::quat(glm::vec3(glm::radians(.84f * 90.0f) * timeInfo.time,
			glm::radians(.10f * 90.0f) * timeInfo.time,
			glm::radians(.55f * 90.0f) * timeInfo.time));

		m_sceneGraph->Update(timeInfo);
	}

	void Render() override
	{
		const auto &commandBuffer = m_renderContext->BeginCommandBuffer();
		const auto &frameIndex = m_renderContext->GetNextFrameIndex();

		m_renderGraph->Record(commandBuffer, frameIndex);

		m_renderContext->EndFrame(commandBuffer);
	}

	void Shutdown() override
	{
		m_ubo.Destroy(m_renderContext->GetDevice());
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
