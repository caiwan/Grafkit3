#include "hello_application.h"
#include "shaders/triangle.frag.h"
#include "shaders/triangle.vert.h"

using namespace PlayerApplication;

PlayerApplication::HelloApplication::HelloApplication() { }

PlayerApplication::HelloApplication::~HelloApplication() { }

void PlayerApplication::HelloApplication::Init()
{
	graphicsPipeline = renderContext.CreateGraphicsPipelineBuilder()
						   .AddVertexShader(triangle_vert, triangle_vert_len)
						   .AddFragmentShader(triangle_frag, triangle_frag_len)
						   .Build();
}

void PlayerApplication::HelloApplication::Update() { }

void PlayerApplication::HelloApplication::Compute(VkCommandBuffer& commandBuffer) { }

void PlayerApplication::HelloApplication::Render(VkCommandBuffer& commandBuffer)
{
	// ...
}

void PlayerApplication::HelloApplication::Shutdown() { }
