#include <grafkit/core/window.h>

#include <GLFW/glfw3.h>
#include <stdexcept>

using namespace Grafkit::Core;

Window::Window()
{
	Init(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_TITLE.data());
	SetFullscreen(false);
	SetVsync(true);
	SetResizable(true);
}

Window::Window(const int width, const int height, const char* title, bool fullscreen, bool vsync, bool resizable)
{
	Init(width, height, title);
	SetFullscreen(fullscreen);
	SetVsync(vsync);
	SetResizable(resizable);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::Init(const int width, const int height, const char* title)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window) {
		throw std::runtime_error("Failed to create window");
	}
}

void Window::SetFullscreen(bool fullscreen)
{
	if (fullscreen) {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
	} else {
		int xpos, ypos;
		glfwGetWindowPos(window, &xpos, &ypos);
		glfwSetWindowMonitor(window, nullptr, xpos, ypos, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);
	}
}

void Window::SetVsync(bool vsync) { glfwSwapInterval(vsync ? 1 : 0); }

void Window::SetResizable(bool resizable) { glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE); }

void Window::SetSize(const size_t width, const size_t height) { glfwSetWindowSize(window, width, height); }

void Window::PollEvents() { glfwPollEvents(); }

bool Window::IsClosing() const { return glfwWindowShouldClose(window); }

WindowBufferSize Window::GetBufferSize() const
{
	WindowBufferSize size;
	glfwGetFramebufferSize(window, &size.width, &size.height);
	return size;
}
