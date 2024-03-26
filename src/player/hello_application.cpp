#include "hello_application.h"
#include "shaders/triangle.frag.h"
#include "shaders/triangle.vert.h"

using namespace PlayerApplication;

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 768;

PlayerApplication::HelloApplication::HelloApplication()
	: Application(WIDTH, HEIGHT, "Hello Application")
{
}

PlayerApplication::HelloApplication::~HelloApplication() { }

void PlayerApplication::HelloApplication::Init()
{

	// Setup vertices
	std::vector<Grafkit::Vertex> vertices {
		{ { -1.0f, +1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, /*{}, {}, {} */ },
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, /*{}, {}, {} */ },
		{ { +1.0f, +1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, /*{}, {}, {} */ },
		{ { +1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, /*{}, {}, {} */ },
	};

	// Setup indices
	std::vector<uint32_t> indices { 0, 1, 2, 2, 1, 3 };

	graphicsPipeline = renderContext.CreateGraphicsPipelineBuilder()
						   .AddVertexShader(triangle_vert, triangle_vert_len)
						   .AddFragmentShader(triangle_frag, triangle_frag_len)
						   .SetVertexInputDescription(Grafkit::Vertex::GetVertexDescription())
						   //.AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
						   .AddLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
						   .Build();

	mesh = std::make_shared<Grafkit::Mesh>(renderContext.GetDevice());
	mesh->Create(vertices, indices);
	mesh->AddPrimitive(0, static_cast<uint32_t>(indices.size()), 0, static_cast<uint32_t>(vertices.size()), nullptr);
}

void PlayerApplication::HelloApplication::Update() { }

void PlayerApplication::HelloApplication::Compute([[maybe_unused]] VkCommandBuffer& commandBuffer) { }

void PlayerApplication::HelloApplication::Render([[maybe_unused]] VkCommandBuffer& commandBuffer)
{
	graphicsPipeline->Bind(commandBuffer);
	mesh->Draw(commandBuffer);
}

void PlayerApplication::HelloApplication::Shutdown()
{
	mesh.reset();
	graphicsPipeline.reset();
}
