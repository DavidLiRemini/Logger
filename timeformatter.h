#pragma once
#include "logstream.h"
#include <time.h>
#include <string>
#include <chrono>

namespace Logger_nsp
{
	namespace details
	{
		class TimeFormatter
		{
		private:
			static const std::string strMap[42];
			static unsigned hash(const char* str);
			time_t rawtime;
			struct tm * timeinfo = nullptr;

		public:
			TimeFormatter();
			//************************************
			// @Method:    GetStr
			// @Returns:   std::string
			// @Parameter: str
			// @Brief:	内部获取hashCode的方法
			//************************************
			static std::string GetStr(const char* str);
			//************************************
			// @Method:    GetTimeStr
			// @Returns:   std::string
			// @Brief:	获取当前时间字串
			//************************************
			std::string GetTimeStr();
		};
	}
}