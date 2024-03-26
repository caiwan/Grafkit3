#include <grafkit/application.h>

using namespace Grafkit;

Application::Application()
	: window()
	, renderContext(window)
{
}

Application::Application(const int width, const int height, const std::string windowTitle)
	: window(width, height, windowTitle.c_str())
	, renderContext(window)
{
}

void Grafkit::Application::Run()
{
	Init();
	while (window.IsClosing() == false) {
		window.PollEvents();

		if (!window.IsClosing()) {
			Update();
			auto commandBuffer = renderContext.BeginCommandBuffer();
			Compute(commandBuffer);
			renderContext.BeginFrame(commandBuffer);
			Render(commandBuffer);
			renderContext.EndFrame(commandBuffer);
		}
	}
	renderContext.Flush();
	Shutdown();
}

Application::~Application() { }
