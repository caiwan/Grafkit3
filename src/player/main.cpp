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
#include <grafkit/render/scenegraph.h>
#include <grafkit/render/texture.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <grafkit/resource/image_builder.h>
#include <grafkit/resource/material_builder.h>
#include <grafkit/resource/scenegraph_builder.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

#include "cube.h"
#include "shaders/triangle.frag.h"
#include "shaders/triangle.vert.h"

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 768;

class HelloApplication : public Grafkit::Application {
private:
	Grafkit::Core::DescriptorSetPtr m_materialDescriptor;
	Grafkit::Core::DescriptorSetPtr m_modelviewDescriptor;
	Grafkit::Core::PipelinePtr m_graphicsPipeline;

	Grafkit::ScenegraphPtr m_sceneGraph;

	struct {
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
	}

	virtual ~HelloApplication() = default;

	virtual void Init() override
	{
		// m_resources = std::make_unique<Grafkit::Resource::ResoureManger>(m_renderContext.GetDevice());

		m_materialDescriptor = m_renderContext.DescriptorBuilder()
								   .AddLayoutBindings(Grafkit::Material::GetLayoutBindings()[Grafkit::TEXTURE_SET])
								   .Build();

		m_modelviewDescriptor = m_renderContext.DescriptorBuilder()
									.AddLayoutBindings(Grafkit::Material::GetLayoutBindings()[Grafkit::CAMERA_VIEW_SET])
									.Build();

		m_graphicsPipeline = m_renderContext.PipelineBuilder()
								 .AddVertexShader(triangle_vert, triangle_vert_len)
								 .AddFragmentShader(triangle_frag, triangle_frag_len)
								 .SetVertexInputDescription(Grafkit::Vertex::GetVertexDescription())
								 .AddPushConstants(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Grafkit::ModelView))
								 .AddDescriptorSets(Grafkit::Material::GetLayoutBindings())
								 .Build();

		Grafkit::Core::ImagePtr image = Grafkit::Resource::CheckerImageBuilder({
																				   { 256, 256, 1 },
																				   { 16, 16 },
																				   { 65, 105, 225, 255 },
																				   { 255, 165, 79, 255 },
																			   })
											.BuildResource(m_renderContext.GetDevice());

		Grafkit::TexturePtr texture
			= Grafkit::Resource::TextureBuilder({}).SetImage(image).BuildResource(m_renderContext.GetDevice());

		Grafkit::MaterialPtr material = Grafkit::Resource::MaterialBuilder({})
											.SetPipeline(m_graphicsPipeline)
											.AddDescriptorSet(m_materialDescriptor, Grafkit::TEXTURE_SET)
											.AddDescriptorSet(m_modelviewDescriptor, Grafkit::CAMERA_VIEW_SET)
											.AddTexture(Grafkit::DIFFUSE_TEXTURE_BINDING, texture)
											.BuildResource(m_renderContext.GetDevice());

		m_ubo = Grafkit::Core::UniformBuffer<Grafkit::CameraView>::CreateBuffer(m_renderContext.GetDevice());
		m_modelviewDescriptor->Update(m_ubo.buffer, Grafkit::MODEL_VIEW_BINDING);

		Grafkit::MeshPtr mesh = Grafkit::Resource::MeshBuilder({
																   .primitives = { {
																	   .vertices = TestApplication::vertices,
																	   .indices = TestApplication::indices,
																	   .material = material,
																   } },
															   })
									.BuildResource(m_renderContext.GetDevice());

		m_sceneGraph = std::make_shared<Grafkit::Scenegraph>();
		m_sceneGraph->AddMesh(mesh);
		m_sceneGraph->AddMaterial(material);
		m_sceneGraph->AddTexture(texture);

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

	void Update([[maybe_unused]] const Grafkit::TimeInfo& timeInfo) override
	{
		m_ubo.data.projection = glm::perspective(glm::radians(45.0f), m_renderContext.GetAspectRatio(), 0.1f, 100.0f);
		m_ubo.data.camera = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
		m_ubo.Update(m_renderContext.GetDevice(), m_renderContext.GetNextFrameIndex());

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

	void Compute([[maybe_unused]] const Grafkit::Core::CommandBufferRef& commandBuffer) override { }
	void Render([[maybe_unused]] const Grafkit::Core::CommandBufferRef& commandBuffer) override
	{
		m_sceneGraph->Draw(commandBuffer);
		// throw std::runtime_error("Halt and catch fire");
	}

	virtual void Shutdown() override
	{
		m_sceneGraph.reset();
		m_materialDescriptor.reset();
		m_modelviewDescriptor.reset();
		m_ubo.Destroy(m_renderContext.GetDevice());
		m_graphicsPipeline.reset();
	}
};

int main()
{
	try {
		HelloApplication app;
		app.Run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
