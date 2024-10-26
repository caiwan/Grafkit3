#include "grafkit/core/log.h"
#include "stdafx.h"
#include <iostream>

using namespace Grafkit::Core;

std::mutex Log::GMutex;
std::unique_ptr<Logger> Log::GLogger;

constexpr std::string_view TRACE_COLOR_CODE = "\033[1;90m";
constexpr std::string_view DEBUG_COLOR_CODE = "\033[1;30m";
constexpr std::string_view WARNING_COLOR_CODE = "\033[1;33m";
constexpr std::string_view ERROR_COLOR_CODE = "\033[1;31m";
constexpr std::string_view RESET_COLOR_CODE = "\033[0m";

constexpr size_t MAX_BUFFER_LENGTH = 4096;

void Grafkit::Core::DefaultLogger::Log(const LogLevel level, const std::string& message) const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	switch (level) {
	case LogLevel::Trace:
		std::cout << TRACE_COLOR_CODE << "TRACE: " << message << RESET_COLOR_CODE << std::endl;
		break;
	case LogLevel::Debug:
		std::cout << DEBUG_COLOR_CODE << "DEBUG: " << message << RESET_COLOR_CODE << std::endl;
		break;
	case LogLevel::Info:
		std::cout << "INFO: " << message << std::endl;
		break;
	case LogLevel::Warning:
		std::cout << WARNING_COLOR_CODE << "WARNING: " << message << RESET_COLOR_CODE << std::endl;
		break;
	case LogLevel::Error:
		std::cerr << ERROR_COLOR_CODE << "ERROR: " << message << RESET_COLOR_CODE << std::endl;
		break;
	}
}

#if !USE_STD_FORMAT
std::string Grafkit::Core::Logger::FormatString(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::vector<char> buffer(MAX_BUFFER_LENGTH);
	vsnprintf(buffer.data(), buffer.size(), format, args);
	va_end(args);
	return { buffer.data() };
}
#endif
