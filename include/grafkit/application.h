#ifndef GRAFKIT_APPLICATION_H
#define GRAFKIT_APPLICATION_H

#include <grafkit/core/window.h>
#include <grafkit/render.h>

namespace Grafkit {

	class Application {
	public:
		Application();
		explicit Application(const int width, const int height, const std::string& windowTitle);
		virtual ~Application();
		void Run();

	protected:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(const TimeInfo& timeInfo) = 0;

		virtual void Render() = 0;

		std::unique_ptr<Grafkit::Core::IWindow> m_window;
		std::unique_ptr<Grafkit::RenderContext> m_renderContext;
	};
} // namespace Grafkit

#endif // __GRAFKIT_APPLICATION_H__
