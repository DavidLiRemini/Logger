#include "logging.h"
#include "timeformatter.h"

namespace Logger_nsp
{

	details::LogStream* Logger::stream = nullptr;
	const std::string Logger::prefix = "[Level] [Time]             [Trid]  [Line]     [File]                     [Function]           [Text]\n";
	const char* LogLevelName[7] = {
		"UNKNOWN",
		"TRACE",
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"FATAL"
	};

	void Logger::SetLevel(int level)
	{

	}
	Logger::LogLevel Logger::GetLogLevel()
	{
		return Logger::INFO;
	}

	details::LogStream* Logger::GetStream()
	{
		if (stream == nullptr)
		{
			stream = new details::LogStream("TestRollLog");
			(*stream) << prefix;
		}
		return stream;
	}
	
	Logger::Logger() {

	}

	Logger::Logger(int line, const char* file, LogLevel level)
	{
		threadStr << std::this_thread::get_id();
		const char* levelName = LogLevelName[level];
		std::string timeStr = details::TimeFormatter().GetTimeStr();
		std::string threadId = threadStr.str();
		unsigned int lineNumber = line;
		const char* fileName = file;
		char temp[150] = { 0 };
#if defined(_WIN32) && defined(__MSC_VER)
		size_t len = sprintf_s(temp, sizeof(temp), "%-7s %s %-7s Line: %04d File: %-20s ", levelName, timeStr.c_str(), 
			threadId.c_str(), lineNumber, fileName);
#elif defined(__GNUC__)
		size_t len = snprintf(temp, sizeof(temp), "%-7s %s %-7s Line: %04d File: %-20s ", levelName, timeStr.c_str(),
			threadId.c_str(), lineNumber, fileName);
#endif
		if (stream)
		{
			(*stream) << temp;
		}
		threadStr.clear();
		threadStr.str("");
		//level time18 threadid5 __line__ __File__ tag text;
		//[Level] [time] [threadid] [line]4 [file]20 [func]20

	}

	Logger::Logger(int line, const char* file, const char* func, LogLevel level)
	{
		threadStr << std::this_thread::get_id();
		const char* levelName = LogLevelName[level];
		std::string timeStr = details::TimeFormatter().GetTimeStr();
		std::string threadId = threadStr.str();
		unsigned int lineNumber = line;
		const char* fileName = file;
		const char* funcName = func;
		char temp[150] = { 0 };
		size_t len = sprintf_s(temp, sizeof(temp), "%-7s %s %-7s Line: %04d File: %-20s Func: %-14s ", levelName, timeStr.c_str(), 
			threadId.c_str(), lineNumber, fileName, func);
		if (stream)
		{
			(*stream) << temp;
		}
		threadStr.clear();
		threadStr.str("");
	}
}