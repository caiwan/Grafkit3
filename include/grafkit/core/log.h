#ifndef GRAFKIT_LOG_H
#define GRAFKIT_LOG_H

#include <cstdarg>
#include <memory>
#include <mutex>
#include <string>

// #if __cplusplus >= 202002L
// #include <format>
// #define USE_STD_FORMAT
// #endif

namespace Grafkit {
	namespace Core {

		class Logger {
		public:
			enum class LogLevel { Debug, Info, Warning, Error };

			virtual ~Logger() = default;

			template <typename... Args> void Debug(const std::string& message, Args&&... args) const
			{
				LogMessage(LogLevel::Debug, message, std::forward<Args>(args)...);
			};

			template <typename... Args> void Info(const std::string& message, Args&&... args) const
			{
				LogMessage(LogLevel::Info, message, std::forward<Args>(args)...);
			};

			template <typename... Args> void Warning(const std::string& message, Args&&... args) const
			{
				LogMessage(LogLevel::Warning, message, std::forward<Args>(args)...);
			};

			template <typename... Args> void Error(const std::string& message, Args&&... args) const
			{
				LogMessage(LogLevel::Error, message, std::forward<Args>(args)...);
			};

		protected:
			template <typename... Args>
			void LogMessage(const LogLevel level, const std::string& format, Args&&... args) const
			{
#if USE_STD_FORMAT
				std::string message = std::vformat(format, std::make_format_args(args)...);
#else
				std::string message = FormatString(format.c_str(), std::forward<Args>(args)...);
#endif
				Log(level, message);
			}
#if !USE_STD_FORMAT
			static std::string FormatString(const char* format, ...);
#endif

			virtual void Log(const LogLevel level, const std::string& message) const = 0;
		};

		class DefaultLogger : public Logger {
		protected:
			void Log(const LogLevel level, const std::string& message) const override;

		private:
			mutable std::mutex m_mutex;
		};

		class Log {
		public:
			Log(const Log&) = delete;
			Log& operator=(const Log&) = delete;

			static Logger& Instance()
			{
				std::lock_guard<std::mutex> lock(g_mutex);
				if (!g_logger) {
					g_logger.reset(new DefaultLogger());
				}
				return *g_logger;
			}

			static void setLogger(Logger* newLogger)
			{
				std::lock_guard<std::mutex> lock(g_mutex);
				g_logger.reset(newLogger);
			}

		private:
			Log() = default;

			static std::mutex g_mutex;
			static std::unique_ptr<Logger> g_logger;
		};

	} // namespace Core
} // namespace Grafkit

#endif // LOG_H
