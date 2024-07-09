#include <grafkit/core/window.h>

#include <GLFW/glfw3.h>
#include <stdexcept>

using namespace Grafkit::Core;

GLFWWindow::GLFWWindow()
{
	Init({ DEFAULT_WIDTH, DEFAULT_HEIGHT }, DEFAULT_TITLE.data());
	SetWindowMode(WindowMode::Windowed);
	SetVsync(WindowVsync::On);
	SetResizable(WindowResizable::On);
}

GLFWWindow::GLFWWindow(const WindowSize size,
	const std::string& title,
	const WindowMode mode,
	const WindowVsync vsync,
	const WindowResizable resizable)
{
	Init(size, title);
	SetWindowMode(mode);
	SetVsync(vsync);
	SetResizable(resizable);
}

GLFWWindow::~GLFWWindow()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void GLFWWindow::Init(const WindowSize& size, const std::string& title)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(size.width, size.height, title.c_str(), nullptr, nullptr);
	if (!m_window) {
		throw std::runtime_error("Failed to create window");
	}
}

void GLFWWindow::SetWindowMode(WindowMode windowMode)
{
	int xpos = 0, ypos = 0;
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	switch (windowMode) {
	case WindowMode::Windowed:
		glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		break;
	case WindowMode::Fullscreen:
		glfwGetWindowPos(m_window, &xpos, &ypos);
		glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowMonitor(m_window, nullptr, xpos, ypos, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);
		break;
	case WindowMode::Borderless:
		glfwGetWindowPos(m_window, &xpos, &ypos);
		glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowMonitor(m_window, nullptr, xpos, ypos, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);
		break;
	}
}

void GLFWWindow::SetVsync(WindowVsync vsync) { glfwSwapInterval(vsync == WindowVsync::On ? 1 : 0); }

void GLFWWindow::SetResizable(WindowResizable resizable)
{
	glfwWindowHint(GLFW_RESIZABLE, resizable == WindowResizable::On ? GLFW_TRUE : GLFW_FALSE);
}

void GLFWWindow::Resize(const int width, const int height) { glfwSetWindowSize(m_window, width, height); }

void GLFWWindow::PollEvents() { glfwPollEvents(); }

bool GLFWWindow::IsClosing() const { return glfwWindowShouldClose(m_window); }

WindowSize GLFWWindow::GetBufferSize() const
{
	WindowSize size;
	glfwGetFramebufferSize(m_window, &size.width, &size.height);
	return size;
}

VkSurfaceKHR GLFWWindow::CreateSurface(const VkInstance& instance) const
{
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	glfwCreateWindowSurface(instance, m_window, nullptr, &vkSurface);
	return vkSurface;
}
