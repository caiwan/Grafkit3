#include <grafkit/application.h>

using namespace Grafkit;

Application::Application()
	: window()
	, renderContext(window)
{
}

void Grafkit::Application::Run()
{
	Init();
	while (window.IsClosing() == false)
	{
		window.PollEvents();
		Update();
		auto commandBuffer = renderContext.BeginCommandBuffer();
		Compute(commandBuffer);
		renderContext.BeginFrame(commandBuffer);
		Render(commandBuffer);
		renderContext.DrawFrame(commandBuffer);
	}
	Shutdown();
	// renderContext.Flush();
}

Application::~Application() { }
