#include "stdafx.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "grafkit/core/log.h"
#include "grafkit/core/window.h"

using namespace Grafkit::Core;

namespace {
	void ErrorCallback(int error, const char* description)
	{
		Log::Instance().Error("GLFW error: %d: %s", error, description);
	}
} // namespace

GLFWWindow::GLFWWindow(const WindowSize size,
	const std::string& title,
	const WindowMode mode,
	const WindowVsync vsync,
	const WindowResizable resizable)
{
	Init(size, title);
	SetVsync(vsync);
	SetWindowMode(mode);
	SetResizable(resizable);

	PollEvents();
}

GLFWWindow::~GLFWWindow()
{
	if (m_window) {
		// glfwSetWindowMonitor(m_window, nullptr, 0, 0, m_size.width, m_size.height, 0);
		glfwDestroyWindow(m_window);
	}
	glfwTerminate();
}

void GLFWWindow::Init(const WindowSize& size, const std::string& title)
{
	glfwSetErrorCallback(ErrorCallback);

	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(size.width, size.height, title.c_str(), nullptr, nullptr);
	if (!m_window) {
		glfwTerminate();
		throw std::runtime_error("Failed to create window");
	}

	glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);

	m_size = size;
}

void GLFWWindow::SetWindowMode(WindowMode windowMode)
{
	int xpos = 0, ypos = 0;
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	if (m_window) {
		switch (windowMode) {
		case WindowMode::Fullscreen:
			glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			break;
		case WindowMode::Windowed:
			glfwGetWindowPos(m_window, &xpos, &ypos);
			glfwSetWindowMonitor(m_window, nullptr, xpos, ypos, m_size.width, m_size.height, 0);
			glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);
			break;
		case WindowMode::Borderless:
			glfwGetWindowPos(m_window, &xpos, &ypos);
			glfwSetWindowMonitor(m_window, nullptr, 0, 0, m_size.width, m_size.height, 0);
			glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);
			break;
		}
	}
}

void GLFWWindow::SetVsync(WindowVsync vsync)
{
	if (m_window) {
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(vsync == WindowVsync::On ? 1 : 0);
	}
}

void GLFWWindow::SetResizable(WindowResizable resizable)
{
	if (m_window) {
		glfwWindowHint(GLFW_RESIZABLE, resizable == WindowResizable::On ? GLFW_TRUE : GLFW_FALSE);
	}
}

void GLFWWindow::Resize(const int width, const int height)
{
	if (m_window) {
		m_size = { width, height };
		glfwSetWindowSize(m_window, width, height);
	}
}

void GLFWWindow::Show(bool visible)
{
	if (m_window) {

		if (visible && !m_isVisible) {
			glfwShowWindow(m_window);
		} else if (!visible && m_isVisible) {
			glfwHideWindow(m_window);
		}
		m_isVisible = visible;
	}
}

void GLFWWindow::Focus()
{
	if (m_window) {
		glfwFocusWindow(m_window);
	}
}

void GLFWWindow::PollEvents() { glfwPollEvents(); }

bool GLFWWindow::IsClosing() const { return glfwWindowShouldClose(m_window); }

WindowSize GLFWWindow::GetBufferSize() const
{
	if (m_window) {
		WindowSize size;
		glfwGetFramebufferSize(m_window, &size.width, &size.height);
		return size;
	}

	return {};
}

VkSurfaceKHR GLFWWindow::CreateSurface(const VkInstance& instance) const
{
	if (m_window) {
		VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
		if (glfwCreateWindowSurface(instance, m_window, nullptr, &vkSurface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
		return vkSurface;
	}
	return VK_NULL_HANDLE;
}
