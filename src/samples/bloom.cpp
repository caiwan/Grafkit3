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

#include "cube_mesh.h"
#include "shaders/forward_render.frag.h"
#include "shaders/forward_render.vert.h"
#include "shaders/gaussblur.frag.h"
#include "shaders/quad.vert.h"
#include "shaders/red.frag.h"

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 768;

constexpr uint32_t FORWARD_PIPELINE_DESCRIPTOR = 0;
constexpr uint32_t GUAUSS_BLUR_PIPELINE_DESCRIPTOR = 1;

// constexpr uint32_t INPUT_SAMPLER_SET = 0;
// constexpr uint32_t IMAGE_SAMPLER_BINDING = 0;

struct BlurParams
{
	float blurScale;
	float blurStrength;

	static std::vector<Grafkit::Core::DescriptorSetLayoutBinding> GetLayoutBindings()
	{
		return {
			{
				// INPUT_SAMPLER_SET,
				// {
				// 	{
				// 		IMAGE_SAMPLER_BINDING,
				// 		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				// 		VK_SHADER_STAGE_FRAGMENT_BIT,
				// 	},
				// },
			},
		};
	}
};

class HelloApplication : public Grafkit::Application
{

public:
private:
	Grafkit::Core::DescriptorSetPtr m_materialDescriptor;
	Grafkit::Core::DescriptorSetPtr m_modelviewDescriptor;
	Grafkit::Core::PipelinePtr m_forwardRender;
	Grafkit::Core::PipelinePtr m_gaussBlur;

	Grafkit::Asset::AssetLoaderPtr m_assetLoader;
	Grafkit::Resource::ResourceManagerPtr m_resources;

	Grafkit::ScenegraphPtr m_sceneGraph;

	Grafkit::FullScreenQuadPtr m_fullscreenQuad;
	Grafkit::Core::RenderTargetPtr m_forwardRenderTarget;
	Grafkit::Core::RenderTargetPtr m_gaussBlurTarget;

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

	// BlurParams m_blurParams;

	Grafkit::Core::UniformBuffer<Grafkit::CameraView> m_ubo;

public:
	HelloApplication() : Grafkit::Application(WIDTH, HEIGHT, "Test application")
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

		// MARK: Fullscreen quad setup
		m_fullscreenQuad = std::make_shared<Grafkit::FullScreenQuad>(device);
		m_fullscreenQuad->Create();

		// MARK: PostFx setup
		m_forwardRenderTarget = Grafkit::Core::RenderTargetBuilder(m_renderContext->GetDevice())
									.SetSize(m_renderContext->GetExtent())
									.AddAttachment(VK_FORMAT_R8G8B8A8_UNORM,
										VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
									.AddAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
									.Build();

		m_gaussBlurTarget = Grafkit::Core::RenderTargetBuilder(m_renderContext->GetDevice())
								.SetSize(m_renderContext->GetExtent())
								.AddAttachment(VK_FORMAT_R8G8B8A8_UNORM,
									VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
								.Build();

		// Descriptor setup
		m_materialDescriptor = m_renderContext->DescriptorBuilder()
								   .AddLayoutBindings(Grafkit::Material::GetLayoutBindings()[Grafkit::TEXTURE_SET])
								   .Build();

		m_modelviewDescriptor = m_renderContext->DescriptorBuilder()
									.AddLayoutBindings(Grafkit::Material::GetLayoutBindings()[Grafkit::CAMERA_VIEW_SET])
									.Build();

		m_renderContext->AddStaticPipelineDescriptor(GUAUSS_BLUR_PIPELINE_DESCRIPTOR,
			Grafkit::Core::PipelineDescriptor{
				Grafkit::FullScreenQuad::Vertex::GetVertexDescription(),
				BlurParams::GetLayoutBindings(),
				{
					// { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(m_blurParams) },
				},
			});

		m_renderContext->AddStaticPipelineDescriptor(FORWARD_PIPELINE_DESCRIPTOR,
			Grafkit::Core::PipelineDescriptor{
				Grafkit::Vertex::GetVertexDescription(),
				Grafkit::Material::GetLayoutBindings(),
				{
					{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Grafkit::ModelView)},
				},
			});

		// Pipelines
		m_forwardRender =
			m_renderContext->PipelineBuilder(
							   FORWARD_PIPELINE_DESCRIPTOR, Grafkit::MakeReference(*m_forwardRenderTarget))
				.AddVertexShader(forward_render_vert, forward_render_vert_len)
				.AddFragmentShader(forward_render_frag, forward_render_frag_len)
				.Build();

		m_gaussBlur = m_renderContext->PipelineBuilder(FORWARD_PIPELINE_DESCRIPTOR)
						  .AddVertexShader(quad_vert, quad_vert_len)
						  //   .AddFragmentShader(gaussblur_frag, gaussblur_frag_len)
						  .AddFragmentShader(red_frag, red_frag_len)
						  .Build();

		Grafkit::Core::ImagePtr image = Grafkit::Resource::CheckerImageBuilder({
																				   .size = {256, 256, 1},
																				   .divisions = {16, 16},
																				   .color1 = {65, 105, 225, 255},
																				   .color2 = {255, 165, 79, 255},
																			   })
											.BuildResource(device, resources);

		Grafkit::MaterialPtr material = Grafkit::Resource::MaterialBuilder({})
											.SetPipeline(m_forwardRender)
											.AddDescriptorSet(m_materialDescriptor, Grafkit::TEXTURE_SET)
											.AddDescriptorSet(m_modelviewDescriptor, Grafkit::CAMERA_VIEW_SET)
											.AddTextureImage(Grafkit::DIFFUSE_TEXTURE_BINDING, image)
											.BuildResource(device, resources);

		m_ubo = Grafkit::Core::UniformBuffer<Grafkit::CameraView>::CreateBuffer(device);
		m_modelviewDescriptor->Update(m_ubo.buffer, Grafkit::MODEL_VIEW_BINDING);

		Grafkit::MeshPtr mesh = Grafkit::Resource::MeshBuilder()
									.AddPrimitive(TestApplication::vertices, TestApplication::indices, 0)
									.AddMaterial(0, material)
									.BuildResource(device, resources);

		m_sceneGraph = std::make_shared<Grafkit::Scenegraph>();
		m_sceneGraph->AddMesh(mesh);
		m_sceneGraph->AddMaterial(material);

		// Nodes
		m_nodes.rootNode = m_sceneGraph->CreateNode();
		m_nodes.centerNode = m_sceneGraph->CreateNode(m_nodes.rootNode, mesh);
		m_nodes.leftNode = m_sceneGraph->CreateNode(m_nodes.centerNode, mesh);
		m_nodes.rightNode = m_sceneGraph->CreateNode(m_nodes.centerNode, mesh);
		m_nodes.frontNode = m_sceneGraph->CreateNode(m_nodes.centerNode, mesh);
		m_nodes.rearNode = m_sceneGraph->CreateNode(m_nodes.centerNode, mesh);
		m_nodes.topNode = m_sceneGraph->CreateNode(m_nodes.centerNode, mesh);
		m_nodes.bottomNode = m_sceneGraph->CreateNode(m_nodes.centerNode, mesh);
	}

	void Update([[maybe_unused]] const Grafkit::TimeInfo &timeInfo) override
	{
		m_ubo.data.projection = glm::perspective(glm::radians(45.0f), m_renderContext->GetAspectRatio(), 0.1f, 100.0f);
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

		m_nodes.leftNode->rotation = glm::quat(glm::vec3(glm::radians(.5f * 30.0f) * timeInfo.time,
			glm::radians(.5f * 45.0f) * timeInfo.time,
			glm::radians(.5f * 60.0f) * timeInfo.time));

		m_nodes.rightNode->rotation = glm::quat(glm::vec3(glm::radians(.5f * 30.0f) * timeInfo.time,
			glm::radians(.5f * 45.0f) * timeInfo.time,
			glm::radians(.5f * 60.0f) * timeInfo.time));

		m_nodes.frontNode->rotation = glm::quat(glm::vec3(glm::radians(.5f * 30.0f) * timeInfo.time,
			glm::radians(.5f * 45.0f) * timeInfo.time,
			glm::radians(.5f * 60.0f) * timeInfo.time));

		m_nodes.rearNode->rotation = glm::quat(glm::vec3(glm::radians(.5f * 30.0f) * timeInfo.time,
			glm::radians(.5f * 45.0f) * timeInfo.time,
			glm::radians(.5f * 60.0f) * timeInfo.time));

		m_nodes.topNode->rotation = glm::quat(glm::vec3(glm::radians(.5f * 30.0f) * timeInfo.time,
			glm::radians(.5f * 45.0f) * timeInfo.time,
			glm::radians(.5f * 60.0f) * timeInfo.time));

		m_nodes.bottomNode->rotation = glm::quat(glm::vec3(glm::radians(.5f * 30.0f) * timeInfo.time,
			glm::radians(.5f * 45.0f) * timeInfo.time,
			glm::radians(.5f * 60.0f) * timeInfo.time));

		m_sceneGraph->Update(timeInfo);
	}

	void Render() override
	{
		const auto commandBuffer = m_renderContext->BeginCommandBuffer();

		m_forwardRenderTarget->BeginRenderPass(commandBuffer);
		m_sceneGraph->Draw(commandBuffer);
		m_forwardRenderTarget->EndRenderPass(commandBuffer);

		// ... and present
		// + add VBlur + HBlur + Present

		m_renderContext->BeginFrame(commandBuffer);
		m_gaussBlur->Bind(**commandBuffer);
		m_fullscreenQuad->Bind(commandBuffer);
		m_fullscreenQuad->Draw(commandBuffer);
		m_renderContext->EndFrame(commandBuffer);
	}

	void Shutdown() override
	{
		m_fullscreenQuad.reset();
		m_sceneGraph.reset();
		m_materialDescriptor.reset();
		m_modelviewDescriptor.reset();
		m_ubo.Destroy(m_renderContext->GetDevice());
		m_forwardRender.reset();
		m_gaussBlur.reset();
		m_forwardRenderTarget.reset();
		m_gaussBlurTarget.reset();
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
