#include "logging.h"
#include "logstream.h"
#include "timeformatter.h"
#include "logging.h"
#include "fileutility.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <typeinfo>
#include <chrono>
#include <vector>
#include <time.h>
#include <sstream>

//#include <boost/smart_ptr/shared_ptr.hpp>
//#include <boost/smart_ptr/make_shared_object.hpp>
//#include <boost/phoenix/bind.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/log/core.hpp>
//#include <boost/log/trivial.hpp>
//
//#include <boost/log/expressions.hpp>
//#include <boost/log/attributes.hpp>
//#include <boost/log/sources/basic_logger.hpp>
//#include <boost/log/sources/severity_logger.hpp>
//#include <boost/log/sources/severity_channel_logger.hpp>
//#include <boost/log/support/date_time.hpp>
//#include <boost/log/expressions/formatters/date_time.hpp>
//#include <boost/log/sources/record_ostream.hpp>
//#include <boost/log/sinks/sync_frontend.hpp>
//#include <boost/log/sinks/text_ostream_backend.hpp>
//#include <boost/log/attributes/scoped_attribute.hpp>
//#include <boost/log/utility/value_ref.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>


//namespace logging = boost::log;
//namespace src = boost::log::sources;
//namespace expr = boost::log::expressions;
//namespace sinks = boost::log::sinks;
//namespace attrs = boost::log::attributes;
//namespace keywords = boost::log::keywords;

// We define our own severity levels
//enum severity_level
//{
//	NORMAL,
//	NOTIFICATION,
//	WARNING,
//	ERROR,
//	CRITICAL
//};
//
//// The operator puts a human-friendly representation of the severity level to the stream
//std::ostream& operator<< (std::ostream& strm, severity_level level)
//{
//	static const char* strings[] =
//	{
//		"NORMAL",
//		"NOTIFICATION",
//		"WARNING",
//		"ERROR",
//		"CRITICAL"
//	};
//
//	if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
//		strm << strings[level];
//	else
//		strm << static_cast<int>(level);
//
//	return strm;
//}
//
////[ example_tutorial_filtering
//BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
//BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
//BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)
//
//void init()
//{
//	// Setup the common formatter for all sinks
//	/*logging::formatter fmt = expr::stream
//		<< std::setw(6) << std::setfill('0') << line_id << std::setfill(' ')
//		<< ": <" << severity << ">\t"
//		<< expr::if_(expr::has_attr(tag_attr))
//		[
//			expr::stream << "[" << tag_attr << "] "
//		]
//
//	<< expr::smessage << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S");*/
//	/*boost::log::formatter scope_formatter = boost::log::expressions::stream << "[" <<
//		boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") <<
//		"] [" << boost::log::expressions::attr<boost::log::attributes::current_process_id::value_type>("ProcessID") <<
//		"-" << boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID") << "] [" <<
//		boost::log::expressions::attr<boost::log::trivial::severity_level>("Severity") <<
//		"] " << boost::log::expressions::format_named_scope("Scope", boost::log::keywords::format = "%c[%F:%l] ",
//			boost::log::keywords::depth = 1) << boost::log::expressions::smessage;*/
//
//	logging::formatter fmt2 = expr::stream
//		<< std::setw(6) << std::setfill('0') << line_id << std::setfill(' ')
//		<< ": " << std::setiosflags(std::ios::left) << std::setw(14) << severity << "\t" << std::resetiosflags(std::ios::left)
//		<< expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%m-%d %H:%M:%S.%f") <<
//		expr::attr<attrs::current_process_id::value_type>("ProcessID");
//
//	// Initialize sinks
//	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
//	boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
//
//	sink->locked_backend()->add_stream(
//		boost::make_shared< std::ofstream >("full.log"));
//
//	sink->set_formatter(fmt2);
//
//	logging::core::get()->add_sink(sink);
//
//	sink = boost::make_shared< text_sink >();
//
//	sink->locked_backend()->add_stream(
//		boost::make_shared< std::ofstream >("important.log"));
//
//	sink->set_formatter(fmt2);
//
//	sink->set_filter(severity >= WARNING || (expr::has_attr(tag_attr) && tag_attr == "IMPORTANT_MESSAGE"));
//
//	logging::core::get()->add_sink(sink);
//	// Add attributes
//	logging::add_common_attributes();
//}
////]
//
//#if 0
//
////[ example_tutorial_filtering_bind
//bool my_filter(logging::value_ref< severity_level, tag::severity > const& level,
//	logging::value_ref< std::string, tag::tag_attr > const& tag)
//{
//	return level >= warning || tag == "IMPORTANT_MESSAGE";
//}
//
//void init()
//{
//	//<-
//
//	// Setup the common formatter for all sinks
//	logging::formatter fmt = expr::stream
//		<< std::setw(6) << std::setfill('0') << line_id << std::setfill(' ')
//		<< ": <" << severity << ">\t"
//		<< expr::if_(expr::has_attr(tag_attr))
//		[
//			expr::stream << "[" << tag_attr << "] "
//		]
//	<< expr::smessage;
//
//	// Initialize sinks
//	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
//	boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
//
//	sink->locked_backend()->add_stream(
//		boost::make_shared< std::ofstream >("full.log"));
//
//	sink->set_formatter(fmt);
//
//	logging::core::get()->add_sink(sink);
//
//	sink = boost::make_shared< text_sink >();
//
//	sink->locked_backend()->add_stream(
//		boost::make_shared< std::ofstream >("important.log"));
//
//	sink->set_formatter(fmt);
//
//	//->
//	// ...
//
//	namespace phoenix = boost::phoenix;
//	sink->set_filter(phoenix::bind(&my_filter, severity.or_none(), tag_attr.or_none()));
//
//	// ...
//	//<-
//
//	logging::core::get()->add_sink(sink);
//
//	// Add attributes
//	logging::add_common_attributes();
//
//	//->
//}
////]
//
//#endif
//void time_logging();
//void logging_function()
//{
//	src::severity_logger< severity_level > slg;
//	BOOST_LOG_SEV(slg, NORMAL) << "starting to time nested function";
//	time_logging();
//	BOOST_LOG_SEV(slg, NORMAL) << "stopping to time nested function";
//
//	
//}
//
//void time_logging()
//{
//	src::severity_logger<severity_level>slg;
//	BOOST_LOG_SEV(slg, NORMAL) << "A regular message";
//	BOOST_LOG_SEV(slg, WARNING) << "Something bad is going on but I can handle it";
//	BOOST_LOG_SEV(slg, CRITICAL) << "Everything crumbles, shoot me now!";
//
//	{
//		BOOST_LOG_SCOPED_THREAD_TAG("Tag", "IMPORTANT_MESSAGE");
//		BOOST_LOG_SEV(slg, NORMAL) << "An important message";
//	}
//}

//class Clock {
//public:
//	typedef an arithmetic - like type        rep;
//	typedef an instantiation of ratio      period;
//	typedef boost::chrono::duration<rep, period> duration;
//	typedef boost::chrono::time_point<Clock>     time_point;
//	static constexpr bool is_steady = true or false;
//
//	static time_point now();
//};

#define PRINT(X)\
	printf("Line: %d File: %s\n", __LINE__, __FILE__);
struct Time_val
{
	time_t seconds;
	int64_t microseconds;
};

const char* monthName[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


static Logger_nsp::details::LogStream* stream = Logger_nsp::Logger::GetStream();

//void LogErrorTest()
//{
//	for (auto it : monthName)
//	{
//		LOG_ERROR << "month Name " << it << "\n";
//	}
//}
void LogTraceTest()
{
	int i = 19;
	LOG_TRACE << "This is a LogTraceTest " <<
		"Here variable i.value= " << i << " address is: " << &i << "\n";
}
void LogDebugTest()
{
	double debugvariable = 8.99999;
	LOG_DEBUG << "This is a LogDebug Test "
		<< "double variable's value is: " << debugvariable << "\n";
}

void LogWarningTest()
{
	std::vector<int>v;
	for (int i = 0; i < 20; i++)
	{
		v.push_back(i + 19);
		LOG_INFO << "v.member: " << (i + 19) << "\n";
	}
	LOG_INFO << "\n";

	int* p = new int(20);
	LOG_WARNING << "Here I use new operator to generator a integer " <<
		"value is: " << *p << " You should caution memory leaks\n";
}

void LogFatalTest()
{
	std::vector<std::string>str;
	for (auto it : monthName)
	{
		str.push_back(it);
	}
	for (auto t : str)
	{
		LOG_FATAL << "Month Name " << t << " address " << &t << "\n";
	}
}
int main(int, char*[])
{
	/*std::ofstream filestr("test");
	const std::string fname = "[Level] [Time]             [Thid]  [Line]     [File]                     [Function]           [Text]";
	filestr << fname;
	filestr.close();
	printf("%-9s %-20s %-7s %-6s %-22s %-16s %s\n", "[Level]", "[Time]", "[ThreadId]", "[Line]", "[File]", "[Function]","[Text]");
	std::atomic<short>val = 19;
	char buf[10] = { 0 };
	Logger_nsp::details::Convert(buf, val.operator short());
	auto id = std::this_thread::get_id();
	std::stringstream stream;
	stream << id;
	std::string strid = stream.str();
	const char* levelName = "TRACE";
	std::string timeStr = Logger_nsp::details::TimeFormatter().GetTimeStr();
	unsigned int lineNumber = __LINE__;
	const char* fileName = __FILE__;
	const char* func = __FUNCTION__;
	char temp[120] = { 0 };

	size_t len = sprintf_s(temp, sizeof(temp), "[%7s] %s  [%-5s] [Line]: %04d [File]: %-20s [Func]: %-14s ", 
		levelName, timeStr.c_str(),strid.c_str(), lineNumber, fileName, func);
	Logger_nsp::details::TimeFormatter fmt;
	double db = 3.658965;
	stream.clear();
	stream.str("");
	stream << "this";
	auto s = stream.str();*/
	LOG_INFO << "Log Test beginning \n";
	for (int i = 0; i <500; i++)
	{
		//LogErrorTest();
		double db = 6.589;
		LOG_DEBUG << "db is: " << db << "\n";
		float fx = 10.6f;
		//LOG__ERROR << "this";
		//LOG_ERROR << "Caution float can safe convert to double, double can not convert to float safely\n";
		LogTraceTest();
		LogDebugTest();
		LogWarningTest();
		LogFatalTest();
		//printf("Log 次数: %d\n", i);
		//Sleep(1000);
	}

	//Sleep(3000);
	for (int j = 0; j < 100; j++)
	{
		LogFatalTest();
	}

	LOG_INFO << "Log Test end\n";
	//LOG_TRACE << "Log trace Test!\n";

	//LOG_FATAL << "start logging\n";
	//int x = 10;
	//LOG_FATAL << "x.address is " << &x << "\n";
	//for (int i = 0; i < 100; i++)
	//{
	//	LOG_FATAL << "Temporary variable: " << i << "\n";
	//}
	//LOG_FATAL << "Finished logging\n";
	/*LOG_INFO << "this is just a test\n";
	int x = 10;
	LOG_INFO << "x.address is " << &x << "\n";
	LOG_INFO << "finished logging";*/
	/*printf("%s\n", __FUNCTION__);
	Logger_nsp::FileUtility::FileUtil file1("newfile");
	const char* sen = "this is just a test\n";
	for (int i = 0; i < 200; i++)
	{
		file1.Append(sen, strlen(sen));
	}
	file1.Flush();*/
	/*int z = 10;
	LOG_INFO << "this is just a address test " << &z;
	std::ostringstream str;
	double x = 1.6;
	char temp[20] = { 0 };
	size_t len = sprintf_s(temp, sizeof(temp), "%.12f", x);
	str << temp;
	std::string s = str.str();
	char* pPath = ::getenv("Path");
	const char* p = "1";
	PRINT(p);*/
	//printf("%s\n", pPath);
	//printf("%03s\n", p);
	//TimeFormatter fmt;
	//for (int i = 0; i < 400; i++)
	//{
	//	fmt.init();
	//	Sleep(1000);
	//}
	//return 0;
	delete ::stream;
	/*int z;
	std::cin >> z;*/
}
