#ifndef GRAFKIT_CORE_WINDOW_H
#define GRAFKIT_CORE_WINDOW_H

#include <cstddef>
#include <string_view>
//
#include <grafkit/common.h>

typedef struct GLFWwindow GLFWwindow;

namespace Grafkit {
	class RenderContext;
}

namespace Grafkit::Core {
	class Device;
	class Instance;

	constexpr int DEFAULT_WIDTH = 800;
	constexpr int DEFAULT_HEIGHT = 600;
	constexpr std::string_view DEFAULT_TITLE("Grafkit Application");

	struct WindowBufferSize {
		int width;
		int height;
	};

	class GKAPI Window {
		friend class Grafkit::RenderContext;
		friend class Core::Device;
		friend class Core::Instance;

	public:
		Window();
		explicit Window(const int width,
			const int height,
			const char* title,
			bool fullscreen = false,
			bool vsync = true,
			bool resizable = true);

		virtual ~Window();

		void SetFullscreen(bool fullscreen);
		void SetVsync(bool vsync);
		void SetResizable(bool resizable);
		void SetSize(const size_t width, const size_t height);

		void PollEvents();
		[[nodiscard]] bool IsClosing() const;

		[[nodiscard]] WindowBufferSize GetBufferSize() const;

	protected:
		GLFWwindow* GetWindow() const { return window; }

	private:
		void Init(const int width, const int height, const char* title);
		GLFWwindow* window;
	};
} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_WINDOW_H__
