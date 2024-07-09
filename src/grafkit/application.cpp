#include <chrono>
#include <grafkit/application.h>
#include <grafkit/core/log.h>
#include <iostream>

using namespace Grafkit;

Application::Application()
{
	m_window = std::make_unique<Core::GLFWWindow>();
	m_renderContext = std::make_unique<RenderContext>(MakeReference(*m_window.get()));
}

Application::Application(const int width, const int height, const std::string& windowTitle)
{
	m_window = std::make_unique<Core::GLFWWindow>(Core::WindowSize({ width, height }),
		windowTitle,
		Core::WindowMode::Windowed,
		Core::WindowVsync::On,
		Core::WindowResizable::On);
	m_renderContext = std::make_unique<RenderContext>(MakeReference(*m_window.get()));
}

void Grafkit::Application::Run()
{
	Init();

	TimeInfo timeInfo {};

	double lastFrameTime = 0.0;
	double fpsTimer = 0.0;
	int frameCount = 0;

	while (!m_window->IsClosing()) {

		const auto startTime = std::chrono::steady_clock::now();

		m_window->PollEvents();

		if (!m_window->IsClosing()) {
			Update(timeInfo);
			const auto commandBuffer = m_renderContext->BeginCommandBuffer();
			Compute(commandBuffer);
			m_renderContext->BeginFrame(commandBuffer);
			Render(commandBuffer);
			m_renderContext->EndFrame(commandBuffer);
		}

		const auto endTime = std::chrono::steady_clock::now();

		std::chrono::duration<double> duration = endTime - startTime;
		lastFrameTime = duration.count();

		timeInfo.deltaTime = lastFrameTime;
		timeInfo.time += lastFrameTime;

		fpsTimer += lastFrameTime;
		frameCount++;

		if (fpsTimer >= 1.0) {
			Core::Log::Instance().Info("FPS: %d", frameCount);
			fpsTimer = 0.0;
			frameCount = 0;
		}
	}
	m_renderContext->Flush();
	Shutdown();
}

Application::~Application() = default;
