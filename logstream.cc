#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include "logstream.h"
#include <sstream>
#include <chrono>

namespace Logger_nsp
{
	namespace details
	{
		std::atomic<bool> LogStream::inProgress (false);
		std::atomic<short> LogStream::atomicCounter(0);
		
		const char digits[] = "9876543210123456789";
		static_assert(sizeof(digits) == 20, "digits bits not equal");
		static const char* zero = digits + 9;

		const char hexDigits[] = "0123456789ABCDEF";
		static_assert(sizeof(hexDigits) == 17, "digits bits not equal");

		//Efficient Conversion From Integer to String
		template <typename T>
		size_t Convert(char buf[], T value)
		{
			T data = value;
			char* ptr = buf;

			do 
			{
				int lsd = static_cast<int>(data % 10);
				*ptr++ = zero[lsd];
				data /= 10;
			} while (data != 0);

			if (value < 0)
			{
				*ptr++ = '-';
			}
			*ptr = '\0';
			std::reverse(buf, ptr);
			return ptr - buf;
		}

		size_t ConvertHex(char buf[], uintptr_t value)
		{
			uintptr_t data = value;
			char* ptr = buf;

			do 
			{
				int lsd = static_cast<int>(data % 16);
				*ptr++ = hexDigits[lsd];
				data /= 16;
			} while (data != 0);

			*ptr = '\0';
			std::reverse(buf, ptr);
			return ptr - buf;
		}

		template <typename T>
		void LogStream::FormatInteger(T value)
		{
			std::lock_guard<std::mutex>lk(lockMutex);
			if (!inProgress)
			{
				inProgress = true;
			}
			if (buffer.Avail() >= kMaxDigitlen)
			{
				size_t len = Convert(buffer.Current(), value);
				buffer.Increase(len);
			}
		}

		LogStream::Self& LogStream:: operator<<(short s)
		{
			*this << static_cast<int>(s);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(unsigned short us)
		{
			*this << static_cast<short>(us);
			return *this;
		}

		LogStream::Self& LogStream::operator<<(int t)
		{
			FormatInteger(t);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(unsigned t)
		{
			FormatInteger(t);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(long t)
		{
			FormatInteger(t);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(unsigned long t)
		{
			FormatInteger(t);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(long long t)
		{
			FormatInteger(t);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(unsigned long long t)
		{
			FormatInteger(t);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(float v)
		{
			*this << static_cast<double>(v);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(double v)
		{
			char temp[20] = { 0 };
			std::ostringstream str;
			sprintf_s(temp, sizeof(temp), "%.12f", v);
			str << temp;
			*this << str.str();
			
			return *this;
		}
		LogStream::Self& LogStream::operator<<(char v)
		{
			*this << (unsigned char)v;
			return *this;
		}
		LogStream::Self& LogStream::operator<<(unsigned char v)
		{
			std::lock_guard<std::mutex>guard(lockMutex);
			if (!inProgress)
			{
				inProgress = true;
			}
			buffer.Append((char*)&v, 1);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(const void*v)
		{
			std::lock_guard<std::mutex>guard(lockMutex);
			if (!inProgress)
			{
				inProgress = true;
			}
			uintptr_t value = reinterpret_cast<uintptr_t>(v);
			if (buffer.Avail() >= kMaxDigitlen)
			{
				char* ptr = const_cast<char*>(buffer.Current());
				ptr[0] = '0';
				ptr[1] = 'X';

				size_t len = ConvertHex(ptr + 2, value);
				buffer.Increase(len + 2);
			}
			return *this;
		}
		LogStream::Self& LogStream::operator<<(const char* v)
		{
			std::lock_guard<std::mutex>guard(lockMutex);
			if (!inProgress)
			{
				inProgress = true;
			}
			if (v != nullptr)
			{
				buffer.Append(v, strlen(v));
			}
			else
				buffer.Append("(NULL)", 6);
			return *this;
		}
		LogStream::Self& LogStream::operator<<(const std::string& v)
		{
			*this << v.c_str();
			return *this;
		}

		void LogStream::Submit()
		{
			static int counter = 0;
			size_t lenth = 0;
			//std::unique_lock<std::mutex>lk(util->GetMutex());
			std::unique_lock<std::mutex>lk(buffer.GetUtil()->GetMutex());
			{
				/*printf("提交数据\n");*/
				size_t index = buffer.GetSubmitIndex();
				const char* dataSource = buffer.GetData() + index;
				lenth = buffer.Length() - index;
				//printf("提交数据长度为 %d\n", lenth);
				if (lenth > 0)
				{

					if (lenth >= 65536)
					{
						lenth = buffer.GetUtil()->AvailSubmitBuffer() - 64;
					}
					index += lenth;
					//printf("util.fp 0x%x %d %s\n", buffer.GetUtil()->getfp(), __LINE__, __FILE__);
					char* dataDestination = (char*)buffer.GetUtil()->GetSubmitBuffer();
					char* origin = buffer.GetUtil()->GetOriginBuffer();
					
					/**
					 * 注意这里不能直接提交给写入线程，由于在密集写入的情况下，根据CPU调度的策略
					 * 此时几乎不会分给写入线程相应的时间片(主要是写入线程依靠信号量来通知)，而
					 * 提交线程则是每秒10次的速度提交,所以此时有大量的数据写入Fixbuffer的缓冲区，
					 * 而此时如果直接提交给写入线程，由于写入线程没有机会运行，所以很快会导致写入缓冲
					 * 溢出，这会直接导致程序退出。解决策略是提交之前判断写入缓冲有多少Available缓冲
					 * 区，若足够则直接进行写入，否则只是更新提交缓冲的index标志，缓冲数据留待下次
					 * 能够写入的时候再进行写入。
					 */
					
					if ((dataDestination + lenth) < (origin + 65536))
					{
						buffer.SetSubmitIndex(index);
						// lenth + datadestination > 65536;
						assert((dataDestination + lenth) < ( origin + 65536) &&  "Caution ! Buffer Out of range!");
						memcpy(dataDestination, dataSource, lenth);
						*(dataDestination + lenth) = '\0';
						if (index == buffer.Length() && buffer.Has_unSubmitBuf())
						{
							//printf("全部内容已提交\n");
							buffer.SetSubmitFinished(false);
							buffer.SetSubmitIndex(0);
							buffer.SetLastBufferLength(-1);
							buffer.Bzero();
							
							printf("全部提交完成，切换文件句柄\n");
							buffer.RollFile();
						}
					}
					else
					{
						outOfRange = true;
					}
				}
			}
			if (lenth > 0 && !outOfRange)
			{
				buffer.GetUtil()->SetSubmitSize(lenth);
			}
			lk.unlock();
			if (lenth > 0)
			{
				buffer.GetUtil()->NotifyMe();
			}
		}

		LogStream::LogStream(const std::string& fileName)
			:finished(false),
			outOfRange(false)
			//lastSubmitIndex(0)
		{
			buffer.SetBaseFileName(fileName);
			buffer.Start();
			/*char temp[10] = { 0 };
			Convert(temp, atomicCounter.operator short());
			const std::string name = fileName + "_" + temp +".log";*/
			/*util = new FileUtility::FileUtil(name);
			util->Start();*/
			StartCounter();
		}

		LogStream::~LogStream()
		{
			printf("LogStream 析构\n");
			std::this_thread::sleep_for(std::chrono::seconds(6));
			StopCounter();
			/*if (util != nullptr)
			{
				delete util;
				util = nullptr;
			}*/
		}

		void LogStream::Tick()
		{
			while (!finished)
			{
				if (!inProgress)
				{
					this->Submit();
				}
				std::unique_lock<std::mutex>lk(lockMutex);
				inProgress = false;
				lk.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		void LogStream::StartCounter()
		{
			try
			{
				counter = std::thread(&LogStream::Tick, this);
			}
			catch (...)
			{
				finished = true;
				throw;
			}
			
		}

		void LogStream::StopCounter()
		{
			finished = true;
			counter.join();
		}

	}
}
