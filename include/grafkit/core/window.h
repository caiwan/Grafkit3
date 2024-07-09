#ifndef GRAFKIT_CORE_WINDOW_H
#define GRAFKIT_CORE_WINDOW_H

#include <cstddef>
#include <string_view>
//
#include <grafkit/common.h>

using GLFWwindow = struct GLFWwindow;

namespace Grafkit {
	class RenderContext;
}

namespace Grafkit::Core {
	class Device;
	class Instance;

	constexpr int DEFAULT_WIDTH = 800;
	constexpr int DEFAULT_HEIGHT = 600;
	constexpr std::string_view DEFAULT_TITLE("Grafkit Application");

	struct WindowSize {
		int width;
		int height;
	};

	enum class WindowMode { Windowed, Fullscreen, Borderless };
	enum class WindowVsync { Off, On };
	enum class WindowResizable { Off, On };

	GKAPI class IWindow {
	public:
		virtual ~IWindow() = default;

		virtual void SetWindowMode(WindowMode windowMode) = 0;
		virtual void SetVsync(WindowVsync vsync) = 0;
		virtual void SetResizable(WindowResizable resizable) = 0;
		virtual void Resize(const int width, const int height) = 0;

		virtual void PollEvents() = 0;

		[[nodiscard]] virtual bool IsClosing() const = 0;
		[[nodiscard]] virtual WindowSize GetBufferSize() const = 0;
		[[nodiscard]] virtual VkSurfaceKHR CreateSurface(const VkInstance& instance) const = 0;
	};

	// ---

	class GKAPI GLFWWindow : public IWindow {
		friend class Grafkit::RenderContext;
		friend class Core::Device;
		friend class Core::Instance;

	public:
		GLFWWindow();

		explicit GLFWWindow(const WindowSize size,
			const std::string& title = DEFAULT_TITLE.data(),
			const WindowMode mode = WindowMode::Windowed,
			const WindowVsync vsync = WindowVsync::On,
			const WindowResizable resizable = WindowResizable::On);

		~GLFWWindow() override;

		void SetWindowMode(WindowMode windowMode) override;
		void SetVsync(WindowVsync vsync) override;
		void SetResizable(WindowResizable resizable) override;
		void Resize(const int width, const int height) override;

		void PollEvents() override;

		[[nodiscard]] bool IsClosing() const override;
		[[nodiscard]] WindowSize GetBufferSize() const override;
		[[nodiscard]] VkSurfaceKHR CreateSurface(const VkInstance& instance) const override;

	private:
		void Init(const WindowSize& size, const std::string& title);
		GLFWwindow* m_window;
	};
} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_WINDOW_H__
