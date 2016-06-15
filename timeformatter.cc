#include "timeformatter.h"
#include <assert.h>
namespace Logger_nsp
{
	namespace details
	{
		const std::string TimeFormatter::strMap[42] = {
			"04", " ", "08", " ", "11", " ", " ", " ", "01", " ", " ", " ",
			"10", " ", "09", " ", " ", "07", " ", "06", " ", " ", "02", " ",
			" ", " ", " ", " ", " ", "03", " ", " ", " ", " ", " ", " ",
			"05", " ", " ", "12", " ", " "
		};

		TimeFormatter::TimeFormatter()
		{}
		unsigned TimeFormatter::hash(const char* str)
		{
			unsigned h = 0;
			while (*str != '\0')
			{
				h = h * 19 + *str;
				str++;
			}
			return h % 41;
		}

		std::string TimeFormatter::GetStr(const char* str)
		{
			int i = hash(str);
			return strMap[i];
		}

		std::string TimeFormatter::GetTimeStr()
		{
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			std::string timeStr = asctime(timeinfo);
			std::string prefix = GetStr(timeStr.substr(4, 3).c_str());
			std::string suffix = timeStr.substr(8, 11);
			char tmp[10] = { 0 };
			auto n = std::chrono::system_clock::now().time_since_epoch().count();
			auto msc = (n % 10000000) / 10000;
			Logger_nsp::details::Convert(tmp, msc);
			char temp[20] = { 0 };
#if defined(_WIN32) && defined(_MSC_VER) // using windows vc compiler
			size_t len = sprintf_s(temp, sizeof(temp), "%s-%s.%03s", prefix.c_str(), suffix.c_str(), tmp);
#elif defined(__GNUC__)	// using GCC compiler
			size_t len = snprintf(temp, sizeof(temp), "%s-%s.%03s", prefix.c_str(), suffix.c_str(), tmp);
#endif
			assert(len == 18);
			//printf("%s-%s.%03s\n", prefix.c_str(), suffix.c_str(), tmp);
			return temp;
		}
	}
}
