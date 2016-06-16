#pragma once

#include "fileutility.h"
#include <string.h>
#include <string>
#define CC_DYNAMIC_CONVERSION(x)\
	char* address = nullptr;\
	if (dynamic_cast<SmallInternalBuf*>(x))\
	{\
		address = dynamic_cast<SmallInternalBuf*>(x)->data;\
	}\
	else\
	{\
		address = dynamic_cast<LargeInternalBuf*>(x)->data;\
	}
namespace Logger_nsp
{
	namespace details
	{
		const int kSmallBuffer = 4096;
		const int kLargeBuffer = 4096 * 1024;
		//************************************
		// @Method:    ConvertHex
		// @Returns:   size_t
		// @Parameter: buf
		// @Parameter: value
		// @Brief:	Efficient way convert unsigned to Hex string
		//************************************
		size_t ConvertHex(char buf[], uintptr_t value);
		//************************************
		// @Method:    Convert
		// @Returns:   size_t
		// @Parameter: buf
		// @Parameter: value
		// @Brief:	Efficient way convert integer to Decimal string
		//************************************
		template <typename T>
		size_t Convert(char buf[], T value);
		template <int SIZE>
		class FixBuffer;

		template <int SIZE>
		char* GetActualTypeDataSource(typename FixBuffer<SIZE>::InternalBuf* ptr)
		{
			char* retValue = nullptr;
			if (dynamic_cast<typename FixBuffer<SIZE>::SmallInternalBuf*>(ptr))
			{
				retValue = dynamic_cast<typename FixBuffer<SIZE>::SmallInternalBuf*>(ptr)->data;
			}
			else
				retValue = dynamic_cast<typename FixBuffer<SIZE>::LargeInternalBuf*>(ptr)->data;
			return retValue;
		}
		template <int SIZE>
		class FixBuffer
		{
		private:
			/**
			 *
			 * @Brief:	内部缓冲结构
			 */
		struct InternalBuf
			{
				InternalBuf* prev;
				InternalBuf* next;
				InternalBuf()
					:prev(nullptr),
					next(nullptr)
				{
				}
				virtual ~InternalBuf(){}
			};

			struct SmallInternalBuf : public InternalBuf
			{
			public:
				char data[SIZE];
				SmallInternalBuf()
					:InternalBuf()
				{
					memset(data, 0, SIZE);
				}
				
			};
			struct LargeInternalBuf : public InternalBuf
			{
			public:
				char data[2 * SIZE];
				LargeInternalBuf()
					:InternalBuf()
				{
					memset(data, 0, 2 * SIZE);
				}
			};
			template <int SZ>
			friend char* GetActualTypeDataSource(typename FixBuffer<SZ>::InternalBuf* ptr);
			//
	private:
			std::string baseFileName;
			InternalBuf* currentBufPtr;
			SmallInternalBuf firstBuffer;
			SmallInternalBuf secondBuffer;
			LargeInternalBuf backBuf;
			char* cur;
			bool hasUnSubmitBuf;
			bool stillHasUnsubmitBuf = false;
			bool outOfRange = false;
			bool switchHandler = false;
			size_t lastSubmitIndex;
			int firstBufferLength;
			int secondBufferLength;
			void InitBuf()
			{
				firstBuffer.next = &secondBuffer;
				secondBuffer.next = &backBuf;
				backBuf.next = &firstBuffer;

				backBuf.prev = &secondBuffer;
				secondBuffer.prev = &firstBuffer;
				firstBuffer.prev = &backBuf;
			}

			char* ConvertToActualTypeAndGetSource()const
			{
				char* retValue = nullptr;
				if (dynamic_cast<SmallInternalBuf*>(currentBufPtr))
				{
					retValue = dynamic_cast<SmallInternalBuf*>(currentBufPtr)->data;
				}
				else
					retValue = dynamic_cast<LargeInternalBuf*>(currentBufPtr)->data;
				return retValue;

			}
			char* end()const
			{
				CC_DYNAMIC_CONVERSION(currentBufPtr);
				if (address == backBuf.data)
				{
					return const_cast<char*>(address + 2 * SIZE);
				}
				else
					return const_cast<char*>(address + SIZE);
			}

			void SetRollHandlerOrNot(bool state)
			{
				switchHandler = state;
			}
		private:
			static std::atomic<short>atomicCounter;
			FileUtility::FileUtil* util;
			std::mutex lockMutex;
		public:
			FixBuffer()
				:currentBufPtr(nullptr),
				cur(nullptr),
				hasUnSubmitBuf(false),
				stillHasUnsubmitBuf(false),
				outOfRange(false),
				switchHandler(false),
				firstBufferLength(-1),
				secondBufferLength(-1),
				util(nullptr)
			{
				currentBufPtr = &firstBuffer;
				cur = firstBuffer.data;
				InitBuf();
			}

			~FixBuffer() {
				//
				//printf("util.fp 0x%x %d %s\n", util->getfp(), __LINE__, __FILE__);
				if (util != nullptr)
				{
					delete util;
					util = nullptr;
				}
			}
			//************************************
			// @Method:    Start
			// @Returns:   void
			// @Brief:	启动整个线程任务
			//************************************
			void Start()
			{
				char temp[10] = { 0 };
				details::Convert(temp, atomicCounter.operator short());
				++atomicCounter;
				const std::string fileName = baseFileName + "_" + temp + ".log";
				util = new FileUtility::FileUtil(fileName);
				util->Start();
				//printf("util.fp 0x%x %d %s\n", util->getfp(), __LINE__, __FILE__);
			}
			
			
			//************************************
			// @Method:    SetBaseName
			// @Returns:   void
			// @Parameter: bName
			// @Brief:	设置文件基名
			//************************************
			void SetBaseName(std::string& bName)
			{
				baseFileName = bName;
			}
			//************************************
			// @Method:    Append
			// @Returns:   void
			// @Parameter: buf
			// @Parameter: len
			// @Brief:	追加记录
			//************************************
			void Append(const char* buf, size_t len)
			{
				//do something
				//printf("util.fp 0x%x %d %s\n", util->getfp(), __LINE__, __FILE__);
				std::lock_guard<std::mutex>lk(util->GetMutex());
				/*printf("lastindex是: %d 追加内容为: %s\n",lastSubmitIndex, buf);
				printf("lastBufferLength 是: %d\n", lastBufferLength);*/
				if (Avail() >= len)
				{
					memcpy(cur, buf, len);
					cur += len;
				}
				else //双缓冲切换
				{
					if (lastSubmitIndex != Length() && outOfRange)
					{
						printf("缓冲区溢出 Out of Range\n");
						CC_DYNAMIC_CONVERSION(currentBufPtr);
						secondBufferLength = cur - address;
						stillHasUnsubmitBuf = true;
					}
					//printf("超过缓冲区，进行交换\n");
					if (lastSubmitIndex != Length() && !outOfRange)
					{
						hasUnSubmitBuf = true;
						outOfRange = true;
						firstBufferLength = Length();
					}
					cur = GetActualTypeDataSource<SIZE>(currentBufPtr->next);
					if (cur == firstBuffer.data)
					{
						currentBufPtr = &firstBuffer;
					}
					else if (cur == secondBuffer.data)
						currentBufPtr = &secondBuffer;
					else
						currentBufPtr = &backBuf;

					memcpy(cur, buf, len);
					cur += len;
					char temp[10] = { 0 };
					details::Convert(temp, atomicCounter.operator short());
					++atomicCounter;
					std::string fileName = baseFileName + "_" + temp + ".log";
					//printf("util.fp 0x%x %d %s\n", util->getfp(), __LINE__, __FILE__);

					util->CreateNewLogFile(fileName);
					/**
					 *
					 * 切换时候已经全部刷新过去
					 */
					if (!hasUnSubmitBuf && !stillHasUnsubmitBuf)
					{
						printf("缓冲区满时已全部提交完成，只需重设下标\n");
						lastSubmitIndex = 0;
						printf("切换文件句柄\n");
						RollFile();
					}
				}
				/**
				 *
				 * @Brief: 这里不能在这里判断的原因是，如果主线程需要依靠子线程的更新来进行相应的更改
				 * 而，如果子线程并没有获取时间片，那么后来的append将一直追加到缓冲区中，而因为lastsubmitindex
				 * 一直没有进行更新，所以这里不会进入if判断句，直到最后一句append结束之后，不在有append来追加数据
				 * 所以如果此时子线程开始更新数据，那么原缓冲区的内容会被成功的写入，但是后写的数据，因为lastSubmitIndex
				 * 和lastBufferLength都没有进行正确的更新所以提交的时候提交的长度将为0，这将导致后来的数据不能正确写入。
				 */
				/*printf("lastSubmitIndex= %d\n", Length());
				if (lastSubmitIndex == Length())
				{
					printf("提交完成，重设提交下标\n");
					memset(currentBufPtr->next, 0, SIZE);
					lastSubmitIndex = 0;
					lastBufferLength = -1;
				}*/
				
			}

			//************************************
			// @Method:    Bzero
			// @Returns:   void
			// @Brief:	清空备份缓冲数据
			//************************************
			void Bzero()
			{
				memset(currentBufPtr->next, 0, SIZE);
			}
			
			//************************************
			// @Method:    Has_unSubmitBuf
			// @Returns:   bool
			// @Brief:	是否有未提交缓冲
			//************************************
			bool Has_unSubmitBuf()const
			{
				return hasUnSubmitBuf || stillHasUnsubmitBuf;
			}
			//************************************
			// @Method:    SetSubmitIndex
			// @Returns:   void
			// @Parameter: index
			// @Brief:	设置已提交缓冲位置
			//************************************
			void SetSubmitIndex(size_t index)
			{
				lastSubmitIndex = index;
			}
			//************************************
			// @Method:    GetSubmitIndex
			// @Returns:   size_t
			// @Brief: 获取已提交缓冲位置
			//************************************
			size_t GetSubmitIndex()const
			{
				return lastSubmitIndex;
			}

			//************************************
			// @Method:    SetFirstBufferLength
			// @Returns:   void
			// @Parameter: length
			// @Brief:	设置上一缓冲区已缓冲数据的总长度
			//************************************
			void SetFirstBufferLength(int length)
			{
				firstBufferLength = length;
			}
			//************************************
			// @Method:    SetSecondBufferLength
			// @Returns:   void
			// @Parameter: length
			// @Brief:	设置第二个已缓冲满的数据总长度
			//************************************
			void SetSecondBufferLength(int length)
			{
				secondBufferLength = length;
			}
			//************************************
			// @Method:    Increase
			// @Returns:   void
			// @Parameter: v
			// @Brief: 增加下标
			//************************************
			void Increase(int v)
			{
				cur += v;
			}
			//************************************
			// @Method:    Avail
			// @Returns:   size_t
			// @Brief:	可用缓冲大小
			//************************************
			size_t Avail()const
			{
				return static_cast<size_t>(end() - cur);
			}
			//************************************
			// @Method:    Length
			// @Returns:   size_t
			// @Brief: 获取缓冲区数据长度
			//************************************
			size_t Length()const
			{
				//printf("在length 函数中 %d\n", lastBufferLength);
				if (firstBufferLength != -1)
				{
					return firstBufferLength;
				}
				if (secondBufferLength != -1)
				{
					return secondBufferLength;
				}
				
				char* ret = GetActualTypeDataSource<SIZE>(currentBufPtr);
				return cur - ret;
			}
			//************************************
			// @Method:    GetData
			// @Returns:   const char*
			// @Brief:	获取内部缓冲始地址
			//************************************
			const char* GetData()const
			{
				if (firstBufferLength == -1 && !hasUnSubmitBuf && !stillHasUnsubmitBuf && !switchHandler)
				{
					return ConvertToActualTypeAndGetSource();
				}
				if (secondBufferLength == -1 && !stillHasUnsubmitBuf)
				{
					if (hasUnSubmitBuf)
					{
						if (currentBufPtr->prev)
						{
							char* ret = GetActualTypeDataSource<SIZE>(currentBufPtr->prev);
							return ret;
						}
						return nullptr;
					}
					else
					{
						char* ret = ConvertToActualTypeAndGetSource();
						return ret;
					}

				}
				else
				{
					char* ret = nullptr;
					if (hasUnSubmitBuf && firstBufferLength != -1 && stillHasUnsubmitBuf && secondBufferLength!= -1)
					{
						//return firstBuffer.data;
						if (currentBufPtr->prev->prev)
						{
							ret = GetActualTypeDataSource<SIZE>(currentBufPtr->prev->prev);
							return ret;
						}
					}
					else if (!hasUnSubmitBuf && firstBufferLength == -1 && stillHasUnsubmitBuf && secondBufferLength != -1)
					{
						if (currentBufPtr->prev)
						{
							ret = GetActualTypeDataSource<SIZE>(currentBufPtr->prev);
						}
						return ret;
					}
					else
					{
						ret = ConvertToActualTypeAndGetSource();
						return ret;
					}
				}
			}
			void Reset() 
			{
				
				CC_DYNAMIC_CONVERSION(currentBufPtr);
				cur = address; 
			}

			//************************************
			// @Method:    SetSubmitFinished
			// @Returns:   void
			// @Parameter: state
			// @Brief:	设置是否成功提交完成
			//************************************
			void SetSubmitFinished(bool state)
			{
				hasUnSubmitBuf = state;
			}
			void SetifStillHasUnSubmit(bool state)
			{
				stillHasUnsubmitBuf = state;
			}
			void SetOutofRangeState(bool state)
			{
				outOfRange = state;
			}
			//************************************
			// @Method:    AsString
			// @Returns:   std::string
			// @Brief:	一种简单转换为字符串的方法
			//************************************
			std::string AsString()const
			{
				
				CC_DYNAMIC_CONVERSION(currentBufPtr);
				return std::string(address, Length());
			}
			//************************************
			// @Method:    Clear
			// @Returns:   void
			// @Brief:	清空当前缓冲区
			//************************************
			void Clear()
			{
				
				CC_DYNAMIC_CONVERSION(currentBufPtr);
				if (address == backBuf.data)
				{
					memset(address, 0, 2 * SIZE);
				}
				else
					memset(address, 0, SIZE);
			}
			char* Current()const
			{
				return (char*)cur;
			}
			FileUtility::FileUtil* GetUtil()
			{
				
				//printf("util: 0x%x util.fp 0x%x %d %s\n", util, util->getfp(), __LINE__, __FILE__);
				return util;
			}

			//************************************
			// @Method:    SetBaseFileName
			// @Returns:   void
			// @Parameter: name
			// @Brief:	设置文件基名称
			//************************************
			void SetBaseFileName(std::string name)
			{
				baseFileName = name;
			}

			void RollFile()
			{
				util->SwitchFileHandler();
				SetRollHandlerOrNot(true);
			}

			int GetSecondLengthSize()const
			{
				return secondBufferLength;
			}
			int GetFirstLengthSize()const
			{
				return firstBufferLength;
			}
		};

		template <int SIZE>
		std::atomic<short> FixBuffer<SIZE>::atomicCounter(0);

		
		class LogStream
		{
		private:
			const int kMaxDigitlen = 32;
			static std::atomic<bool> inProgress;
			typedef details::FixBuffer<details::kLargeBuffer> Buffer;
			Buffer buffer;
			//unsigned lastSubmitIndex;
			//************************************
			// @Method:    FormatInteger
			// @Returns:   void
			// @Brief:	简单的格式化整数类型方法
			//************************************
			template <typename T>
			void FormatInteger(T);
		private:
			std::mutex lockMutex;
			std::atomic<bool> finished;
			std::atomic<bool> outOfRange;
			std::thread counter;
		public:
			typedef LogStream Self;
			LogStream(const LogStream&) = delete;
			LogStream& operator=(const LogStream&) = delete;
		public:
			LogStream() = default;
			~LogStream();
			LogStream(const std::string& fileName);
			Self& operator<<(bool b)
			{
				buffer.Append((b ? "1" : "0"), 1);
				return *this;
			}

			/**
			 *
			 * @Brief:	重载一系列操作符
			 */
			Self& operator<<(short);
			Self& operator<<(unsigned short);
			Self& operator<<(int);
			Self& operator<<(unsigned);
			Self& operator<<(long);
			Self& operator<<(unsigned long);
			Self& operator<<(long long);
			Self& operator<<(unsigned long long);
			Self& operator<<(char);
			Self& operator<<(unsigned char);
			Self& operator<<(const void*);
			Self& operator<<(const char*);
			Self& operator<<(const std::string&);
			Self& operator<<(float);
			Self& operator<<(double);

			//************************************
			// @Method:    Submit
			// @Returns:   void
			// @Brief:	提交任务
			//************************************
			void Submit();
			//************************************
			// @Method:    Tick
			// @Returns:   void
			// @Brief:	定时提交的方法
			//************************************
			void Tick();
			//************************************
			// @Method:    StartCounter
			// @Returns:   void
			// @Brief:	启动定时器
			//************************************
			void StartCounter();
			//************************************
			// @Method:    StopCounter
			// @Returns:   void
			// @Brief:	停止定时器
			//************************************
			void StopCounter();
			
			//void RollFile();
		};
	}
}
