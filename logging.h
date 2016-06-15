#pragma once

#include "logstream.h"
#include <sstream>
namespace Logger_nsp
{
	class Logger
	{
	private:
		static const std::string prefix;
		static details::LogStream* stream;
		std::stringstream threadStr;
	public:
		enum LogLevel
		{
			UNKNOWN,
			TRACE,
			DEBUG,
			INFO,
			WARNING,
			ERROR,
			FATAL,
		};

		static void SetLevel(int level);
		static LogLevel GetLogLevel();
		static details::LogStream* GetStream();
		Logger();
		Logger(int line, const char* file, LogLevel level);
		Logger(int line, const char* file, const char* func, LogLevel level);

	};

#define LOG_TRACE\
	(*(Logger_nsp::Logger(__LINE__, __FILE__, __FUNCTION__, Logger_nsp::Logger::LogLevel::TRACE).GetStream()))
#define LOG_INFO\
	(*(Logger_nsp::Logger(__LINE__, __FILE__, __FUNCTION__, Logger_nsp::Logger::LogLevel::INFO).GetStream()))
#define LOG_DEBUG\
	(*(Logger_nsp::Logger(__LINE__, __FILE__,__FUNCTION__, Logger_nsp::Logger::LogLevel::DEBUG).GetStream()))
#define LOG_WARNING\
	(*(Logger_nsp::Logger(__LINE__, __FILE__, Logger_nsp::Logger::LogLevel::WARNING).GetStream()))
#define LOG_FATAL\
	(*(Logger_nsp::Logger(__LINE__, __FILE__, Logger_nsp::Logger::LogLevel::FATAL).GetStream()))
#define LOG_ERROR\
	(*(Logger_nsp::Logger(__LINE__, __FILE__, Logger_nsp::Logger::LogLevel::ERROR).GetStream()))
}