#ifndef __GRAFKIT_APPLICATION_H__
#define __GRAFKIT_APPLICATION_H__

#include <grafkit/core/window.h>
#include <grafkit/render.h>

namespace Grafkit
{

	class Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();

	protected:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;
		virtual void Update() = 0;
		virtual void Compute(VkCommandBuffer &commandBuffer) = 0;
		virtual void Render(VkCommandBuffer &commandBuffer) = 0;

		Grafkit::Core::Window window;
		Grafkit::RenderContext renderContext;
	};
} // namespace Grafkit

#endif // __GRAFKIT_APPLICATION_H__
