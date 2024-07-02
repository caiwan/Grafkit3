#ifndef GRAFKIT_CORE_WINDOW_H
#define GRAFKIT_CORE_WINDOW_H

#include <cstddef>
#include <string_view>
//
#include <grafkit/common.h>

using GLFWwindow = struct GLFWwindow;

namespace Grafkit::Core {
	class Device;
	class Instance;

	struct WindowSize {
		int width = 0;
		int height = 0;
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
		virtual void Show(bool visible) = 0;
		virtual void Focus() = 0;

		virtual void PollEvents() = 0;

		[[nodiscard]] virtual bool IsClosing() const = 0;
		[[nodiscard]] virtual WindowSize GetBufferSize() const = 0;
		[[nodiscard]] virtual VkSurfaceKHR CreateSurface(const VkInstance& instance) const = 0;
	};

	// ---

	class GKAPI GLFWWindow : public IWindow {
	public:
		GLFWWindow();

		explicit GLFWWindow(const WindowSize size,
			const std::string& title,
			const WindowMode mode = WindowMode::Windowed,
			const WindowVsync vsync = WindowVsync::On,
			const WindowResizable resizable = WindowResizable::On);

		~GLFWWindow() override;

		void SetWindowMode(WindowMode windowMode) final;
		void SetVsync(WindowVsync vsync) final;
		void SetResizable(WindowResizable resizable) final;
		void Resize(const int width, const int height) final;
		void Show(bool visible) final;
		void Focus() final;

		void PollEvents() final;

		[[nodiscard]] bool IsClosing() const final;
		[[nodiscard]] WindowSize GetBufferSize() const final;
		[[nodiscard]] VkSurfaceKHR CreateSurface(const VkInstance& instance) const final;

	private:
		void Init(const WindowSize& size, const std::string& title);
		GLFWwindow* m_window = nullptr;
		WindowSize m_size = {};
		bool m_isVisible = false;
	};
} // namespace Grafkit::Core

#endif // __GRAFKIT_CORE_WINDOW_H__
